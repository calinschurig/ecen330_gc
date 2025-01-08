#include "game.h"
#include "render.h"
#include "chunk.h"
#include "d2deque.h"
#include "gen.h"
#include "joy.h"
#include "pin.h"

typedef enum {
    INIT,
    NOT_MOVING,
    MOVING_UP,
    MOVING_RIGHT,
    MOVING_DOWN,
    MOVING_LEFT
} game_state_t;

static game_state_t s;
static chunk_coord_t cc; // the chunk_coord of the top left chunk. 
static int16_t tl_x,tl_y; // top left corner of the rendered chunks. 
static d2deque_t chunks;
static uint32_t pain_counter = 0;
static uint8_t difficulty = CONFIG_CACTUS_LEVEL;

static void world_gen_up(d2deque_t* d, chunk_coord_t* top_left_chunk);
static void world_gen_down(d2deque_t* d, chunk_coord_t* top_left_chunk);
static void world_gen_right(d2deque_t* d, chunk_coord_t* top_left_chunk);
static void world_gen_left(d2deque_t* d, chunk_coord_t* top_left_chunk);
static bool is_joy_left();
static bool is_joy_right();
static bool is_joy_up();
static bool is_joy_down();
static bool is_cullable_row(uint16_t c_row);
static bool is_cullable_col(uint16_t c_col);
static chunk_tile_t get_tile_at_screen_location(int16_t x, int16_t y, d2deque_t* d);
static bool is_colliding_up();
static bool is_colliding_down();
static bool is_colliding_right();
static bool is_colliding_left();

// initializes the game. 
void game_init() {
    render_init();
    joy_init();
    s = INIT;
    chunks = d2deque_init();
    cc.coord[0] = CONFIG_START_X_CHUNK;
    cc.coord[1] = CONFIG_START_Y_CHUNK;
    tl_x = CONFIG_START_X_TL;
    tl_y = CONFIG_START_Y_TL;
    pain_counter = 0;
    pin_reset(HW_BTN_A);
    pin_reset(HW_BTN_B);
    pin_input(HW_BTN_A, true);
    pin_input(HW_BTN_B, true);
    game_generate_first_frame();
}



// ticks the game, and renders. 
void game_tick() {
    
    switch(s) { // MEALY state transitions
        case INIT: {
        s = NOT_MOVING;
        } break;
        case NOT_MOVING: { // checking for joystick inputs in all directions. 
            if (is_joy_up()) {
                s = MOVING_UP;
            } else if (is_joy_right()) {
                s = MOVING_RIGHT;
            } else if (is_joy_down()) {
                s = MOVING_DOWN;
            } else if (is_joy_left()) {
                s = MOVING_LEFT;
            } else {
                s = NOT_MOVING;
            }
        } break;
        case MOVING_UP: { // not moving up check
            if (!is_joy_up()) {
                s = NOT_MOVING;
            }
        } break;
        case MOVING_RIGHT: { // not moving right check
            if (!is_joy_right()) {
                s = NOT_MOVING;
            }
        } break;
        case MOVING_DOWN: { // not moving down check
            if (!is_joy_down()) {
                s = NOT_MOVING;
            }
        } break;
        case MOVING_LEFT: { // not moving left check
            if (!is_joy_left()) {
                s = NOT_MOVING;
            }
        } break;
        default: { // error handling
            s = INIT;
        }
    }


    switch(s) { // MOORE state actions
        case INIT: {
        s = NOT_MOVING;
        } break;
        case NOT_MOVING: {

        } break;
        case MOVING_UP: { // checking for 
            if (!is_colliding_up()) {
                tl_y += CONFIG_STEP_SIZE;
                if (is_colliding_up()) {
                    pain_counter++;
                }
            }
            if (is_cullable_row(chunks.num_rows-1)) {
                d2deque_rfront_row(&chunks);
            }
            if (!is_cullable_row(0)) {
                world_gen_up(&chunks, &cc);
            }
        } break;
        case MOVING_RIGHT: {
            if (!is_colliding_right()) {
                tl_x -= CONFIG_STEP_SIZE;
                if (is_colliding_right()) {
                    pain_counter++;
                }
            }
            if (is_cullable_col(0)) {
                d2deque_rfront_col(&chunks);
                tl_x += CONFIG_CHUNK_W;
                cc.coord[0]++;
            }
            if (!is_cullable_col(chunks.num_cols-1)) {
                world_gen_right(&chunks, &cc);
            }
        } break;
        case MOVING_DOWN: {
            if (!is_colliding_down()) {
                tl_y -= CONFIG_STEP_SIZE;
                if (is_colliding_down()) {
                    pain_counter++;
                }
            }
            if (is_cullable_row(0)) {
                d2deque_rback_row(&chunks);
                tl_y+=CONFIG_CHUNK_H;
                cc.coord[1]--;
            }
            if (!is_cullable_row(chunks.num_rows-1)) {
                world_gen_down(&chunks, &cc);
                
            }
        } break;
        case MOVING_LEFT: {
            if (!is_colliding_left()) {
                tl_x += CONFIG_STEP_SIZE;
                if (is_colliding_left()) {
                    pain_counter++;
                }
            }
            
            if (is_cullable_col(chunks.num_cols-1)) {
                d2deque_rback_col(&chunks);
            }
            if (!is_cullable_col(0)) {
                world_gen_left(&chunks, &cc);
            }
            
        } break;
    }

    static bool last_a = false;
    bool this_a = !pin_get_level(HW_BTN_A);
    if (!last_a && this_a) {
        difficulty += CONFIG_CACTUS_INCREMENT;
    }
    last_a = this_a;

    static bool last_b = false;
    bool this_b = !pin_get_level(HW_BTN_B);
    if (!last_b && this_b) {
        difficulty -= CONFIG_CACTUS_INCREMENT;
    }
    last_b = this_b;

    render_landscape(&chunks, tl_x, tl_y);
    lcd_setFontBackground(WHITE);
    lcd_setFontSize(1);
    char pos_buf[CONFIG_POSITION_BUFFER_SIZE];
    sprintf(pos_buf, "(%d, %d)", cc.coord[0], cc.coord[1]);
    lcd_drawString(0, 0, pos_buf, BLACK);
    char pain_buf[CONFIG_POSITION_BUFFER_SIZE*2];
    sprintf(pain_buf, "PAIN: %ld", pain_counter);
    lcd_drawString(0, LCD_CHAR_H, pain_buf, BLACK);

    lcd_fillRect(CONFIG_PLAYER_LEFT_BOUND, CONFIG_PLAYER_UP_BOUND, CONFIG_PLAYER_WIDTH, 
                 CONFIG_PLAYER_HEIGHT, CONFIG_PLAYER_COLOR);
    char dif_buf[CONFIG_POSITION_BUFFER_SIZE];
    sprintf(dif_buf, "DIFFICULTY: %d", difficulty);
    lcd_drawString(0, LCD_CHAR_H*2, dif_buf, BLACK);
    lcd_writeFrame();
}

#define FIRST_TL 60

void game_generate_first_frame() {
    
    tl_x = FIRST_TL;
    tl_y = FIRST_TL;
    world_gen_up(&chunks, &cc);
    world_gen_down(&chunks, &cc);
    world_gen_right(&chunks, &cc);
    
    world_gen_right(&chunks, &cc);
    // world_gen_up(&chunks, &cc);
    world_gen_down(&chunks, &cc);
    world_gen_down(&chunks, &cc);
    // world_gen_down(&chunks, &cc);
    world_gen_right(&chunks, &cc);
    world_gen_right(&chunks, &cc);
    world_gen_right(&chunks, &cc);
    world_gen_right(&chunks, &cc);

    // world_gen_up(&chunks, &cc);
    world_gen_left(&chunks, &cc);
    world_gen_left(&chunks, &cc);
    // world_gen_left(&chunks, &cc);
    // printf("chunks info: num_cols: %d, num_rows: %d\n", chunks.num_cols, chunks.num_rows);
    render_landscape(&chunks, tl_x, tl_y);
    lcd_writeFrame();
    // printf("tl_x:%d, tl_y:%d\n", tl_x, tl_y);
}


// adds a row of newly generated chunks to the deque on the top. 
static void world_gen_up(d2deque_t* d, chunk_coord_t* top_left_chunk) {
    chunk_t buf[D2DEQUE_MAX_COLS];
    gen_row(buf, difficulty, top_left_chunk->coord[0], top_left_chunk->coord[1]+1, d->num_cols);
    d2deque_pushback_row(d, buf);
    top_left_chunk->coord[1]++;
    tl_y -= CONFIG_CHUNK_H;
}

// adds a row of newly generated chunks to the deque on the bottom. 
static void world_gen_down(d2deque_t* d, chunk_coord_t* top_left_chunk) {
    chunk_t buf[D2DEQUE_MAX_COLS];
    gen_row(buf, difficulty, top_left_chunk->coord[0], top_left_chunk->coord[1]-d->num_rows, d->num_cols);
    d2deque_pushfront_row(d, buf);
}

// adds a column of newly generated chunks to the deque on the right. 
static void world_gen_right(d2deque_t* d, chunk_coord_t* top_left_chunk) {
    chunk_t buf[D2DEQUE_MAX_ROWS];
    gen_col(buf, difficulty, top_left_chunk->coord[0]+d->num_cols, top_left_chunk->coord[1]-d->num_rows+1, d->num_rows);
    d2deque_pushback_col(d, buf);
}

// adds a column of newly generated chunks to the deque on the left. 
static void world_gen_left(d2deque_t* d, chunk_coord_t* top_left_chunk) {
    chunk_t buf[D2DEQUE_MAX_ROWS];
    gen_col(buf, difficulty, top_left_chunk->coord[0]-1, top_left_chunk->coord[1]-d->num_rows+1, d->num_rows);
    d2deque_pushfront_col(d, buf);
    top_left_chunk->coord[0]--;
    tl_x -= CONFIG_CHUNK_W;
}

// checks if the joystick is left
static bool is_joy_left() {
    int32_t x, y;
    joy_get_displacement(&x, &y);
    return x < -1*CONFIG_JOY_INPUT_BUFFER;
}
// checks if the joystick is right
static bool is_joy_right() {
    int32_t x, y;
    joy_get_displacement(&x, &y);
    return x > CONFIG_JOY_INPUT_BUFFER;
}
// checks if the joystick is up
static bool is_joy_up() {
    int32_t x, y;
    joy_get_displacement(&x, &y);
    return y < -1*CONFIG_JOY_INPUT_BUFFER;
}
// checks if the joystick is down
static bool is_joy_down() {
    int32_t x, y;
    joy_get_displacement(&x, &y);
    return y > CONFIG_JOY_INPUT_BUFFER;
}

// checks if a row is cullable. 0 is towards the top of the screen.
static bool is_cullable_row(uint16_t c_row) {
    int16_t up_lim = tl_y + c_row*CONFIG_CHUNK_H;
    int16_t down_lim = tl_y+CONFIG_CHUNK_H + c_row*CONFIG_CHUNK_H;
    if (down_lim < -1*CONFIG_SCREEN_CULLING_BUFFER_SIZE) {
        return true;
    } else if (up_lim > LCD_H+CONFIG_SCREEN_CULLING_BUFFER_SIZE) {
        return true;
    } else {
        return false;
    }
}

// checks if a column is cullable. 0 is towards the left of the screen.
static bool is_cullable_col(uint16_t c_col) {
    int16_t left_lim = tl_x + c_col*CONFIG_CHUNK_W;
    int16_t right_lim = tl_x+CONFIG_CHUNK_W + c_col*CONFIG_CHUNK_W;
    if (right_lim < -1*CONFIG_SCREEN_CULLING_BUFFER_SIZE) {
        return true;
    } else if (left_lim > LCD_W+CONFIG_SCREEN_CULLING_BUFFER_SIZE) {
        return true;
    } else {
        return false;
    }
}

static chunk_tile_t get_tile_at_screen_location(int16_t x, int16_t y, d2deque_t* d) {
    uint16_t chunks_x = (x - tl_x) / CONFIG_CHUNK_W;
    uint16_t chunks_y = d->num_rows-1 - (y-tl_y)/CONFIG_CHUNK_H;
    uint8_t tile_x = ((x-tl_x)%CONFIG_CHUNK_W)/CONFIG_CHAR_W;
    uint8_t tile_y = CHUNK_TILE_ROWS-1 - ((y-tl_y)%CONFIG_CHUNK_H)/CONFIG_CHAR_H;
    chunk_t c = d2deque_at(d, chunks_x, chunks_y);
    // chunk_print(c);
    // printf("(%d, %d)", tile_x, tile_y);
    return chunk_get(c, tile_x, tile_y);

}

// checks if the player is colliding upward with a cactus
static bool is_colliding_up() {
    chunk_tile_t ul_c = get_tile_at_screen_location(CONFIG_PLAYER_LEFT_BOUND, CONFIG_PLAYER_UP_BOUND, &chunks);
    chunk_tile_t ur_c = get_tile_at_screen_location(CONFIG_PLAYER_RIGHT_BOUND, CONFIG_PLAYER_UP_BOUND, &chunks);
    if (ul_c == CACTUS_TILE || ur_c == CACTUS_TILE) {
        return true;
    } else {
        return false;
    }
}

// checks if the player is colliding downward with a cactus
static bool is_colliding_down() {
    chunk_tile_t dl_c = get_tile_at_screen_location(CONFIG_PLAYER_LEFT_BOUND, CONFIG_PLAYER_DOWN_BOUND, &chunks);
    chunk_tile_t dr_c = get_tile_at_screen_location(CONFIG_PLAYER_RIGHT_BOUND, CONFIG_PLAYER_DOWN_BOUND, &chunks);
    if (dl_c == CACTUS_TILE || dr_c == CACTUS_TILE) {
        return true;
    } else {
        return false;
    }
}

// checks if the player is colliding downward with a cactus
static bool is_colliding_right() {
    chunk_tile_t ur_c = get_tile_at_screen_location(CONFIG_PLAYER_RIGHT_BOUND, CONFIG_PLAYER_UP_BOUND, &chunks);
    chunk_tile_t dr_c = get_tile_at_screen_location(CONFIG_PLAYER_RIGHT_BOUND, CONFIG_PLAYER_DOWN_BOUND, &chunks);
    if (ur_c == CACTUS_TILE || dr_c == CACTUS_TILE) {
        return true;
    } else {
        return false;
    }
}

// checks if the player is colliding downward with a cactus
static bool is_colliding_left() {
    chunk_tile_t ul_c = get_tile_at_screen_location(CONFIG_PLAYER_LEFT_BOUND, CONFIG_PLAYER_UP_BOUND, &chunks);
    chunk_tile_t dl_c = get_tile_at_screen_location(CONFIG_PLAYER_LEFT_BOUND, CONFIG_PLAYER_DOWN_BOUND, &chunks);
    if (ul_c == CACTUS_TILE || dl_c == CACTUS_TILE) {
        return true;
    } else {
        return false;
    }
}

