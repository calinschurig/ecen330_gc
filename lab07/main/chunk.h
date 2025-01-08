#ifndef CHUNK_H
#define CHUNK_H
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

#define CHUNK_TILE_BITS 2
#define CHUNK_TILE_MAX 4
#define CHUNK_TILES_PER_CHUNK 16
#define CHUNK_TILE_ROWS 4
#define CHUNK_TILE_COLS 4
#define CHUNK_TILES (CHUNK_TILE_ROWS * CHUNK_TILE_COLS)

typedef uint32_t chunk_t;
typedef uint8_t chunk_tile_row_t;

typedef enum {
    DESERT_TILE = 0,
    CACTUS_TILE = 1,
    WATER_TILE = 2,
    TOWN_TILE = 3
} chunk_tile_t;

typedef union {
    chunk_t chunk;
    chunk_tile_row_t rows[4];
} chunk_u_t; 

typedef union {
    uint32_t seed;
    int16_t coord[2];
} chunk_coord_t;

typedef uint8_t chunk_water_info_t; // 8 bits of water direction, starting at N, the NE, 
                                    // then E, the SE, etc...

// Generates a chunk from the given parameters. 
// Seed should be the seed member of the chunk_coord_t union. 
chunk_t chunk_gen(chunk_coord_t seed, uint8_t cactus_level, 
    chunk_water_info_t water, bool has_town);

// get's the chunk tile at the specified location within the chunk.
// x and y should be from 0 to 3, not above 4. 
chunk_tile_t chunk_get(chunk_t chunk, uint8_t x, uint8_t y);

chunk_t chunk_set(chunk_t chunk, uint8_t x, uint8_t y, chunk_tile_t t);

void chunk_print(chunk_t chunk);

void chunk_str(chunk_t chunk, char* buf);

// puts each row of a chunk into a string, specified by bufs. 
void chunk_row_strs(chunk_t chunk, char** bufs);

char chunk_tile_to_char(chunk_tile_t t);










#endif //CHUNK_H