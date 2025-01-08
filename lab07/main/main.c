#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "render.h"
#include "lcd.h"
#include "game.h"
#include "joy.h"


static const char *TAG = "lab07";
// Main application
void app_main(void)
{
	ESP_LOGI(TAG, "Starting");

	// render_init();
	// chunk_coord_t s = {25};
    // chunk_t c = chunk_gen(s, 100, 0b00000000, false);
	// render_chunk(c, 0, 0);

	// lcd_writeFrame();
	game_init();
	while(true) {
		game_tick();
	}
}


// okay, here's the plan: procedurally generated cowboy game, 
// where you go around the wild west stopping at towns and 
// doing such and such...  The actual result is much more simplified,
// but still fun to play! 