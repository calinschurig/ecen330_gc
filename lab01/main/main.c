#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd.h"
#include "pac.h"

static const char *TAG = "lab01";

#define delayMS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

//----------------------------------------------------------------------------//
// Car Implementation - Begin
//----------------------------------------------------------------------------//

// Car constants
#define CAR_CLR rgb565(220,30,0)
#define WINDOW_CLR rgb565(180,210,238)
#define TIRE_CLR BLACK
#define HUB_CLR GRAY
#define CAR_WIDTH 60
#define CAR_HEIGHT 32
#define CAR_TOP_FORWARD_OFFSET 1
#define CAR_TOP_WIDTH 39
#define CAR_TOP_HEIGHT 11
#define HOOD_HEIGHT 3
#define WINDOW_CORNER_RADIUS 2
#define WINDOW_DISTANCE_FROM_ROOF 1
#define WINDOW_WIDTH 16
#define WINDOW_HEIGHT 8
#define WINDOW_DISTANCE_FROM_EDGE 2
#define TIRE_RADIUS 7
#define HUB_RADIUS 4
#define LEFT_WHEEL_X 11
#define RIGHT_WHEEL_X 48
#define WHEEL_Y (CAR_HEIGHT-1-TIRE_RADIUS)



// TODO: Finish car part constants

/**
 * @brief Draw a car at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @details Draw the car components relative to the anchor point (top, left).
 */
void drawCar(coord_t x, coord_t y)
{
	// TODO: Implement car procedurally with lcd geometric primitives.

	lcd_fillRect(x, y+CAR_TOP_HEIGHT, CAR_WIDTH, CAR_HEIGHT-TIRE_RADIUS-CAR_TOP_HEIGHT, CAR_CLR);
	lcd_fillCircle(x+LEFT_WHEEL_X, y+WHEEL_Y, TIRE_RADIUS, TIRE_CLR);
	lcd_fillCircle(x+LEFT_WHEEL_X, y+WHEEL_Y, HUB_RADIUS, HUB_CLR);
	lcd_fillCircle(x+RIGHT_WHEEL_X, y+WHEEL_Y, TIRE_RADIUS, TIRE_CLR);
	lcd_fillCircle(x+RIGHT_WHEEL_X, y+WHEEL_Y, HUB_RADIUS, HUB_CLR);
	const int16_t hood_left_x = x+CAR_TOP_FORWARD_OFFSET+CAR_TOP_WIDTH;
	const int16_t hood_bottom_y = y+CAR_TOP_HEIGHT-1;
	lcd_fillTriangle(hood_left_x, hood_bottom_y-HOOD_HEIGHT+1, hood_left_x, hood_bottom_y, x+CAR_WIDTH-1, hood_bottom_y, CAR_CLR);
	const int16_t car_top_left_x = x+CAR_TOP_FORWARD_OFFSET;
	// const int16_t car_top_bottom = y+CAR_TOP_HEIGHT-1;
	lcd_fillRect(car_top_left_x, y, CAR_TOP_WIDTH, CAR_TOP_HEIGHT, CAR_CLR);
	
	lcd_fillRoundRect(car_top_left_x+WINDOW_DISTANCE_FROM_EDGE, y+WINDOW_DISTANCE_FROM_ROOF, 
		WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_CORNER_RADIUS, WINDOW_CLR);
	lcd_fillRoundRect(car_top_left_x+CAR_TOP_WIDTH-WINDOW_DISTANCE_FROM_EDGE-WINDOW_WIDTH-1, y+WINDOW_DISTANCE_FROM_ROOF, 
		WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_CORNER_RADIUS, WINDOW_CLR);


	// printf("WHEEL_Y: %d\n", WHEEL_Y);
	// printf("BOTTOM_Y: %d\n", CAR_TOP_HEIGHT+1+CAR_HEIGHT-1-TIRE_RADIUS-CAR_TOP_HEIGHT);
	// printf("BOTTOM_OF_CAR: %d\n", CAR_HEIGHT-1-TIRE_RADIUS-CAR_TOP_HEIGHT);

}

//----------------------------------------------------------------------------//
// Car Implementation - End
//----------------------------------------------------------------------------//

// Main display constants
#define BACKGROUND_CLR rgb565(0,60,90)
#define TITLE_CLR GREEN
#define STATUS_CLR WHITE
#define STR_BUF_LEN 12 // string buffer length
#define FONT_SIZE 2
#define FONT_W (LCD_CHAR_W*FONT_SIZE)
#define FONT_H (LCD_CHAR_H*FONT_SIZE)
#define STATUS_W (FONT_W*3)

#define WAIT 1000 // milliseconds
#define DELAY_EX3 20 // milliseconds

// Object position and movement
#define OBJ_X 100
#define OBJ_Y 100
#define OBJ_MOVE 3 // pixels


void app_main(void)
{
	ESP_LOGI(TAG, "Start up");
	lcd_init();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_setFontSize(FONT_SIZE);
	lcd_drawString(0, 0, "Hello World! (lcd)", TITLE_CLR);
	printf("Hello World! (terminal)\n");
	delayMS(WAIT);

	// TODO: Exercise 1 - Draw car in one location.
	// * Fill screen with background color
	// * Draw string "Exercise 1" at top left of screen with title color
	// * Draw car at OBJ_X, OBJ_Y
	// * Wait 2 seconds
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 1", TITLE_CLR);
	drawCar(OBJ_X, OBJ_Y);
	delayMS(WAIT);

	// TODO: Exercise 2 - Draw moving car (Method 1), one pass across display.
	// Clear the entire display and redraw all objects each iteration.
	// Use a loop and increment x by OBJ_MOVE each iteration.
	// Start x off screen (negative coordinate).
	// Move loop:
	// * Fill screen with background color
	// * Draw string "Exercise 2" at top left of screen with title color
	// * Draw car at x, OBJ_Y
	// * Display the x position of the car at bottom left of screen
	//   with status color
	int16_t i = -CAR_WIDTH-1;
	// Draws a moving car, clearing the entire screen between frames without
	// using a frame buffer. 
	for (i = -CAR_WIDTH-1; i <= LCD_W+1; i+=OBJ_MOVE) {
		lcd_fillScreen(BACKGROUND_CLR);
		lcd_drawString(0, 0, "Exercise 2", TITLE_CLR);
		drawCar(i,OBJ_Y);
		char x_str_buf[STR_BUF_LEN] = "";
		sprintf(x_str_buf, "%d", i);
		lcd_drawString(0, LCD_H-FONT_H, x_str_buf, STATUS_CLR);
	}
	

	// TODO: Exercise 3 - Draw moving car (Method 2), one pass across display.
	// Move by erasing car at old position, then redrawing at new position.
	// Objects that don't change or move are drawn once.
	// Before loop:
	// * Fill screen once with background color
	// * Draw string "Exercise 3" at top left of screen with title color
	// Move loop:
	// * Erase car at old position by drawing a rectangle with background color
	// * Draw car at new position
	// * Erase status at bottom by drawing a rectangle with background color
	// * Display new position status of car at bottom left of screen
	// After running the above first, add a 20ms delay within the loop
	// at the end to see the effect.
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 3", TITLE_CLR);

	// draws a car moving accross the screen, but rather than clearing the entire
	// screen, erases the previous frame's object. Doesn't use a frame buffer. 
	for (i = -CAR_WIDTH-1; i <= LCD_W+1; i+=OBJ_MOVE) {
		
		lcd_fillRect(i-OBJ_MOVE, OBJ_Y, CAR_WIDTH, CAR_HEIGHT, BACKGROUND_CLR);
		drawCar(i,OBJ_Y);
		
		lcd_fillRect(0, LCD_H-FONT_H, STATUS_W, FONT_H, BACKGROUND_CLR);
		char x_str_buf[STR_BUF_LEN] = "";
		sprintf(x_str_buf, "%d", i);
		lcd_drawString(0, LCD_H-FONT_H, x_str_buf, STATUS_CLR);

		delayMS(DELAY_EX3);
	}

	

	// TODO: Exercise 4 - Draw moving car (Method 3), one pass across display.
	// First, draw all objects into a cleared, off-screen frame buffer.
	// Then, transfer the entire frame buffer to the screen.
	// Before loop:
	// * Enable the frame buffer
	// Move loop:
	// * Fill screen (frame buffer) with background color
	// * Draw string "Exercise 4" at top left with title color
	// * Draw car at x, OBJ_Y
	// * Display position of the car at bottom left with status color
	// * Write the frame buffer to the LCD

	lcd_frameEnable();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 4", TITLE_CLR);

	// Draws a car moving accross the screen using a frame buffer. 
	for (i = -CAR_WIDTH-1; i <= LCD_W+1; i+=OBJ_MOVE) {
		
		lcd_fillRect(i-OBJ_MOVE, OBJ_Y, CAR_WIDTH, CAR_HEIGHT, BACKGROUND_CLR);
		drawCar(i,OBJ_Y);
		
		lcd_fillRect(0, LCD_H-FONT_H, STATUS_W, FONT_H, BACKGROUND_CLR);
		char x_str_buf[STR_BUF_LEN] = "";
		sprintf(x_str_buf, "%d", i);
		lcd_drawString(0, LCD_H-FONT_H, x_str_buf, STATUS_CLR);
		lcd_writeFrame();
	}
	lcd_frameDisable();


	// TODO: Exercise 5 - Draw an animated Pac-Man moving across the display.
	// Use Pac-Man sprites instead of the car object.
	// Cycle through each sprite when moving the Pac-Man character.
	// Before loop:
	// * Enable the frame buffer
	// Nest the move loop inside a forever loop:
	// Move loop:
	// * Fill screen (frame buffer) with background color
	// * Draw string "Exercise 5" at top left with title color
	// * Draw Pac-Man at x, OBJ_Y with yellow color;
	//   Cycle through sprites to animate chomp
	// * Display position at bottom left with status color
	// * Write the frame buffer to the LCD
	
	lcd_frameEnable();

	lcd_fillScreen(BACKGROUND_CLR);
	lcd_drawString(0, 0, "Exercise 5", TITLE_CLR);
	const uint8_t pidx[] = {0, 1, 2, 1};
	uint8_t pac_frame = 0;

	// draws an  animated pacman sprite moving accross the screen and looping around
	for (i = -PAC_W-1; true; i+=OBJ_MOVE) {
		
		lcd_fillRect(i-OBJ_MOVE, OBJ_Y, PAC_W, PAC_H, BACKGROUND_CLR);
		lcd_drawBitmap(i, OBJ_Y, pac[pidx[pac_frame]], PAC_W, PAC_H, YELLOW);
		pac_frame++;
		if (pac_frame >= 4) {
			pac_frame = 0;
		}
		
		lcd_fillRect(0, LCD_H-FONT_H, STATUS_W, FONT_H, BACKGROUND_CLR);
		char x_str_buf[STR_BUF_LEN] = "";
		sprintf(x_str_buf, "%d", i);
		lcd_drawString(0, LCD_H-FONT_H, x_str_buf, STATUS_CLR);
		lcd_writeFrame();
		if (i > LCD_W+1) {
			i = -PAC_W-1;
		}
	}
}
