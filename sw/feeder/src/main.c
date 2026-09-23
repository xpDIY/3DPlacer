#include "string.h"
#include "py32f0xx_hal.h"
#include "gparser.h"
#include "uart_control.h"
#include "feeder_control.h"
#include "py32f0xx_bsp_clock.h"
#include "configuration.h"

int idxMotor=0;
static void APP_GPIO_Config(void);
static void APP_SWC_AsInput_Config(void);


// Keep SWD alive briefly after power-up so the debugger can attach before
// SWCLK is reused as the SWC input. Keep this short so the feeder starts
// answering M888 soon after power-up.
#define SWD_GRACE_PERIOD_MS      2000U
#define STARTUP_MOTOR_RUN_MS     5000U

void APP_ErrorHandler(void)
{ 
  while (1);
}


int main(void)
{
  HAL_Init();
  uint32_t boot_tick = HAL_GetTick();
  BSP_HSI_24MHzClockConfig();                
  APP_GPIO_Config();
  //enable motor
  HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_EN,GPIO_PIN_SET);
  // Boot indication: LED1 stays on until the feeder is ready to answer M888.
  HAL_GPIO_WritePin(GPIOA,PIN_LED1,GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB,PIN_LED2,GPIO_PIN_RESET);
  //This is for led push button
  read_feeder_data_from_flash();
  //USART2_Config();
  USART1_Config();
  APP_AdcConfig();
  HAL_HalfDuplex_EnableReceiver(&UartOwHandle);
  for(volatile int i=0;i<1000000;++i); //wait for voltage to stabilize
  init_adc();

  // Keep SWD available for a short window after power-up (no reset pin required).
  // Measured from boot, so the feeder is ready to answer M888 at
  // SWD_GRACE_PERIOD_MS after power-up (the ADC settle above is inside it).
  {
    uint32_t elapsed = HAL_GetTick() - boot_tick;
    if (elapsed < SWD_GRACE_PERIOD_MS)
    {
      HAL_Delay(SWD_GRACE_PERIOD_MS - elapsed);
    }
  }

  // Reuse SWCLK pin as active-low input for cover-tape position detect
  APP_SWC_AsInput_Config();

#if IS_AUTO_FEEDER
  // Power-on motor run: reverse direction and stop early when SWC is pulled low.
  // The loose-part feeder has no tape cover to home, so it skips this and is
  // ready to answer M888 as soon as the SWD grace period ends.
  motor_dir_forward = 0;
  start_motor();
  uint32_t startup_motor_tick = HAL_GetTick();
  while ((HAL_GetTick() - startup_motor_tick) < STARTUP_MOTOR_RUN_MS)
  {
    if (HAL_GPIO_ReadPin(GPIOA, PIN_SWC_IN) == GPIO_PIN_RESET)
    {
      break;
    }
  }
  stop_motor();
  motor_dir_forward = 1;
#endif

  // Ready to answer M888: turn off the boot indication.
  HAL_GPIO_WritePin(GPIOA, PIN_LED1, GPIO_PIN_RESET);

  //start_motor();
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
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);    

  GPIO_InitStruct.Pin = PIN_PART_LED;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);    

  GPIO_InitStruct.Pin = PIN_PART_DET;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  // PA12 (PD1): normally HIGH from interrupter, LOW when beam interrupted.
  // Keep internal pulldown so released line is read as LOW.
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  //for switch control
  GPIO_InitStruct.Pin = PIN_SW_B;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);    

}

static void APP_SWC_AsInput_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Pin = PIN_SWC_IN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
