#include "string.h"
#include "py32f0xx_hal.h"
#include "gparser.h"
#include "uart_control.h"
#include "feeder_control.h"
#include "py32f0xx_bsp_clock.h"
#include "configuration.h"

int idxMotor=0;
static void APP_GPIO_Config(void);

void APP_ErrorHandler(void)
{ 
  while (1);
}


int main(void)
{
  HAL_Init();                  
  BSP_HSI_24MHzClockConfig();                
  APP_GPIO_Config();
  //enable motor
  HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_EN,GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA,PIN_LED1,GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA,PIN_LED2,GPIO_PIN_SET);
  //This is for led push button
  HAL_GPIO_WritePin(GPIOA,PIN_LED2,GPIO_PIN_RESET);
  read_feeder_data_from_flash();
  //USART2_Config();
  USART1_Config();
  APP_AdcConfig();
  HAL_HalfDuplex_EnableReceiver(&UartOwHandle);
  /*
  if (HAL_UART_Receive_IT(&UartOwHandle, (uint8_t *)aRxBuffer, 1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
  */
  while (1)
  {
    process_ow_data();
    process_feeder();
  }
}


static void APP_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Pin = PIN_MOTOR_EN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  //adc pin for gpio 4
  GPIO_InitStruct.Pin = PIN_RPOS;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  
//adc pin for gpio 5
  GPIO_InitStruct.Pin = PIN_CPOS;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  

  GPIO_InitStruct.Pin = PIN_MOTOR_A1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = PIN_MOTOR_A2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = PIN_LED1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  

  GPIO_InitStruct.Pin = PIN_LED2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);    

  GPIO_InitStruct.Pin = PIN_PART_DET;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);    

  //for switch control
  GPIO_InitStruct.Pin = PIN_SW_B;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);    

}



