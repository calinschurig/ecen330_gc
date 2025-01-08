#include "pcg_variants.h"
#include <stdio.h>
#include "src/config.h"
#include "src/deque.h"
#include "src/chunk.h"
#include"src/rapidhash.h"


int main() {
    pcg32i_random_t state = PCG32I_INITIALIZER;
    uint32_t my_random_var = pcg32i_random_r(&state);
    // printf("my_random_var: %u\n", my_random_var);
    // my_random_var = pcg32i_random_r(&state);
    // printf("my_random_var: %u\n", my_random_var);
    // printf(" a change?\n");

    // printf(" more stuff\n");
    deque_t d = deque_init();

    deque_pushback(&d, 10000);

    // printf("wow!\n");
    chunk_tile_t tile = chunk_get(0x0000001F, 2, 0);
    // printf("tile: %d\n", tile);
    chunk_coord_t s = {25};
    chunk_t c = chunk_gen(s, 55, 0b00000000, false);
    uint8_t x = 0, y = 1;
    // c = chunk_set(c, x, y, 1);
    chunk_print(c);

    // char t = chunk_tile_to_char( chunk_get(c, 3, 0) );
    
    
    printf("%d,%d: %c\n", x, y, chunk_tile_to_char(chunk_get(c, x, y)));
    
}