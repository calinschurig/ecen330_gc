#ifndef RENDER_H
#define RENDER_H

#include "chunk.h"
#include "d2deque.h"
#include "config.h"

// initializes the lcd and sets the text size
bool render_init();

// renders an entire d2deque of chunks, with the x and y specifying the top left corner.
void render_landscape(d2deque_t* d, int16_t x, int16_t y); 

// renders a chunk to the lcd screen, with the x and y specifying the top left corner.
void render_chunk(chunk_t chunk, int16_t x, int16_t y);


#endif // RENDER_H