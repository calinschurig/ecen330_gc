#include "deque.h"

// initializes a deque a returns it.
deque_t deque_init() {
    deque_t new_deque;
    new_deque.front = 0;
    new_deque.length = 0;
    return new_deque;
}

// gets the length of the deque.
uint16_t deque_len(deque_t* d) {
    return d->length;
}

// copies the item at index and returns it.
chunk_t deque_at(deque_t* d, uint16_t index) {
    return d->buffer[ (d->front+index) % DEQUE_MAX_SIZE];
}

// pushes a new item at the back of the deque, no length checks.
void deque_pushback(deque_t* d, chunk_t chunk) {
    d->length = (d->length+1) % DEQUE_MAX_SIZE;
    d->buffer[ (d->front+d->length)%DEQUE_MAX_SIZE ];
}

// removes and returns an item from the back of the deque, no length checks.
chunk_t deque_popback(deque_t* d) {
    return d->buffer[ (d->front+d->length--)%DEQUE_MAX_SIZE ];
}

// returns a pointer to the item at the end of the deque, no length checks. 
chunk_t* deque_peepback(deque_t* d) {
    return d->buffer + ( (d->front+d->length)%DEQUE_MAX_SIZE );
}

// copies the item at the back and returns it. 
chunk_t deque_copyback(deque_t* d) {
    return d->buffer[ (d->front+d->length)%DEQUE_MAX_SIZE ];
}

// pushes a new item to the front of the deque, no length checks.
void deque_pushfront(deque_t* d, chunk_t chunk) {
    d->front = (d->front == 0)? DEQUE_MAX_SIZE : d->front--;
    d->buffer[d->front] = chunk;
}

// removes an item from the front and returns it, no length checks.
chunk_t deque_popfront(deque_t* d) {
    chunk_t return_chunk = d->buffer[d->front];
    d->front = (d->front == DEQUE_MAX_SIZE)? 0 : d->front++;
    return return_chunk;
}

// returns a pointer to the item at the front, no length checks.
chunk_t* deque_peepfront(deque_t* d) {
    return d->buffer + d->front;
}

// copies the item at the front and returns it, no length checks.
chunk_t deque_copyfront(deque_t* d) {
    return d->buffer[d->front];
}
