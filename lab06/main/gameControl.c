
#include <stdio.h>
#include <stdlib.h> // rand

#include "hw.h"
#include "lcd.h"
#include "cursor.h"
#include "sound.h"
#include "pin.h"
#include "missile.h"
#include "plane.h"
#include "gameControl.h"
#include "config.h"

// sound support
#include "missileLaunch.h"

#define FIRE_MISSILE_BTN HW_BTN_A
#define STATUS_BUF_LEN 20
#define VOLUME 		10 //should be a value between 0 and 100

// M3: Define stats constants

// All missiles
missile_t missiles[CONFIG_MAX_TOTAL_MISSILES];

// Alias into missiles array
missile_t *player_missiles = missiles+0;
missile_t *enemy_missiles = missiles+CONFIG_MAX_PLAYER_MISSILES;
missile_t *plane_missile = missiles+CONFIG_MAX_PLAYER_MISSILES+
									CONFIG_MAX_ENEMY_MISSILES;

// M3: Declare stats variables
static uint32_t impacted_missiles = 0;
static uint32_t missiles_fired = 0;


// Initialize the game control logic.
// This function initializes all missiles, planes, stats, etc.
void gameControl_init(void)
{
	// Initialize missiles
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
		missile_init_enemy(enemy_missiles+i);
	for (uint32_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++)
		missile_init_idle(player_missiles+i);
	missile_init_idle(plane_missile);

	// M3: Initialize plane
	plane_init(plane_missile);

	// M3: Initialize stats
	impacted_missiles = 0;
	missiles_fired = 0;
	

	// M3: Set sound volume
	sound_init(MISSILELAUNCH_SAMPLE_RATE);
	sound_set_volume(VOLUME);
}

// WARNING: make sure the missile is idle first. 
static void fire_player_missile(missile_t* m) {
	coord_t x, y;
	cursor_get_pos(&x, &y);
	missile_init_player(m, x, y);
	sound_start(missileLaunch, MISSILELAUNCH_SAMPLE_RATE, false);
}

// checks for a collision between missile exploding and missile m. 
static bool check_collision(missile_t* exploding, missile_t* m) {
	coord_t x, y;
	missile_get_pos(m, &x, &y);
	return missile_is_colliding(exploding, x, y);
}

// Update the game control logic.
// This function calls the missile & plane tick functions, reinitializes
// idle enemy missiles, handles button presses, fires player missiles,
// detects collisions, and updates statistics.
void gameControl_tick(void)
{
	// Tick missiles in one batch
	for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++)
		missile_tick(missiles+i);

	// Reinitialize idle enemy missiles
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
		if (missile_is_idle(enemy_missiles+i))
			missile_init_enemy(enemy_missiles+i);

	// M2: Check for button press. If so, launch a free player missile.
	static bool this_btn_push = false;
	static bool last_btn_push = false;
	if ( (last_btn_push != (this_btn_push = !pin_get_level(FIRE_MISSILE_BTN))) && this_btn_push ) {
		
		// looping through all player missiles checking for an idle one. 
		for (uint32_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++) {
			if (missile_is_idle(&player_missiles[i])) { // fire it off, count the fire.
				fire_player_missile(&player_missiles[i]);
				missiles_fired++;
				break;
			}
		}
	}
	last_btn_push = this_btn_push;

	// M2: Check for moving non-player missile collision with an explosion.
	// in this loop, enemy_missiles indexes into both enemy and plane missiles
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES+CONFIG_MAX_PLANE_MISSILES; i++) {
		if (missile_is_exploding(&enemy_missiles[i])) {
			continue;
		}
		// checking collisions between the missile in question and every other missile
		for (uint32_t j = 0; j < CONFIG_MAX_TOTAL_MISSILES; j++) {
			if (check_collision(&missiles[j], &enemy_missiles[i])) {
				missile_explode(&enemy_missiles[i]);
				break;
			}
		}
	}

	// M3: Count non-player impacted missiles
	
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES+CONFIG_MAX_PLANE_MISSILES; i++) {
		if (missile_is_impacted(&enemy_missiles[i])) {
			impacted_missiles++;
		}
	}

	// M3: Tick plane & draw stats
	plane_tick();
	char shot[STATUS_BUF_LEN];
	char impacted[STATUS_BUF_LEN];
	snprintf(shot, STATUS_BUF_LEN, "Shot: %ld", missiles_fired);
	snprintf(impacted, STATUS_BUF_LEN, "Impacted: %ld", impacted_missiles);
	lcd_drawString(LCD_CHAR_W, 0, shot, CONFIG_COLOR_STATUS);
	lcd_drawString(LCD_CHAR_W+LCD_W/2, 0, impacted, CONFIG_COLOR_STATUS);


	// M3: Check for flying plane collision with an explosion.
	for (uint32_t j = 0; j < CONFIG_MAX_TOTAL_MISSILES; j++) {
		coord_t x=0, y=0;
		plane_get_pos(&x, &y);
		if (missile_is_colliding(&missiles[j], x, y) || missile_is_colliding(&missiles[j], x+CONFIG_PLANE_WIDTH, y)) {
			plane_explode();
			break;
		}
	}
}
