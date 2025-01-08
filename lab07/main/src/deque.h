#ifndef DEQUE_H
#define DEQUE_H


#include <stddef.h>
#include <stdint.h>
#include "config.h"
#include "chunk.h"

#define DEQUE_MAX_SIZE CONFIG_DEQUE_MAX_SIZE

// #define deque_at(DEQUE, index) ((DEQUE).buffer[ ((DEQUE).front+index)%DEQUE_SIZE ])

typedef struct {
    uint16_t front;
    uint16_t length;
    chunk_t buffer[DEQUE_MAX_SIZE];
} deque_t;



// initializes a deque a returns it.
deque_t deque_init();

// gets the length of the deque.
uint16_t deque_len(deque_t* d);

// copies the item at index and returns it.
chunk_t deque_at(deque_t* d, uint16_t index);

// pushes a new item at the back of the deque, no length checks.
void deque_pushback(deque_t* d, chunk_t chunk);

// removes and returns an item from the back of the deque, no length checks.
chunk_t deque_popback(deque_t* d);

// returns a pointer to the item at the end of the deque, no length checks. 
chunk_t* deque_peepback(deque_t* d);

// copies the item at the back and returns it. 
chunk_t deque_copyback(deque_t* d);

// pushes a new item to the front of the deque, no length checks.
void deque_pushfront(deque_t* d, chunk_t chunk);

// removes an item from the front and returns it, no length checks.
chunk_t deque_popfront(deque_t* d);

// returns a pointer to the item at the front, no length checks.
chunk_t* deque_peepfront(deque_t* d);

// copies the item at the front and returns it, no length checks.
chunk_t deque_copyfront(deque_t* d);



#endif // DEQUE_H