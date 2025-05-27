Project Title: Data Aggregation Block

A. Basic Functionality
The "Data Aggregation Block" project implements a real-time MAVLink V1 message handling system using DMA, UART, and CMSIS-RTOS V2 on STM32. The core functionality includes:

1. Receiving MAVLink V1 Packets
- UART1 and UART2 receive data via DMA (circular mode).
- Received bytes are pushed into dedicated FIFO buffers using IDLE interrupts.

2. Parsing MAVLink Messages (Task UART1/UART2)
- Task_UART1 and Task_UART2 are notified by ISR when new data is available.

- Each task reads data from its corresponding FIFO buffer, parses the MAVLink message, extracts key payload content, and forwards the result to a shared FreeRTOS message queue (QUEUE_t msg).
3. Displaying Parsed Output (Task UART3)
- Task_UART3 listens on the message queue.
- When a new message arrives, it transmits the content to a PC terminal via UART3, allowing real-time monitoring of the aggregated data.

B. Project Structure
1. Custom Libraries
- fifo.c, fifo.h => Lightweight circular FIFO buffer for non-blocking byte storage from DMA.
- MAVLinkV1.c, MAVLinkV1.h => Minimal MAVLink V1 parser/encoder for embedded systems.

2. Application Source Files
- main.c => Entry point, peripheral initialization, and kernel start.
- stm32f1xx_it.c => ISR definitions, including UART IDLE interrupt handling and DMA pointer tracking.
- freertos.c => Definitions of Task_UART1, Task_UART2, and Task_UART3 using CMSIS-RTOS V2 API.

3. Auto-Generated Hardware Abstraction Files (by STM32CubeMX)
dma.c, gpio.c, usart.c, etc.
=> Peripheral initialization and hardware configuration.

