/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "usart.h"
#include "fifo.h"
#include "mavlink.h"
#include "common/mavlink.h"   // hoặc dialect tương ứng
extern osMessageQueueId_t UARTQueueHandle;
extern fifo_t uart1_fifo;
extern fifo_t uart2_fifo;
extern UART_HandleTypeDef huart3;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UART1Task */
osThreadId_t UART1TaskHandle;
uint32_t UART1TaskBuffer[ 128 ];
osStaticThreadDef_t UART1TaskControlBlock;
const osThreadAttr_t UART1Task_attributes = {
  .name = "UART1Task",
  .cb_mem = &UART1TaskControlBlock,
  .cb_size = sizeof(UART1TaskControlBlock),
  .stack_mem = &UART1TaskBuffer[0],
  .stack_size = sizeof(UART1TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for UART2Task */
osThreadId_t UART2TaskHandle;
uint32_t UART2TaskBuffer[ 128 ];
osStaticThreadDef_t UART2TaskControlBlock;
const osThreadAttr_t UART2Task_attributes = {
  .name = "UART2Task",
  .cb_mem = &UART2TaskControlBlock,
  .cb_size = sizeof(UART2TaskControlBlock),
  .stack_mem = &UART2TaskBuffer[0],
  .stack_size = sizeof(UART2TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for UART3Task */
osThreadId_t UART3TaskHandle;
uint32_t UART3TaskBuffer[ 128 ];
osStaticThreadDef_t UART3TaskControlBlock;
const osThreadAttr_t UART3Task_attributes = {
  .name = "UART3Task",
  .cb_mem = &UART3TaskControlBlock,
  .cb_size = sizeof(UART3TaskControlBlock),
  .stack_mem = &UART3TaskBuffer[0],
  .stack_size = sizeof(UART3TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for UARTQueue */
osMessageQueueId_t UARTQueueHandle;
uint8_t UARTQueueBuffer[ 10 * sizeof( QUEUE_t ) ];
osStaticMessageQDef_t UARTQueueControlBlock;
const osMessageQueueAttr_t UARTQueue_attributes = {
  .name = "UARTQueue",
  .cb_mem = &UARTQueueControlBlock,
  .cb_size = sizeof(UARTQueueControlBlock),
  .mq_mem = &UARTQueueBuffer,
  .mq_size = sizeof(UARTQueueBuffer)
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartUART1Task(void *argument);
void StartUART2Task(void *argument);
void StartUART3Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of UARTQueue */
  UARTQueueHandle = osMessageQueueNew (10, sizeof(QUEUE_t), &UARTQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

//  /* creation of UART1Task */
  UART1TaskHandle = osThreadNew(StartUART1Task, NULL, &UART1Task_attributes);
  if (UART1TaskHandle == NULL) Error_Handler();
//
//  /* creation of UART2Task */
  UART2TaskHandle = osThreadNew(StartUART2Task, NULL, &UART2Task_attributes);
  if (UART2TaskHandle == NULL) Error_Handler();
//
//  /* creation of UART3Task */
  UART3TaskHandle = osThreadNew(StartUART3Task, NULL, &UART3Task_attributes);
  if (UART3TaskHandle == NULL) Error_Handler();
//
  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartUART1Task */
/**
* @brief Function implementing the UART1Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUART1Task */
void StartUART1Task(void *argument)
{
  /* USER CODE BEGIN StartUART1Task */
	mavlink_message_t msg;
	mavlink_status_t status;
	uint8_t c;
	QUEUE_t q_msg;

  /* Infinite loop */
  for(;;)
  {
	  // Đợi tín hiệu từ ISR DMA (đã push dữ liệu vào FIFO)
	  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	    // Xử lý FIFO
	    while (!fifo_is_empty(&uart1_fifo)) {
	      fifo_pop(&uart1_fifo, &c);

	      // Parse MAVLink từng byte
	      if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
	        // Gửi bản tin vào queue nếu hợp lệ
	        q_msg.msg = msg;
	        q_msg.src = 1; // UART1

	        osMessageQueuePut(UARTQueueHandle, &q_msg, 0, 0);
	      }
	    }
  }
  /* USER CODE END StartUART1Task */
}

/* USER CODE BEGIN Header_StartUART2Task */
/**
* @brief Function implementing the UART2Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUART2Task */
void StartUART2Task(void *argument)
{
  /* USER CODE BEGIN StartUART2Task */
	  mavlink_message_t msg;
	  mavlink_status_t status;
	  uint8_t c;
	  QUEUE_t q_msg;

  /* Infinite loop */
  for(;;)
  {
	    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	    while (!fifo_is_empty(&uart2_fifo)) {
	      fifo_pop(&uart2_fifo, &c);

	      if (mavlink_parse_char(MAVLINK_COMM_1, c, &msg, &status)) {
	        q_msg.msg = msg;
	        q_msg.src = 2; // UART2

	        osMessageQueuePut(UARTQueueHandle, &q_msg, 0, 0);
	      }
	    }
  }
  /* USER CODE END StartUART2Task */
}

/* USER CODE BEGIN Header_StartUART3Task */
/**
* @brief Function implementing the UART3Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUART3Task */
void StartUART3Task(void *argument)
{
  /* USER CODE BEGIN StartUART3Task */
	  QUEUE_t q_msg;
	  uint8_t tx_buf[MAVLINK_MAX_PACKET_LEN];
	  uint16_t len;
	  for(;;)
	  {
	    // Chờ có message từ queue
	    if (osMessageQueueGet(UARTQueueHandle, &q_msg, 0, osWaitForever) == osOK)
	    {
	      // Chuyển message → buffer byte
	      len = mavlink_msg_to_send_buffer(tx_buf, &q_msg.msg);

	      // Gửi ra UART3
	      HAL_UART_Transmit(&huart3, tx_buf, len, HAL_MAX_DELAY);
	    }
	  }
  /* USER CODE END StartUART3Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

