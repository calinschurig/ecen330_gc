#include "d2deque.h"



// initializes a deque a returns it.
d2deque_t d2deque_init() {
    return (d2deque_t) {
        .front_row=0,
        .num_rows=0,
        .front_col=0,
        .num_cols=0,
    };
}

// returns true if the deque has all rows filled
bool d2deque_row_isfull(d2deque_t* d) {
    return d->num_rows == D2DEQUE_MAX_ROWS;
}

// returns true if the deque has all rows filled
bool d2deque_col_isfull(d2deque_t* d) {
    return d->num_cols == D2DEQUE_MAX_COLS;
}

// gets the number of active rows in the deque.
uint16_t d2deque_row_len(d2deque_t* d) {
    return d->num_rows;
}

// gets the number of active columns in the deque.
uint16_t d2deque_col_len(d2deque_t* d) {
    return d->num_cols;
}

// copies the item at location and returns it.
chunk_t d2deque_at(d2deque_t* d, uint16_t x, uint16_t y) {
    x = (x + d->front_col) % D2DEQUE_MAX_COLS;
    y = (y + d->front_row) % D2DEQUE_MAX_ROWS;
    return d->buffer[x][y];
}

// returns a pointer to the item at the location in the deque, no length checks. 
chunk_t* d2deque_peep_at(d2deque_t* d, uint16_t x, uint16_t y) {
    x = (x + d->front_col) % D2DEQUE_MAX_COLS;
    y = (y + d->front_row) % D2DEQUE_MAX_ROWS;
    return &d->buffer[x][y];
}

// pushes a new row of items at the back of the d2deque. 
bool d2deque_pushback_row(d2deque_t* d, chunk_t* chunks) {
    if (d2deque_row_isfull(d)) {
        return false;
    }
    d->num_rows++;
    uint16_t row = (d->front_row + d->num_rows-1) % D2DEQUE_MAX_ROWS;
    for (uint16_t i = 0; i < d->num_cols; i++) {
        d->buffer[ (i+d->front_col)%D2DEQUE_MAX_COLS ][row] = chunks[i];
    }
    return true;
}

// pushes a new row of items to the front of the deque, no length checks.
bool d2deque_pushfront_row(d2deque_t* d, chunk_t* chunks) {
    if (d2deque_row_isfull(d)) {
        return false;
    }
    d->num_rows++;
    d->front_row = (d->front_row > 0) ? d->front_row-1 : D2DEQUE_MAX_ROWS-1; 
    uint16_t row = d->front_row; 
    for (uint16_t i = 0; i < d->num_cols; i++) {
        d->buffer[ (i+d->front_col)%D2DEQUE_MAX_COLS ][row] = chunks[i];
    }
    return true;
}

// pushes a new column of items at the back of the d2deque. 
bool d2deque_pushback_col(d2deque_t* d, chunk_t* chunks) {
    if (d2deque_col_isfull(d)) {
        return false;
    }
    d->num_cols++;
    uint16_t col = (d->front_col + d->num_cols-1) % D2DEQUE_MAX_COLS;
    for (uint16_t j = 0; j < d->num_rows; j++) {
        d->buffer[col][ (j+d->front_row)%D2DEQUE_MAX_ROWS ] = chunks[j];
    }
    return true;
}

// pushes a new column of items to the front of the d2deque, no length checks.
bool d2deque_pushfront_col(d2deque_t* d, chunk_t* chunks) {
    if (d2deque_col_isfull(d)) {
        return false;
    }
    d->num_cols++;
    d->front_col = (d->front_col > 0) ? d->front_col-1 : D2DEQUE_MAX_COLS-1;
    uint16_t col = d->front_col;
    for (uint16_t j = 0; j < d->num_rows; j++) {
        d->buffer[col][ (j+d->front_row)%D2DEQUE_MAX_ROWS ] = chunks[j];
    }
    return true;
}

// removes the front column of the d2deque. 
bool d2deque_rfront_col(d2deque_t* d) {
    if (d->num_cols == 0) {
        return false;
    } else {
        d->num_cols--;
        d->front_col = (d->front_col+1) % D2DEQUE_MAX_COLS;
        return true;
    }
}

// removes the back column of the d2deque. 
bool d2deque_rback_col(d2deque_t* d) {
    if (d->num_cols == 0) {
        return false;
    } else {
        d->num_cols--;
        return true;
    }
}

// removes the front row of the d2deque. 
bool d2deque_rfront_row(d2deque_t* d) {
    if (d->num_rows == 0) {
        return false;
    } else {
        d->num_rows--;
        d->front_row = (d->front_row+1) % D2DEQUE_MAX_ROWS;
        return true;
    }
}

// removes the back row of the d2deque. 
bool d2deque_rback_row(d2deque_t* d) {
    if (d->num_rows == 0) {
        return false;
    } else {
        d->num_rows--;
        return true;
    }
}
