#include "main.h"
#include "py32f0xx_it.h"

/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
  if(LL_TIM_IsActiveFlag_UPDATE(TIM1) && LL_TIM_IsEnabledIT_UPDATE(TIM1))
  {
    LL_TIM_ClearFlag_UPDATE(TIM1);
    APP_TIM1UpdateCallback();
  }
}