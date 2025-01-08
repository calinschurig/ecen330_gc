#include "gen.h"


// generates a row of n chunks starting at (x,y), with increasing values of x.
// buf should be at least n long. 
void gen_row(chunk_t* buf, uint8_t cactus_level, int16_t x, int16_t y, uint16_t n) {
    for (int32_t i = 0; i < n; i++) {
        chunk_coord_t cc = {.coord[0]=x+i, .coord[1]=y};
        buf[i] = chunk_gen(cc, cactus_level, 0b00000000, false);
    }
}

// generates a column of n chunks starting at (x,y), with increasing values of y.
// buf should be at least n long. 
void gen_col(chunk_t* buf, uint8_t cactus_level, int16_t x, int16_t y, uint16_t n) {
    for (int32_t i = 0; i < n; i++) {
        chunk_coord_t cc = {.coord[0]=x, .coord[1]=y+i};
        buf[i] = chunk_gen(cc, cactus_level, 0b00000000, false);
    }
}