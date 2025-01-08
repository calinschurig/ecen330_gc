#include "board.h"
#include "config.h"
#include "game.h"
#include "nav.h"
#include "graphics.h"

#include "hw.h"
#include "pin.h"
#include "lcd.h"
#include "com.h"

#include <stdio.h>


typedef uint8_t loc_t;
#define LOC_NUM_BITS (sizeof(loc_t)*8)
#define LOC_HALF_BITS (LOC_NUM_BITS/2)
#define LOC_HALF_CAP (0x1<<LOC_HALF_BITS)
#define LOC(R, C) ( (loc_t) ((( ((loc_t)((R)%LOC_HALF_CAP)))<<LOC_HALF_BITS) + ((C)%LOC_HALF_CAP)))


typedef enum { // state enumeration
    game_init_st,
    game_new_game_st,
    game_wait_mark_st,
    game_mark_st,
    game_wait_restart_st,
} game_state_t;

// state variables for game
static mark_t player_turn = X_m;
static mark_t local_mark = no_m;
static game_state_t game_state = game_init_st;
static uint8_t remote_recieved_flag = false;

// Initialize the game logic.
void game_init(void) {
    board_clear();
    player_turn = X_m;
    game_state = game_init_st;
}

// returns the char version of player mark. 
static char get_player_letter(mark_t player_mark) {
    if (player_mark != X_m && player_mark != O_m) {
        return ' ';
    }
    return (player_mark == X_m)? 'X' : 'O';
}

// displays which player's turn it currently is.
static void display_current_player_message(void) {
    char buffer[GAME_MESSAGE_BUFFER_SIZE];
    sprintf(buffer, "Current player: %C", get_player_letter(player_turn));
    graphics_drawMessage(buffer, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
}

// displays a winner message
static void display_player_winner_message(mark_t winner) {
    char buffer[GAME_MESSAGE_BUFFER_SIZE];
    sprintf(buffer, "%C is the winner!", get_player_letter(player_turn));
    graphics_drawMessage(buffer, CONFIG_MESS_CLR, CONFIG_BACK_CLR);
}

// displays a draw game message
static void display_game_draw_message() {
    graphics_drawMessage("The game is a draw!", CONFIG_MESS_CLR, CONFIG_BACK_CLR);
}

// draws an X or O in the given grid location
static void draw_player_mark(int8_t r, int8_t c, mark_t mark) {    
    if (mark == X_m) {
        graphics_drawX(r, c, CONFIG_MARK_CLR);
    } else if (mark == O_m) {
        graphics_drawO(r, c, CONFIG_MARK_CLR);
    }
}

// takes a loc_t and splits it into the r and c. 
static void unpack_loc(loc_t loc, int8_t* r, int8_t* c) {
    *r = (loc>>LOC_HALF_BITS)%LOC_HALF_CAP;
    *c = loc%LOC_HALF_CAP;
}

// returns the alternating X or O mark, or no_m otherwise. 
static mark_t next_m (mark_t mark) {
    if (mark == X_m) {
        return O_m;
    } else if (mark == O_m) {
        return X_m;
    } else {
        return no_m;
    }
}

// gets the mark location if A is pressed. returns true
// if a mark is found, returns false otherwise. Puts location
// information into r and c if it returns true.
static bool get_local_mark_location(int8_t* r, int8_t* c) {
    uint8_t is_A_pressed = !pin_get_level(HW_BTN_A);
    if (is_A_pressed) {
        nav_get_loc(r, c);
        remote_recieved_flag = false;
        return true;
    } else {
        return false;
    }
}

// gets the mark location if it has been sent over the
// uart connection. If it does, will return true and
// store the location in r and c, otherwise returns false. 
static bool get_remote_mark_location(int8_t* r, int8_t* c) {
    loc_t loc;
    
    if (com_read(&loc, 1)) {
        unpack_loc(loc, r, c);
        remote_recieved_flag = true;
        // printf("get_remote_mark_location: %d\n", remote_recieved_flag);
        return true;
    } else {
        // printf("get_remote_mark_location: %d\n", remote_recieved_flag);
        return false;
    }
}

// returns 0 if no mark was recieved, 1 if mark was recieved
// if expected mark is no_m or NULL, then not sure what mark this 
// player is, so accept anything. 
static bool get_mark_location(int8_t* r, int8_t* c, mark_t expected_mark) {
    if (local_mark != X_m && local_mark != O_m 
        && expected_mark != X_m && expected_mark != O_m) 
    { // if local_mark and expected_mark are not set
        if (get_remote_mark_location(r, c)) {
            return true;
        } else if (get_local_mark_location(r, c)) {
            return true;
        } else {
            return false;
        }
    }
    if (expected_mark == local_mark) {
        return get_local_mark_location(r, c);
    } else {
        return get_remote_mark_location(r, c);
    }
}

// Update the game logic.
void game_tick(void) {
    
    
    // bool is_menu_pressed = !pin_get_level(HW_BTN_MENU);

    // transitions
    switch (game_state) {
        // initial state
        case game_init_st: {
            game_state = game_new_game_st;
        } break;
        // sets all the variables for a new game. 
        case game_new_game_st: {
            // transitioning to wait_mark_st
            game_state = game_wait_mark_st; { 
                board_clear();
                player_turn = X_m;
                nav_set_loc(CONFIG_BOARD_R/2, CONFIG_BOARD_C/2); // todo: set to 0?
                lcd_fillScreen(CONFIG_BACK_CLR);
                graphics_drawGrid(CONFIG_GRID_CLR);
                display_current_player_message();

                loc_t loc;
                while(com_read(&loc, sizeof(loc_t)));
                
            }
        } break;
        // waits for A to be pressed, then attempts to mark and
        // transition to game_mark_st
        case game_wait_mark_st: {
            int8_t r, c;
            // getting the mark location, either from local or through the uart. 
            if (get_mark_location(&r, &c, no_m)) {
                // checking if that location is available for marking, in which case mark it.
                if (board_get(r, c) == no_m) { 
                    // transition to game_mark_st, and also marking all of my stuff.
                    game_state = game_mark_st; {
                        loc_t loc = LOC(r, c);
                        com_write(&loc, sizeof(loc_t));
                        board_set(r, c, player_turn);
                        
                        draw_player_mark(r, c, player_turn);
                    }
                }
            }

        } break;
        // determines if the game is won or drawn, then 
        // displays the appropriate message and goes to the
        // appropriate state. 
        case game_mark_st: {
            // transition to board winner. 
            if (board_winner(player_turn)) {
                game_state = game_wait_restart_st; {
                    display_player_winner_message(player_turn);
                }
            // transition to draw.
            } else if (CONFIG_BOARD_SPACES == board_mark_count()) {
                game_state = game_wait_restart_st; {
                    display_game_draw_message();
                }
            // transitions to next turn. 
            } else { 
                game_state = game_wait_mark_st; {
                    player_turn = (player_turn == X_m)? O_m : X_m;
                    display_current_player_message();
                }
            }
        } break;
        // waits for a restart button for a new game. 
        case game_wait_restart_st: {
            uint8_t is_start_pressed = !pin_get_level(HW_BTN_START);
            if (is_start_pressed) {
                game_state = game_new_game_st;
            }
        } break;
        // error handling. 
        default: {
            game_state = game_init_st;
        } break;
    }

    // action within states
    switch (game_state) {
        // initial state, making sure to init all our variables and move into game_new_game_st.
        case game_init_st: {

        } break;
        case game_new_game_st: {
            
        } break;
        case game_wait_mark_st: {

        } break;
        case game_mark_st: {

        } break;
        case game_wait_restart_st: {

        } break;
        default: {

        } break;
    }

}