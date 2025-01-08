#ifndef GEN_H
#define GEN_H

#include "config.h"
#include "chunk.h"

// generates a row of n chunks starting at (x,y), with increasing values of x.
// buf should be at least n long. 
void gen_row(chunk_t* buf, uint8_t cactus_level, int16_t x, int16_t y, uint16_t n);

// generates a column of n chunks starting at (x,y), with increasing values of y.
// buf should be at least n long. 
void gen_col(chunk_t* buf, uint8_t cactus_level, int16_t x, int16_t y, uint16_t n);













#endif // GEN_H