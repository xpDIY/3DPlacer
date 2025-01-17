#ifndef _UART_CONTROL_H
#define _UART_CONTROL_H

#include "py32f0xx_hal.h"

#define CONTROL_USART USART2
#define OW_USART USART1

#define CONTROL_USART_BAUDRATE            115200
//#define USART2_CLK_ENABLE()                __HAL_RCC_USART2_CLK_ENABLE()
#define USART1_CLK_ENABLE()                __HAL_RCC_USART1_CLK_ENABLE()
//#define USART_RX_GPIO_PORT                GPIOA
//#define USART_RX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()
//#define USART_RX_PIN                      GPIO_PIN_1
//#define USART_RX_AF                       GPIO_AF9_USART2

//#define USART_TX_GPIO_PORT                GPIOA
//#define USART_TX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()
//#define USART_TX_PIN                      GPIO_PIN_0
//#define USART_TX_AF                       GPIO_AF9_USART2

//#define USART_IRQHandler                  USART2_IRQHandler
//#define USART_IRQ                         USART2_IRQn


#define USART1_TX_GPIO_PORT                GPIOA
#define USART1_TX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART1_TX_PIN                      PIN_OW
#define USART1_TX_AF                       GPIO_AF8_USART1
#define USART1_IRQHandler                  USART1_IRQHandler
#define USART1_IRQ                         USART1_IRQn


//void USART2_Config(void);
void USART1_Config(void);
void APP_AdcConfig(void);
void process_ow_data();
void PollPos(uint32_t *rpos, uint32_t *cpos);
extern UART_HandleTypeDef UartHandle;
extern UART_HandleTypeDef UartOwHandle;
#endif