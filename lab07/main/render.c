#include "render.h"
#include "chunk.h"
#include "lcd.h"
#include "config.h"


// initializes the lcd and sets the text size
bool render_init() {
    lcd_init();
    lcd_setFontSize(CONFIG_TEXT_SIZE);
    lcd_frameEnable();
    return true;
}


// renders an entire d2deque of chunks, with the x and y specifying the top left corner.
void render_landscape(d2deque_t* d, int16_t x, int16_t y) {
    lcd_setFontSize(CONFIG_TEXT_SIZE);
    lcd_noFontBackground();
    uint16_t n_r = d->num_rows;
    uint16_t n_c = d->num_cols;
    for (int32_t r = 0; r < n_r; r++) {
        for (int32_t c = 0; c < n_c; c++) {
            // printf("rendering chunk x:%ld, y:%ld\n", c, n_r-1-r);
            // chunk_print(d2deque_at(d, c, n_r-1-r));
            render_chunk(d2deque_at(d, c, n_r-1-r), x+c*CONFIG_CHUNK_W, y+r*CONFIG_CHUNK_H);
        }
    }
}

// renders a chunk to the lcd screen, with the x and y specifying the top left corner.
void render_chunk(chunk_t chunk, int16_t x, int16_t y) {
    char buf[CHUNK_TILE_ROWS][CHUNK_TILE_COLS+1];
    char* bufs[CHUNK_TILE_ROWS];
    for (int32_t i = 0; i < CHUNK_TILE_ROWS; i++) {
        bufs[i] = buf[i];
    }
    chunk_row_strs(chunk, bufs);
    lcd_fillRect(x, y, CHUNK_TILE_COLS*CONFIG_CHAR_W, CHUNK_TILE_ROWS*CONFIG_CHAR_H, CONFIG_COLOR_DESERT);
    lcd_setFontBackground(CONFIG_COLOR_DESERT);
    for (int32_t i = 0; i < CHUNK_TILE_ROWS; i++) {
        lcd_drawString(x, y + i*CONFIG_CHAR_H, bufs[CHUNK_TILE_ROWS-1-i], CONFIG_COLOR_CACTUS);
    }
}

