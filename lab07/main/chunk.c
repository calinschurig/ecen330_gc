#include "chunk.h"
#include "pcg_variants.h"
#include <stdio.h>
#include "string.h"
#define WATER_DIRECTIONS 8
#define TILE_MASK 0b11

chunk_t chunk_water_gen(chunk_t c, chunk_water_info_t water);
chunk_t chunk_set(chunk_t chunk, uint8_t x, uint8_t y, chunk_tile_t t);


// generates a chunk from the given parameters. 
// seed should be the seed member of the chunk_coord_t union. 
chunk_t chunk_gen(chunk_coord_t seed, uint8_t cactus_level, 
                  chunk_water_info_t water, bool has_town) {
    chunk_t c = chunk_water_gen(0x0, water);
    pcg32i_random_t rnd =  {seed.seed, seed.seed>>8};
    // struct pcg_state_16 rnd = {seed.seed};
    // pcg_unique_16_xsh_rs_8_random_r(&rnd);

    for (int8_t x = 0; x < CHUNK_TILE_COLS; x++) { for (int8_t y = 0; y <CHUNK_TILE_ROWS; y++) {
        if (chunk_get(c, x, y) != DESERT_TILE) continue;
        if (pcg32i_random_r(&rnd)%256 < cactus_level) {
            c = chunk_set(c, x, y, CACTUS_TILE);
        }
    }}
    // printf("generated chunk (%d,%d):\n", seed.coord[0], seed.coord[1]);
    // chunk_print(c);
    
    return c;
}



// get's the chunk tile at the specified location within the chunk.
// x and y should be from 0 to 3, not above CHUNK_TILE_MAX. 
chunk_tile_t chunk_get(chunk_t chunk, uint8_t x, uint8_t y) {
    x %= CHUNK_TILE_COLS;
    y %= CHUNK_TILE_ROWS;
    uint32_t shift = (CHUNK_TILE_BITS*(x + y*CHUNK_TILE_COLS)); 
    // printf("pretest: %x\n", shift);
    // uint32_t mask = 0b11;
    // printf("test: 0x%x\n", mask);
    return (chunk_tile_t) (chunk >> shift) % CHUNK_TILE_MAX; 
}

void chunk_print(chunk_t chunk) {
    char buffer[CHUNK_TILES + CHUNK_TILE_ROWS + 1];
    chunk_str(chunk, buffer);
    printf("%s", buffer);
}

// needs a buffer of at least CHUNK_TILES + CHUNK_TILE_ROWS + 1
void chunk_str(chunk_t chunk, char* buf) {
    uint32_t b_i = 0;
    for (int32_t y = CHUNK_TILE_COLS-1; y >= 0; y--) { 
        for (int32_t x = 0; x < CHUNK_TILE_ROWS; x++) {
            chunk_tile_t t = chunk_get(chunk, x, y);
            buf[b_i] = chunk_tile_to_char(t);
            b_i++;
        }
        buf[b_i++] = '\n';
    }
    buf[CHUNK_TILES + CHUNK_TILE_ROWS] = '\0';
}

void chunk_row_strs(chunk_t chunk, char** bufs) {
    for (int32_t y = CHUNK_TILE_COLS-1; y >= 0; y--) { 
        for (int32_t x = 0; x < CHUNK_TILE_ROWS; x++) {
            chunk_tile_t t = chunk_get(chunk, x, y);
            bufs[y][x] = chunk_tile_to_char(t);
        }
        bufs[y][CHUNK_TILE_COLS] = '\0';
    }
}


char chunk_tile_to_char(chunk_tile_t t) {
    switch (t) {
        case DESERT_TILE: {
            return ' ';
        } break;
        case CACTUS_TILE: {
            return 'Y';
        } break;
        case WATER_TILE: {
            return '~';
        } break;
        case TOWN_TILE: {
            return 'X';
        } break;
        default: {
            return ' ';
        }
    }
}


chunk_t chunk_set(chunk_t chunk, uint8_t x, uint8_t y, chunk_tile_t t) {
    uint32_t shift = (CHUNK_TILE_BITS*(y*CHUNK_TILE_COLS + x));
    uint32_t mask = 0b11 << shift;
    return (chunk & ~mask) | (t<<shift);
}

chunk_t chunk_water_gen(chunk_t c, chunk_water_info_t water) {
    uint8_t interior_tile_water_levels[2][2] = {{0, 0}, {0, 0}};

    for (int i = WATER_DIRECTIONS-1; i >=0 ; i--) {
        if (water >= (1<<i) ) {
            water -= (1<<i);
            switch (i) {
                case 7: {
                    c = chunk_set(c, 1, 3, WATER_TILE);
                    interior_tile_water_levels[0][1]++;
                    c = chunk_set(c, 2, 3, WATER_TILE);
                    interior_tile_water_levels[1][1]++;
                } break;
                case 6: {
                    c = chunk_set(c, 3, 3, WATER_TILE);
                    interior_tile_water_levels[1][1]++;
                } break;
                case 5: {
                    c = chunk_set(c, 3, 2, WATER_TILE);
                    interior_tile_water_levels[1][1]++;
                    c = chunk_set(c, 3, 1, WATER_TILE);
                    interior_tile_water_levels[1][0]++;
                } break;
                case 4: {
                    c = chunk_set(c, 3, 0, WATER_TILE);
                    interior_tile_water_levels[1][0]++;
                } break;
                case 3: {
                    c = chunk_set(c, 2, 0, WATER_TILE);
                    interior_tile_water_levels[1][0]++;
                    c = chunk_set(c, 1, 0, WATER_TILE);
                    interior_tile_water_levels[0][0]++;
                } break;
                case 2: {
                    c = chunk_set(c, 0, 0, WATER_TILE);
                    interior_tile_water_levels[0][0]++;
                } break;
                case 1: {
                    c = chunk_set(c, 0, 1, WATER_TILE);
                    interior_tile_water_levels[0][0]++;
                    c = chunk_set(c, 0, 2, WATER_TILE);
                    interior_tile_water_levels[0][1]++;
                } break; 
                case 0: {
                    c = chunk_set(c, 0, 3, WATER_TILE);
                    interior_tile_water_levels[0][1]++;
                } break;
            }
        }
    }
    for (uint8_t i = 0; i < 2; i++) { for (uint8_t j = 0; j < 2; j++) {
        if ( interior_tile_water_levels[i][j] >= 2 ) {
            c = chunk_set(c, i+1, j+1, WATER_TILE);
        }
    } }
    return c;
}
