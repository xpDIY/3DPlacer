#ifndef _UART_CONTROL_H
#define _UART_CONTROL_H

#include "py32f0xx_hal.h"

#define CONTROL_USART USART2
#define OW_USART USART1

#define CONTROL_USART_BAUDRATE            115200
#define USART2_CLK_ENABLE()                __HAL_RCC_USART2_CLK_ENABLE()
#define USART1_CLK_ENABLE()                __HAL_RCC_USART1_CLK_ENABLE()
#define USART_RX_GPIO_PORT                GPIOA
#define USART_RX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART_RX_PIN                      GPIO_PIN_3
#define USART_RX_AF                       GPIO_AF4_USART2

#define USART_TX_GPIO_PORT                GPIOA
#define USART_TX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART_TX_PIN                      GPIO_PIN_2
#define USART_TX_AF                       GPIO_AF4_USART2

#define USART_IRQHandler                  USART2_IRQHandler
#define USART_IRQ                         USART2_IRQn


#define USART1_TX_GPIO_PORT                GPIOA
#define USART1_TX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART1_TX_PIN                      GPIO_PIN_7
#define USART1_TX_AF                       GPIO_AF8_USART1
#define USART1_IRQHandler                  USART1_IRQHandler
#define USART1_IRQ                         USART1_IRQn


void USART2_Config(void);
void USART1_Config(void);
void start_receiving_one_wire(UART_HandleTypeDef *UartHandle);
void start_sending_one_wire(UART_HandleTypeDef *UartHandle);
uint8_t process_uart_data(uint8_t receive, uint8_t forward);

extern UART_HandleTypeDef UartHandle;
extern UART_HandleTypeDef UartOwHandle;
extern uint8_t aRxBuffer[2];
extern char aMsg[100];
extern uint8_t bRxBuffer[2];
extern char bMsg[255];
extern uint8_t receiveOw;
#endif