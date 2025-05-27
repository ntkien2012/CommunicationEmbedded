/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "cmsis_os.h"         // CMSIS-RTOS V2: osThreadId_t, vTaskNotifyGiveFromISR
#include "fifo.h"             // Định nghĩa FIFO_Write và FIFO_Buffer_t
#include "usart.h"            // Định nghĩa extern huart1
#include "dma.h"              // Định nghĩa extern hdma_usart1_rx
#include "FreeRTOS.h"         // Định nghĩa BaseType_t
#include "task.h"             // vTaskNotifyGiveFromISR, portYIELD_FROM_ISR
#define UART_RX_BUFFER_SIZE  128
extern FIFO_Buffer_t uart1_fifo;
extern FIFO_Buffer_t uart2_fifo;

extern uint8_t uart1_dma_rx_buf[];
extern uint8_t uart2_dma_rx_buf[];

extern TaskHandle_t UART1TaskHandle;
extern TaskHandle_t UART2TaskHandle;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim4;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  HAL_UART_Transmit(&huart3, (uint8_t*)"HARDFAULT!\r\n", 13, 100);

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel5 global interrupt.
  */
void DMA1_Channel5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel5_IRQn 0 */

  /* USER CODE END DMA1_Channel5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA1_Channel5_IRQn 1 */

  /* USER CODE END DMA1_Channel5_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel6 global interrupt.
  */
void DMA1_Channel6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel6_IRQn 0 */

  /* USER CODE END DMA1_Channel6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
  /* USER CODE BEGIN DMA1_Channel6_IRQn 1 */

  /* USER CODE END DMA1_Channel6_IRQn 1 */
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */

  /* USER CODE END TIM4_IRQn 0 */
  HAL_TIM_IRQHandler(&htim4);
  /* USER CODE BEGIN TIM4_IRQn 1 */

  /* USER CODE END TIM4_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */

void USART1_IRQHandler(void)
{
	  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
	  {
	    __HAL_UART_CLEAR_IDLEFLAG(&huart1);

	    static uint16_t last_dma_pos_uart1 = 0;

	    uint16_t dma_pos = UART_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
	    uint16_t len = (dma_pos >= last_dma_pos_uart1) ? (dma_pos - last_dma_pos_uart1) : (UART_RX_BUFFER_SIZE - last_dma_pos_uart1 + dma_pos);

	    if (len > 0)
	    {
	      if (last_dma_pos_uart1 + len <= UART_RX_BUFFER_SIZE) {
	        FIFO_Write(&uart1_fifo, &uart1_dma_rx_buf[last_dma_pos_uart1], len);
	      } else {
	        uint16_t first_chunk = UART_RX_BUFFER_SIZE - last_dma_pos_uart1;
	        FIFO_Write(&uart1_fifo, &uart1_dma_rx_buf[last_dma_pos_uart1], first_chunk);
	        FIFO_Write(&uart1_fifo, &uart1_dma_rx_buf[0], len - first_chunk);
	      }

	      if (UART1TaskHandle != NULL) {
	          BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	          vTaskNotifyGiveFromISR(UART1TaskHandle, &xHigherPriorityTaskWoken);
	          portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	      }
	    }

	    last_dma_pos_uart1 = dma_pos;
	  }

	  HAL_UART_IRQHandler(&huart1);
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE))
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart2);
    static uint16_t last_dma_pos = 0;

    uint16_t dma_pos = UART_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);
    uint16_t len = (dma_pos >= last_dma_pos) ? (dma_pos - last_dma_pos) : (UART_RX_BUFFER_SIZE - last_dma_pos + dma_pos);

    if (len > 0)
    {
      if (last_dma_pos + len <= UART_RX_BUFFER_SIZE) {
        FIFO_Write(&uart2_fifo, &uart2_dma_rx_buf[last_dma_pos], len);
      } else {
        uint16_t first_chunk = UART_RX_BUFFER_SIZE - last_dma_pos;
        FIFO_Write(&uart2_fifo, &uart2_dma_rx_buf[last_dma_pos], first_chunk);
        FIFO_Write(&uart2_fifo, &uart2_dma_rx_buf[0], len - first_chunk);
      }

      if (UART2TaskHandle != NULL) {
          BaseType_t xHigherPriorityTaskWoken = pdFALSE;
          vTaskNotifyGiveFromISR(UART2TaskHandle, &xHigherPriorityTaskWoken);
          portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      }
    }

    last_dma_pos = dma_pos;
  }

  HAL_UART_IRQHandler(&huart2);
}


/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(B1_Pin);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/* USER CODE BEGIN 1 */
/**
  * @brief Callback khi DMA nhận xong nửa buffer đầu tiên (0 → size/2)
  */
//void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart->Instance == USART1)
//    {
//        FIFO_Write(&uart1_fifo, &uart1_dma_rx_buf[0], UART_RX_BUFFER_SIZE / 2);
//
//        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//        vTaskNotifyGiveFromISR(UART1TaskHandle, &xHigherPriorityTaskWoken);
//        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//    }
//}
//
///**
//  * @brief Callback khi DMA nhận xong nửa buffer sau (size/2 → size)
//  */
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart->Instance == USART1)
//    {
//        FIFO_Write(&uart1_fifo, &uart1_dma_rx_buf[UART_RX_BUFFER_SIZE / 2], UART_RX_BUFFER_SIZE / 2);
//
//        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//        vTaskNotifyGiveFromISR(UART1TaskHandle, &xHigherPriorityTaskWoken);
//        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//    }
//}

/* USER CODE END 1 */
