/*
 * fifo.c
 *
 *  Created on: Apr 10, 2025
 *      Author: Ngo Khanh
 */

#include "fifo.h"

void fifo_init(fifo_t *fifo) {
    fifo->head = fifo->tail = 0;
}

bool fifo_push(fifo_t *fifo, uint8_t byte) {
    uint16_t next = (fifo->head + 1) % FIFO_SIZE;
    if (next == fifo->tail) return false; // full
    fifo->buffer[fifo->head] = byte;
    fifo->head = next;
    return true;
}

bool fifo_pop(fifo_t *fifo, uint8_t *byte) {
    if (fifo->head == fifo->tail) return false; // empty
    *byte = fifo->buffer[fifo->tail];
    fifo->tail = (fifo->tail + 1) % FIFO_SIZE;
    return true;
}

bool fifo_is_empty(fifo_t *fifo) {
    return (fifo->head == fifo->tail);
}

