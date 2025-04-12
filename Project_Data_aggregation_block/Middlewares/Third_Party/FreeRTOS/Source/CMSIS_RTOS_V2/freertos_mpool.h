/*
 * freertos_mpool.h
 *
 *  Created on: Apr 12, 2025
 *      Author: Ngo Khanh
 */

#ifndef FREERTOS_MPOOL_H_
#define FREERTOS_MPOOL_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

/* Memory Pool implementation definitions */
#define MPOOL_STATUS              0x5EED0000U

/* Memory Block header */
typedef struct {
  void *next;                   /* Pointer to next block  */
} MemPoolBlock_t;

/* Memory Pool control block */
typedef struct MemPoolDef_t {
  MemPoolBlock_t    *head;      /* Pointer to head block   */
  SemaphoreHandle_t  sem;       /* Pool semaphore handle   */
  uint8_t           *mem_arr;   /* Pool memory array       */
  uint32_t           mem_sz;    /* Pool memory array size  */
  const char        *name;      /* Pointer to name string  */
  uint32_t           bl_sz;     /* Size of a single block  */
  uint32_t           bl_cnt;    /* Number of blocks        */
  uint32_t           n;         /* Block allocation index  */
  volatile uint32_t  status;    /* Object status flags     */
#if (configSUPPORT_STATIC_ALLOCATION == 1)
  StaticSemaphore_t  mem_sem;   /* Semaphore object memory */
#endif
} MemPool_t;

/* No need to hide static object type, just align to coding style */
#define StaticMemPool_t         MemPool_t

/* Define memory pool control block size */
#define MEMPOOL_CB_SIZE         (sizeof(StaticMemPool_t))

/* Define size of the byte array required to create count of blocks of given size */
#define MEMPOOL_ARR_SIZE(bl_count, bl_size) (((((bl_size) + (4 - 1)) / 4) * 4)*(bl_count))

#endif /* FREERTOS_MPOOL_H_ */
