#ifndef D2DEQUE_H
#define D2DEQUE_H

#include "stdint.h"
#include "chunk.h"
#include "config.h"

#define D2DEQUE_MAX_ROWS CONFIG_D2DEQUE_MAX_ROWS
#define D2DEQUE_MAX_COLS CONFIG_D2DEQUE_MAX_COLS

typedef struct {
    uint16_t front_row;
    uint16_t num_rows;
    uint16_t front_col;
    uint16_t num_cols;
    chunk_t buffer[D2DEQUE_MAX_COLS][D2DEQUE_MAX_ROWS];
} d2deque_t;



// initializes a deque a returns it.
d2deque_t d2deque_init();

// returns true if the deque has all rows filled
bool d2deque_row_isfull(d2deque_t* d);

// returns true if the deque has all rows filled
bool d2deque_col_isfull(d2deque_t* d);

// gets the number of active rows in the deque.
uint16_t d2deque_row_len(d2deque_t* d);

// gets the number of active columns in the deque.
uint16_t d2deque_col_len(d2deque_t* d);

// copies the item at location and returns it.
chunk_t d2deque_at(d2deque_t* d, uint16_t x, uint16_t y);

// returns a pointer to the item at the location in the deque, no length checks. 
chunk_t* d2deque_peep_at(d2deque_t* d, uint16_t x, uint16_t y);

// pushes a new row of items at the back of the d2deque. 
bool d2deque_pushback_row(d2deque_t* d, chunk_t* chunks);

// pushes a new row of items to the front of the deque, no length checks.
bool d2deque_pushfront_row(d2deque_t* d, chunk_t* chunks);

// pushes a new column of items at the back of the d2deque. 
bool d2deque_pushback_col(d2deque_t* d, chunk_t* chunks);

// pushes a new column of items to the front of the d2deque, no length checks.
bool d2deque_pushfront_col(d2deque_t* d, chunk_t* chunks);

// removes the front column of the d2deque. 
bool d2deque_rfront_col(d2deque_t* d);

// removes the back column of the d2deque. 
bool d2deque_rback_col(d2deque_t* d);

// removes the front row of the d2deque. 
bool d2deque_rfront_row(d2deque_t* d);

// removes the back row of the d2deque. 
bool d2deque_rback_row(d2deque_t* d);


#endif // D2DEQUE_H