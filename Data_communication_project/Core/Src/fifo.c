/*
 * fifo.c
 *
 *  Created on: Apr 12, 2025
 *      Author: Ngo Khanh
 */

#include "fifo.h"

bool FIFO_Init(FIFO_Buffer_t *fifo, uint8_t *bufferArray, uint16_t size) {
    if (!fifo || !bufferArray || size == 0) return false;
    fifo->buffer = bufferArray;
    fifo->size = size;
    fifo->head = fifo->tail = 0;
    return true;
}

uint16_t FIFO_Write(FIFO_Buffer_t *fifo, uint8_t *data, uint16_t len) {
    uint16_t i;
    for (i = 0; i < len; i++) {
        uint16_t next = (fifo->head + 1) % fifo->size;
        if (next == fifo->tail) break; // full
        fifo->buffer[fifo->head] = data[i];
        fifo->head = next;
    }
    return i;
}

uint16_t FIFO_Read(FIFO_Buffer_t *fifo, uint8_t *data, uint16_t len) {
    uint16_t available = FIFO_Available(fifo);
    if (len > available) len = available;

    for (uint16_t i = 0; i < len; i++) {
        data[i] = fifo->buffer[fifo->tail];
        fifo->tail = (fifo->tail + 1) % fifo->size;
    }

    return len;
}

uint16_t FIFO_Available(FIFO_Buffer_t *fifo) {
    if (fifo->head >= fifo->tail)
        return fifo->head - fifo->tail;
    else
        return fifo->size - fifo->tail + fifo->head;
}

