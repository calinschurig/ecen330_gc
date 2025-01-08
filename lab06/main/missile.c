#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "lcd.h" // coord_t
#include "missile.h"
#include "config.h"

// The same missile structure is used for all missiles in the game.
// All state variables for a missile are contained in the missile structure.
// There are no global variables maintained in the missile.c file.
// Each missile function requires a pointer argument to a missile struct.

// This enum is used to identify the state of missile.
typedef enum {
    INITIALIZING,
    MOVING,
    EXPLODING_GROWING,
    EXPLODING_SHRINKING,
    IMPACTED,
    IDLE,
} missile_state_t;

#define NUM_LAUNCH_SITES                3
#define LAUNCH_SITE_PLAYER_Y            LCD_H
#define LAUNCH_SITE_X(launch_site_num)  ((LCD_W / (NUM_LAUNCH_SITES+1)) * launch_site_num)
#define LAUNCH_SITE_SPACING             LAUNCH_SITE_X(1)
#define LAUNCH_ENEMY_Y                  LCD_CHAR_H
#define RANDOM_SIDE_X_BUFFER            15
#define RANDOM_SIDE_Y_RANGE             4


/******************** Missile Init Functions ********************/

// Different _init_ functions are used depending on the missile type.

// Initialize the missile as an idle missile. If initialized to the idle
// state, a missile doesn't appear nor does it move.
static void missile_init_general(missile_t *missile) {
    int32_t y_dist = missile->y_origin - missile->y_dest;
    int32_t x_dist = missile->x_origin - missile->x_dest;
    missile->total_length = sqrtf( (float) (x_dist*x_dist + y_dist*y_dist));

	// Tracks the current x,y of missile.
	missile->x_current = missile->x_origin;
	missile->y_current = missile->y_origin;

	// The current length of the flight path from origin to current x,y.
	missile->length = 0;

	// A flag to indicate the missile should be detonated when moving.
	missile->explode_me = false;
}

void missile_init_idle(missile_t *missile) {
	// Current state. The 'enum' is defined in your missile.c file.
	missile->currentState = IDLE;

}

// Initialize the missile as a player missile. This function takes an (x, y)
// destination of the missile (as specified by the user). The origin is the
// closest "firing location" to the destination (there are three firing
// locations evenly spaced along the bottom of the screen).
void missile_init_player(missile_t *missile, coord_t x_dest, coord_t y_dest) {
    missile->currentState = INITIALIZING;
    missile->type = MISSILE_TYPE_PLAYER;
    
    uint8_t launch_site_num = (x_dest + LAUNCH_SITE_SPACING/2)/LAUNCH_SITE_SPACING;
    launch_site_num = (launch_site_num <= 0)? 1 : launch_site_num;
    launch_site_num = (launch_site_num > NUM_LAUNCH_SITES)? NUM_LAUNCH_SITES : launch_site_num;
    missile->x_origin = LAUNCH_SITE_X(launch_site_num);
	missile->y_origin = LAUNCH_SITE_PLAYER_Y;
	missile->x_dest = x_dest;
	missile->y_dest = y_dest;
    missile_init_general(missile);
}

// Initialize the missile as an enemy missile. This will randomly choose the
// origin and destination of the missile. The origin is somewhere near the
// top of the screen, and the destination is the very bottom of the screen.
void missile_init_enemy(missile_t *missile) {
    missile->currentState = INITIALIZING;
    missile->type = MISSILE_TYPE_ENEMY;
    uint32_t r_value_1 = rand();
    uint32_t r_value_2 = rand();
    coord_t x_origin = r_value_1 % (LCD_W - 2*RANDOM_SIDE_X_BUFFER) + RANDOM_SIDE_X_BUFFER;
    coord_t y_origin = r_value_1%RANDOM_SIDE_Y_RANGE + LAUNCH_ENEMY_Y;
    coord_t x_dest = r_value_2 % (LCD_W - 2*RANDOM_SIDE_X_BUFFER) + RANDOM_SIDE_X_BUFFER;
    coord_t y_dest = LCD_H-1;
    missile->x_origin = x_origin;
	missile->y_origin = y_origin;
	missile->x_dest = x_dest;
	missile->y_dest = y_dest;
    missile_init_general(missile);
}

// Initialize the missile as a plane missile. This function takes the (x, y)
// location of the plane as an argument and uses it as the missile origin.
// The destination is randomly chosen along the bottom of the screen.
void missile_init_plane(missile_t *missile, coord_t x_orig, coord_t y_orig) {
    missile->currentState = INITIALIZING;
    missile->type = MISSILE_TYPE_PLANE;
    uint32_t r_value = rand();
    coord_t x_dest = r_value % (LCD_W - 2*RANDOM_SIDE_X_BUFFER) + RANDOM_SIDE_X_BUFFER;
    coord_t y_dest = LCD_H-1;
    missile->x_origin = x_orig;
	missile->y_origin = y_orig;
	missile->x_dest = x_dest;
	missile->y_dest = y_dest;
    missile_init_general(missile);
}

/******************** Missile Control & Tick Functions ********************/

// Used to indicate that a moving missile should be detonated. This occurs
// when an enemy or a plane missile is located within an explosion zone.
void missile_explode(missile_t *missile) {
    missile->explode_me = true;
}

// Tick the state machine for a single missile.
void missile_tick(missile_t *missile) {
    switch(missile->currentState) { // transitions
        case INITIALIZING: {
            missile->currentState = MOVING;
        } break;
        case MOVING: {
            if (missile->explode_me) {
                missile->currentState = EXPLODING_GROWING;
            } else if (missile->length >= missile->total_length) { // if missile should explode/impact
                if (missile->type == MISSILE_TYPE_PLAYER) { // making sure the missile doesn't overshoot
                    missile->explode_me = true;
                    missile->currentState = EXPLODING_GROWING;
                    missile->x_current = missile->x_dest;
                    missile->y_current = missile->y_dest;
                } else {
                    missile->currentState = IMPACTED;
                }
            }
            
        } break;
        case EXPLODING_GROWING: {
            if (missile->radius > CONFIG_EXPLOSION_MAX_RADIUS - CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK) {
                missile->currentState = EXPLODING_SHRINKING;
            }
        } break;
        case EXPLODING_SHRINKING: {
            if (missile->radius < CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK) {
                missile->currentState = IDLE;
            }
        } break;
        case IMPACTED: {
            missile->currentState = IDLE;
        } break;
        case IDLE: {

        } break;
        default: {

        } break;
    } // transitions


    switch(missile->currentState) { // in-state actions
        case INITIALIZING: {

        } break;
        
        case MOVING: {
            float dist_per_tick;
            color_t mis_col;
            switch (missile->type) { // assignments for missile_color and dist_per_tick
                case MISSILE_TYPE_PLAYER: {
                    dist_per_tick = CONFIG_PLAYER_MISSILE_DISTANCE_PER_TICK;
                    mis_col = CONFIG_COLOR_PLAYER_MISSILE;
                } break;
                case MISSILE_TYPE_ENEMY: {
                    dist_per_tick = CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK;
                    mis_col = CONFIG_COLOR_ENEMY_MISSILE;
                } break;
                case MISSILE_TYPE_PLANE: {
                    dist_per_tick = CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK;
                    mis_col = CONFIG_COLOR_PLANE_MISSILE;
                } break;
                default: {
                    dist_per_tick = CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK;
                    mis_col = CONFIG_COLOR_ENEMY_MISSILE;
                }
            } // end assignments for missile_color and dist_per_tick
            missile->length += dist_per_tick;
            float fraction = missile->length / missile->total_length;
            missile->x_current = missile->x_origin + fraction*(missile->x_dest-missile->x_origin);
            missile->y_current = missile->y_origin + fraction*(missile->y_dest-missile->y_origin);
            
            lcd_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, mis_col);
        } break;
        case EXPLODING_GROWING: { // exploding growing
            color_t mis_col = (missile->type == MISSILE_TYPE_ENEMY)? 
                CONFIG_COLOR_ENEMY_MISSILE
                : (missile->type == MISSILE_TYPE_PLAYER)? 
                    CONFIG_COLOR_PLAYER_MISSILE
                    : CONFIG_COLOR_PLANE_MISSILE;
            missile->radius += CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
            lcd_fillCircle(missile->x_current, missile->y_current, (coord_t) missile->radius, mis_col);
        } break; // end exploding growing
        case EXPLODING_SHRINKING: { // exploding shrinking
            color_t mis_col = (missile->type == MISSILE_TYPE_ENEMY)? 
                CONFIG_COLOR_ENEMY_MISSILE
                : (missile->type == MISSILE_TYPE_PLAYER)? 
                    CONFIG_COLOR_PLAYER_MISSILE
                    : CONFIG_COLOR_PLANE_MISSILE;
            missile->radius -= CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
            lcd_fillCircle(missile->x_current, missile->y_current, (coord_t) missile->radius, mis_col);
        } break; // end exploding shrinking
        case IMPACTED: {
            
        } break;
        case IDLE: {

        } break;
        default: {

        } break;
    } // in-state actions
}

/******************** Missile Status Functions ********************/

// Return the current missile position through the pointers *x,*y.
void missile_get_pos(missile_t *missile, coord_t *x, coord_t *y) {
    *x = missile->x_current;
    *y = missile->y_current;
}

// Return the missile type.
missile_type_t missile_get_type(missile_t *missile) {
    return missile->type;
}

// Return whether the given missile is moving.
bool missile_is_moving(missile_t *missile) {
    return (missile->currentState == MOVING);
}

// Return whether the given missile is exploding. If this missile
// is exploding, it can explode another intersecting missile.
bool missile_is_exploding(missile_t *missile) {
    return (missile->currentState == EXPLODING_GROWING 
         || missile->currentState == EXPLODING_SHRINKING);
}

// Return whether the given missile is idle.
bool missile_is_idle(missile_t *missile) {
    return (missile->currentState == IDLE);
}

// Return whether the given missile is impacted.
bool missile_is_impacted(missile_t *missile) {
    return (missile->currentState == IMPACTED);
}

// Return whether an object (e.g., missile or plane) at the specified
// (x,y) position is colliding with the given missile. For a collision
// to occur, the missile needs to be exploding and the specified
// position needs to be within the explosion radius.
bool missile_is_colliding(missile_t *missile, coord_t x, coord_t y) {
    if (!missile_is_exploding(missile)) {
        return false;
    }
    int32_t dist_x = x - missile->x_current;
    int32_t dist_y = y - missile->y_current;
    int32_t dist_squared = dist_x*dist_x + dist_y*dist_y;
    int32_t rad_squared = missile->radius * missile->radius;
    return (dist_squared <= rad_squared);
}
