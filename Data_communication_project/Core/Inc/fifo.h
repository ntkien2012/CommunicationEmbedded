/*
 * fifo.h
 *
 *  Created on: Apr 12, 2025
 *      Author: Ngo Khanh
 */

#ifndef INC_FIFO_H_
#define INC_FIFO_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    uint8_t *buffer;
    uint16_t head;
    uint16_t tail;
    uint16_t size;
} FIFO_Buffer_t;

bool FIFO_Init(FIFO_Buffer_t *fifo, uint8_t *bufferArray, uint16_t size);
uint16_t FIFO_Write(FIFO_Buffer_t *fifo, uint8_t *data, uint16_t len);
uint16_t FIFO_Read(FIFO_Buffer_t *fifo, uint8_t *data, uint16_t len);
uint16_t FIFO_Available(FIFO_Buffer_t *fifo);


#endif /* INC_FIFO_H_ */
