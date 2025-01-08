#include <stdlib.h>
#include "plane.h"
#include "config.h"
#include "lcd.h"
#include "missile.h"

#define PLANE_Y_RANGE 20
#define PLANE_Y_MAX 60
#define MISSILE_X_RANGE (LCD_W-120)
#define MISSILE_X_AVG (LCD_W/2)
#define PLANE_FRONT_X p_x
#define PLANE_BACK_X (p_x+CONFIG_PLANE_WIDTH)
#define PLANE_TOP_Y (p_y - CONFIG_PLANE_HEIGHT/2)
#define PLANE_BOT_Y (p_y + CONFIG_PLANE_HEIGHT/2)
#define PLANE_FRONT_Y (p_y)

static missile_t* p_missile = 0x0;
static coord_t p_x=0, p_y=0, p_x_launch=-1;
static bool explode_plane = false;
static uint32_t idle_ticks = 0;
typedef enum {
    INIT_ST,
    FLYING_ST,
    IDLE_ST,
    ERR_ST = -1,
} P_STATE_T;
P_STATE_T p_state = IDLE_ST;
/******************** Plane Init Function ********************/

// Initialize the plane state machine. Pass a pointer to the missile
// that will be (re)launched by the plane. It will only have one missile.
void plane_init(missile_t *plane_missile) {
    if (plane_missile == 0x0) {
        p_state = ERR_ST;
    } else {
        p_state = IDLE_ST;
    }
    p_missile = plane_missile;
}

/******************** Plane Control & Tick Functions ********************/

// Trigger the plane to explode.
void plane_explode(void) {
    explode_plane = true;
}

// State machine tick function.
void plane_tick(void) {
    // begin MEALY transitions
    switch (p_state) { 
        case INIT_ST: { // INITIALIZATION
            p_state = FLYING_ST; { // making sure that all the variables are set. 
                p_y = (rand()%PLANE_Y_RANGE - PLANE_Y_RANGE) + (PLANE_Y_MAX);
                p_x = LCD_W+CONFIG_PLANE_WIDTH;
                p_x_launch = (rand()%MISSILE_X_RANGE - MISSILE_X_RANGE/2) + MISSILE_X_AVG;
                explode_plane = false;
            }
        } break;
        case FLYING_ST: {
            if (explode_plane) {
                p_state = IDLE_ST;
                idle_ticks = 0;
            } else if ( p_x <= 0-CONFIG_PLANE_WIDTH) {
                p_state = IDLE_ST;
                idle_ticks = 0;
            }
        } break;
        case IDLE_ST: {
            if (idle_ticks >= CONFIG_PLANE_IDLE_TIME_TICKS) {
                p_state = INIT_ST;
            }
        } break;
        case ERR_ST: {
            p_state = ERR_ST;
        } break;
        default: {

        }
    } // end transitions

    // begin in-state actions
    switch (p_state) { 
        case INIT_ST: {

        } break;
        case FLYING_ST: {
            if (p_x <= p_x_launch) {
                p_x_launch = 0-LCD_W;
                missile_init_plane(p_missile, p_x, p_y);
            }
            lcd_fillTriangle(PLANE_FRONT_X, PLANE_FRONT_Y, 
                PLANE_BACK_X, PLANE_TOP_Y, 
                PLANE_BACK_X, PLANE_BOT_Y,
                CONFIG_COLOR_PLANE);
            p_x -= CONFIG_PLANE_DISTANCE_PER_TICK;
        } break;
        case IDLE_ST: {
            idle_ticks++;
        } break;
        case ERR_ST: {

        } break;
        default: {

        }
    } // end in-state actions
}

/******************** Plane Status Function ********************/

// Return the current plane position through the pointers *x,*y.
void plane_get_pos(coord_t *x, coord_t *y) {
    *x = p_x;
    *y = p_y;
}

// Return whether the plane is flying.
bool plane_is_flying(void) {
    return p_state == FLYING_ST;
}
