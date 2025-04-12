/*
 * fifo.h
 *
 *  Created on: Apr 10, 2025
 *      Author: Ngo Khanh
 */

#ifndef INC_FIFO_H_
#define INC_FIFO_H_

#include <stdint.h>
#include <stdbool.h>

#define FIFO_SIZE 256

typedef struct {
	uint8_t buffer[FIFO_SIZE];
	uint16_t head;
	uint16_t tail;
} fifo_t;

void fifo_init(fifo_t *fifo);
bool fifo_push(fifo_t *fifo, uint8_t byte);
bool fifo_pop(fifo_t *fifo, uint8_t *byte);
bool fifo_is_empty(fifo_t *fifo);

#endif /* INC_FIFO_H_ */
