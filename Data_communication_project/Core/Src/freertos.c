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
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "fifo.h"
#include "usart.h"
#include "MAVLinkV1.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
extern FIFO_Buffer_t uart1_fifo;
extern FIFO_Buffer_t uart2_fifo;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */
typedef struct {
  uint8_t data[128];
  uint16_t len;
} QUEUE_t;

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
uint32_t defaultTaskBuffer[ 128 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &defaultTaskBuffer[0],
  .stack_size = sizeof(defaultTaskBuffer),
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
  .priority = (osPriority_t) osPriorityAboveNormal,
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
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for UARTQueue */
osMessageQueueId_t UARTQueueHandle;
uint8_t UARTQueueBuffer[ 32 * sizeof( QUEUE_t ) ];
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
  UARTQueueHandle = osMessageQueueNew (16, sizeof(QUEUE_t), &UARTQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of UART1Task */
  UART1TaskHandle = osThreadNew(StartUART1Task, NULL, &UART1Task_attributes);

  /* creation of UART2Task */
  UART2TaskHandle = osThreadNew(StartUART2Task, NULL, &UART2Task_attributes);

  /* creation of UART3Task */
  UART3TaskHandle = osThreadNew(StartUART3Task, NULL, &UART3Task_attributes);

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
    static mavlink_message_t mav_msg;
    static mavlink_state_t mav_state = MAVLINK_STATE_IDLE;
    QUEUE_t msg;
    uint8_t byte;
    uint32_t ulNotificationValue = 0;

    for (;;)
    {
        xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY);

        do {
            int available = FIFO_Available(&uart1_fifo);
            for (int i = 0; i < available; i++)
            {
                FIFO_Read(&uart1_fifo, &byte, 1);

                if (mavlink_parse_byte(&mav_state, &mav_msg, byte))
                {
                    int payload_len = mav_msg.len;
                    if (payload_len > sizeof(msg.data) - 32) payload_len = sizeof(msg.data) - 32;

                    char payload_str[64];
                    memcpy(payload_str, mav_msg.payload, payload_len);
                    payload_str[payload_len] = '\0';

                    msg.len = snprintf((char*)msg.data, sizeof(msg.data), "UART1: %s\r\n", payload_str);
                    osMessageQueuePut(UARTQueueHandle, &msg, 0, 20);
                }
            }
        } while (FIFO_Available(&uart1_fifo) > 0);
    }
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

    static mavlink_message_t mav_msg;
    static mavlink_state_t mav_state = MAVLINK_STATE_IDLE;
    QUEUE_t msg;
    uint8_t byte;
    uint32_t ulNotificationValue = 0;

    for (;;)
    {
        xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY);

        do {
            int available = FIFO_Available(&uart2_fifo);
            for (int i = 0; i < available; i++)
            {
                FIFO_Read(&uart2_fifo, &byte, 1);

                if (mavlink_parse_byte(&mav_state, &mav_msg, byte))
                {
                    int payload_len = mav_msg.len;
                    if (payload_len > sizeof(msg.data) - 32) payload_len = sizeof(msg.data) - 32;

                    char payload_str[64];
                    memcpy(payload_str, mav_msg.payload, payload_len);
                    payload_str[payload_len] = '\0';

                    msg.len = snprintf((char*)msg.data, sizeof(msg.data), "UART2: %s\r\n", payload_str);
                    osMessageQueuePut(UARTQueueHandle, &msg, 0, 20);
                }
            }
        } while (FIFO_Available(&uart2_fifo) > 0);
    }
}


void StartUART3Task(void *argument)
{
    QUEUE_t msg;

    for (;;)
    {
        if (osMessageQueueGet(UARTQueueHandle, &msg, NULL, osWaitForever) == osOK)
        {
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); // test hoạt động
            HAL_UART_Transmit(&huart3, msg.data, msg.len, 50);
        }
    }
}
/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

