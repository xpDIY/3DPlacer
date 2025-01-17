/**
  ******************************************************************************
  * @file    py32f002b_hal_rcc.c
  * @author  MCU Application Team
  * @brief   RCC HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Reset and Clock Control (RCC) peripheral:
  *           + Initialization and de-initialization functions
  *           + Peripheral Control functions
  *
  @verbatim
  ==============================================================================
                      ##### RCC specific features #####
  ==============================================================================
    [..]
      After reset the device is running from High Speed Internal oscillator
      (from 4 MHz to reach 24MHz) with Flash 0 wait state. Flash prefetch buffer,
      D-Cache and I-Cache are disabled, and all peripherals are off except internal
      SRAM, Flash and JTAG.

      (+) There is no prescaler on High speed (AHB) and Low speed (APB) busses:
          all peripherals mapped on these busses are running at HSI speed.
      (+) The clock for all peripherals is switched off, except the SRAM and FLASH.
      (+) All GPIOs are in analog mode, except the JTAG pins which
          are assigned to be used for debug purpose.

    [..]
      Once the device started from reset, the user application has to:
      (+) Configure the clock source to be used to drive the System clock
          (if the application needs higher frequency/performance)
      (+) Configure the System clock frequency and Flash settings
      (+) Configure the AHB and APB busses prescalers
      (+) Enable the clock for the peripheral(s) to be used
      (+) Configure the clock source(s) for peripherals which clocks are not
          derived from the System clock (RTC, ADC, RNG, HSTIM)

  @endverbatim

  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "py32f002b_hal.h"

/** @addtogroup PY32F002B_HAL_Driver
  * @{
  */

/** @defgroup RCC RCC
  * @brief RCC HAL module driver
  * @{
  */

#ifdef HAL_RCC_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup RCC_Private_Constants RCC Private Constants
 * @{
 */
#define CLOCKSWITCH_TIMEOUT_VALUE  (5000U) /* 5 s    */
#define HSI_TIMEOUT_VALUE          (2U)    /* 2 ms (minimum Tick + 1) */
#define LSI_TIMEOUT_VALUE          (2U)    /* 2 ms (minimum Tick + 1) */
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup RCC_Private_Variables RCC Private Variables
  * @{
  */

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup RCC_Exported_Functions RCC Exported Functions
  * @{
  */

/** @defgroup RCC_Exported_Functions_Group1 Initialization and de-initialization functions
  *  @brief    Initialization and Configuration functions
  *
  @verbatim
 ===============================================================================
           ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]
      This section provides functions allowing to configure the internal and external oscillators
      (HSE, HSI, LSE, LSI, CSS and MCO) and the System busses clocks (SYSCLK, AHB, APB)

    [..] Internal/external clock and PLL configuration
         (+) HSI (high-speed internal): 8 MHz factory-trimmed RC used directly or through
             the PLL as System clock source.

         (+) LSI (low-speed internal): 32 KHz low consumption RC used as IWDG and/or RTC
             clock source.

         (+) HSE (high-speed external): 4 to 48 MHz crystal oscillator used directly or
             through the PLL as System clock source. Can be used also optionally as RTC clock source.

         (+) LSE (low-speed external): 32.768 KHz oscillator used optionally as RTC clock source.

         (+) PLL (clocked by HSI, HSE) providing up to three independent output clocks:


         (+) CSS (Clock security system): once enabled, if a HSE  clock failure occurs
            (HSE used directly or through PLL as System clock source), the System clock
             is automatically switched respectively to HSI or LSI and an interrupt is generated
             if enabled. The interrupt is linked to the Cortex-M0+ NMI (Non-Maskable Interrupt)
             exception vector.

         (+) MCO (microcontroller clock output): used to output LSI, HSI, LSE, HSE or
             main PLL clock (through a configurable prescaler) on PA8 pin.

    [..] System, AHB and APB busses clocks configuration
         (+) Several clock sources can be used to drive the System clock (SYSCLK): HSI,
             HSE, LSI, LSE and main PLL.
             The AHB clock (HCLK) is derived from System clock through configurable
             prescaler and used to clock the CPU, memory and peripherals mapped
             on AHB bus (DMA, GPIO...).and APB (PCLK1) clock is derived
             from AHB clock through configurable prescalers and used to clock
             the peripherals mapped on these busses. You can use
             "@ref HAL_RCC_GetSysClockFreq()" function to retrieve the frequencies of these clocks.

         -@- All the peripheral clocks are derived from the System clock (SYSCLK) except:

            (+@) RTC: the RTC clock can be derived either from the LSI, LSE or HSE clock
                divided by 2 to 31.
                You have to use @ref __HAL_RCC_RTC_ENABLE() and @ref HAL_RCCEx_PeriphCLKConfig() function
                 to configure this clock.

            (+@) RNG(*) requires a frequency equal or lower than 48 MHz.
                 This clock is derived from the main PLL or HSI or System clock.
                 (*) available on certain devices only

            (+@) IWDG clock which is always the LSI clock.


         (+) The maximum frequency of the SYSCLK, HCLK, PCLK is 48 MHz.
             Depending on the device voltage range, the maximum frequency should be
             adapted accordingly.

  @endverbatim



   * @{
  */

/**
  * @brief  Reset the RCC clock configuration to the default reset state.
  * @note   The default reset state of the clock configuration is given below:
  *            - HSI ON and used as system clock source
  *            - HSE OFF
  *            - AHB and APB prescaler set to 1.
  *            - CSS, MCO1 OFF
  *            - All interrupts disabled
  * @note   This function does not modify the configuration of the
  *            - Peripheral clocks
  *            - LSI, LSE and RTC clocks
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RCC_DeInit(void)
{
  uint32_t tickstart;

  /* Get Start Tick*/
  tickstart = HAL_GetTick();

  /* Set HSION bit to the reset value */
  SET_BIT(RCC->CR, RCC_CR_HSION);

  /* Wait till HSI is ready */
  while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
  {
    if ((HAL_GetTick() - tickstart) > HSI_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Set ICSCR to the reset value */
  RCC->ICSCR = 0x00FF10FF;

  /* Get Start Tick*/
  tickstart = HAL_GetTick();

  /* Reset CFGR register (HSI is selected as system clock source) */
  RCC->CFGR = 0x00000000u;

  /* Wait till HSI is ready */
  while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != 0U)
  {
    if ((HAL_GetTick() - tickstart) > CLOCKSWITCH_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Clear CR register in 2 steps: first to clear HSEON in case bypass was enabled */
  RCC->CR = RCC_CR_HSION;

  /* Then again to HSEBYP in case bypass was enabled */
  RCC->CR = RCC_CR_HSION;

  /* Disable all interrupts */
  RCC->CIER = 0x00000000u;

  /* Clear all flags */
  RCC->CICR = 0xFFFFFFFFu;

  /* Update the SystemCoreClock global variable */
  SystemCoreClock = HSI_VALUE;

  /* Adapt Systick interrupt period */
  if (HAL_InitTick(uwTickPrio) != HAL_OK)
  {
    return HAL_ERROR;
  }
  else
  {
    return HAL_OK;
  }
}

/**
  * @brief  Initialize the RCC Oscillators according to the specified parameters in the
  *         @ref RCC_OscInitTypeDef.
  * @param  RCC_OscInitStruct pointer to a @ref RCC_OscInitTypeDef structure that
  *         contains the configuration information for the RCC Oscillators.
  * @note   Transition HSE Bypass to HSE On and HSE On to HSE Bypass are not
  *         supported by this function. User should request a transition to HSE Off
  *         first and then to HSE On or HSE Bypass.
  * @note   Transition LSE Bypass to LSE On and LSE On to LSE Bypass are not
  *         supported by this function. User should request a transition to LSE Off
  *         first and then to LSE On or LSE Bypass.
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef  *RCC_OscInitStruct)
{
  uint32_t tickstart;
  uint32_t temp_sysclksrc;

  /* Check Null pointer */
  if (RCC_OscInitStruct == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_RCC_OSCILLATORTYPE(RCC_OscInitStruct->OscillatorType));

  /*------------------------------- HSE Configuration ------------------------*/
  if (((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_HSE) == RCC_OSCILLATORTYPE_HSE)
  {
    /* Check the parameters */
    assert_param(IS_RCC_HSE(RCC_OscInitStruct->HSEState));

    temp_sysclksrc = __HAL_RCC_GET_SYSCLK_SOURCE();      

    if (temp_sysclksrc == RCC_CFGR_SWS_HSE)
    {
      if (RCC_OscInitStruct->HSEState == RCC_HSE_BYPASS_DISABLE)
      {
        return HAL_ERROR;
      }
    }
    else
    {
      /* Set the new HSE configuration ---------------------------------------*/
      __HAL_RCC_HSE_CONFIG(RCC_OscInitStruct->HSEState);
    }
  }
  /*----------------------------- HSI Configuration --------------------------*/
  if (((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_HSI) == RCC_OSCILLATORTYPE_HSI)
  {
    /* Check the parameters */
    assert_param(IS_RCC_HSI(RCC_OscInitStruct->HSIState));
    assert_param(IS_RCC_HSI_CALIBRATION_VALUE(RCC_OscInitStruct->HSICalibrationValue));
    assert_param(IS_RCC_HSIDIV(RCC_OscInitStruct->HSIDiv));

    /* Check if HSI is used as system clock */
    temp_sysclksrc = __HAL_RCC_GET_SYSCLK_SOURCE();

    if (temp_sysclksrc == RCC_CFGR_SWS_HSISYS)
    {
      /* When HSI is used as system clock it can not be disabled */
      if ((READ_BIT(RCC->CR, RCC_CR_HSIRDY) != 0U) && (RCC_OscInitStruct->HSIState == RCC_HSI_OFF))
      {
        return HAL_ERROR;
      }
      /* Otherwise, just the calibration is allowed */
      else
      {
        /* Adjusts the Internal High Speed oscillator (HSI) calibration value.*/
        __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(RCC_OscInitStruct->HSICalibrationValue);

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSI is ready */
        while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
        {
          if ((HAL_GetTick() - tickstart) > HSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }

        if (temp_sysclksrc == RCC_CFGR_SWS_HSISYS)
        {
          /* Adjust the HSI division factor */
          __HAL_RCC_HSI_CONFIG(RCC_OscInitStruct->HSIDiv);

          /* Update the SystemCoreClock global variable with HSISYS value  */
          SystemCoreClock = (HAL_RCC_GetSysClockFreq() >> ((AHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos]) & 0x1FU));
        }

        /* Adapt Systick interrupt period */
        if (HAL_InitTick(uwTickPrio) != HAL_OK)
        {
          return HAL_ERROR;
        }
      }
    }
    else
    {
      /* Check the HSI State */
      if (RCC_OscInitStruct->HSIState != RCC_HSI_OFF)
      {
        /* Configure the HSI division factor */
        __HAL_RCC_HSI_CONFIG(RCC_OscInitStruct->HSIDiv);

        /* Enable the Internal High Speed oscillator (HSI). */
        __HAL_RCC_HSI_ENABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSI is ready */
        while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
        {
          if ((HAL_GetTick() - tickstart) > HSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }

        /* Adjusts the Internal High Speed oscillator (HSI) calibration value.*/
        __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(RCC_OscInitStruct->HSICalibrationValue);
      }
      else
      {
        /* Disable the Internal High Speed oscillator (HSI). */
        __HAL_RCC_HSI_DISABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSI is disabled */
        while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) != 0U)
        {
          if ((HAL_GetTick() - tickstart) > HSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
  }
  /*------------------------------ LSI Configuration -------------------------*/
  if (((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_LSI) == RCC_OSCILLATORTYPE_LSI)
  {
    /* Check the parameters */
    assert_param(IS_RCC_LSI(RCC_OscInitStruct->LSIState));

    /* Check if LSI is used as system clock */
    if (__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_CFGR_SWS_LSI)
    {
      /* When LSI is used as system clock it will not be disabled */
      if ((((RCC->CSR) & RCC_CSR_LSIRDY) != 0U) && (RCC_OscInitStruct->LSIState == RCC_LSI_OFF))
      {
        return HAL_ERROR;
      }
      else
      {
        /* Adjusts the Internal Low Speed oscillator (LSI) calibration value.*/
        __HAL_RCC_LSI_CALIBRATIONVALUE_ADJUST(RCC_OscInitStruct->LSICalibrationValue);
        
        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till LSI is ready */
        while (READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) == 0U)
        {
          if ((HAL_GetTick() - tickstart) > LSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
        
        temp_sysclksrc = __HAL_RCC_GET_SYSCLK_SOURCE();
        
        if (temp_sysclksrc == RCC_CFGR_SWS_LSI)
        {
          /* Update the SystemCoreClock global variable with LSI value  */
          SystemCoreClock = (HAL_RCC_GetSysClockFreq() >> ((AHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos]) & 0x1FU));
        }
        
        /* Adapt Systick interrupt period */
        if (HAL_InitTick(uwTickPrio) != HAL_OK)
        {
          return HAL_ERROR;
        }
      }
    }
    else
    {
      /* Check the LSI State */
      if (RCC_OscInitStruct->LSIState != RCC_LSI_OFF)
      {
        /* Enable the Internal Low Speed oscillator (LSI). */
        __HAL_RCC_LSI_ENABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till LSI is ready */
        while (READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) == 0U)
        {
          if ((HAL_GetTick() - tickstart) > LSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
        
        /* Adjusts the Internal Low Speed oscillator (LSI) calibration value.*/
        __HAL_RCC_LSI_CALIBRATIONVALUE_ADJUST(RCC_OscInitStruct->LSICalibrationValue);
      }
      else
      {
        /* Disable the Internal Low Speed oscillator (LSI). */
        __HAL_RCC_LSI_DISABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till LSI is disabled */
        while (READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) != 0U)
        {
          if ((HAL_GetTick() - tickstart) > LSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
  }

  /*------------------------------ LSE Configuration -------------------------*/
  if (((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_LSE) == RCC_OSCILLATORTYPE_LSE)
  {
    /* Check the parameters */
    assert_param(IS_RCC_LSE(RCC_OscInitStruct->LSEState));

    /* When the LSE is used as system clock, it is not allowed disable it */
    if (__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_CFGR_SWS_LSE)
    {
      if ((((RCC->BDCR) & RCC_BDCR_LSERDY) != 0U) && (RCC_OscInitStruct->LSEState == RCC_LSE_OFF))
      {
        return HAL_ERROR;
      }
    }
    else
    {
      /* Set driver factor of the LSE*/
      if (RCC_OscInitStruct->LSEState != RCC_LSE_OFF)
      {
        MODIFY_REG(RCC->ECSCR, RCC_ECSCR_LSE_DRIVER_Msk, RCC_OscInitStruct->LSEDriver);
      }
      /* Set the new LSE configuration -----------------------------------------*/
      __HAL_RCC_LSE_CONFIG(RCC_OscInitStruct->LSEState);

      /* Check the LSE State */
      if (RCC_OscInitStruct->LSEState != RCC_LSE_OFF)
      {
#ifndef RCC_NO_DETECT_LSE_READY
        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till LSE is ready */
        while (READ_BIT(RCC->BDCR, RCC_BDCR_LSERDY) == 0U)
        {
          if ((HAL_GetTick() - tickstart) > RCC_LSE_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
#endif
      }
      else
      {
        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till LSE is disabled */
        while (READ_BIT(RCC->BDCR, RCC_BDCR_LSERDY) != 0U)
        {
          if ((HAL_GetTick() - tickstart) > RCC_LSE_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
  }

  return HAL_OK;
}

/**
  * @brief  Initialize the CPU, AHB and APB busses clocks according to the specified
  *         parameters in the RCC_ClkInitStruct.
  * @param  RCC_ClkInitStruct  pointer to a @ref RCC_ClkInitTypeDef structure that
  *         contains the configuration information for the RCC peripheral.
  * @param  FLatency  FLASH Latency
  *          This parameter can be one of the following values:
  *            @arg FLASH_LATENCY_0   FLASH 0 Latency cycle
  *            @arg FLASH_LATENCY_1   FLASH 1 Latency cycle
  *
  * @note   The SystemCoreClock CMSIS variable is used to store System Clock Frequency
  *         and updated by @ref HAL_RCC_GetHCLKFreq() function called within this function
  *
  * @note   The HSI is used by default as system clock source after
  *         startup from Reset, wake-up from STANDBY mode. After restart from Reset,
  *         the HSI frequency is set to 4 MHz, then it reaches its default value 8 MHz.
  *
  * @note   The HSI can be selected as system clock source after
  *         from STOP modes or in case of failure of the HSE used directly or indirectly
  *         as system clock (if the Clock Security System CSS is enabled).
  *
  * @note   The LSI can be selected as system clock source after
  *         in case of failure of the LSE used directly or indirectly
  *         as system clock (if the Clock Security System LSECSS is enabled).
  *
  * @note   A switch from one clock source to another occurs only if the target
  *         clock source is ready (clock stable after startup delay).
  *         If a clock source which is not yet ready is selected, the switch will
  *         occur when the clock source is ready.
  *
  * @note   You can use @ref HAL_RCC_GetClockConfig() function to know which clock is
  *         currently used as system clock source.
  *
  * @note   Depending on the device voltage range, the software has to set correctly
  *         HPRE[3:0] bits to ensure that HCLK not exceed the maximum allowed frequency
  *         (for more details refer to section above "Initialization/de-initialization functions")
  * @retval None
  */
HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef  *RCC_ClkInitStruct, uint32_t FLatency)
{
  uint32_t tickstart;

  /* Check Null pointer */
  if (RCC_ClkInitStruct == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_RCC_CLOCKTYPE(RCC_ClkInitStruct->ClockType));
  assert_param(IS_FLASH_LATENCY(FLatency));

  /* To correctly read data from FLASH memory, the number of wait states (LATENCY)
    must be correctly programmed according to the frequency of the FLASH clock
    (HCLK) and the supply voltage of the device. */

  /* Increasing the number of wait states because of higher CPU frequency */
  if (FLatency > __HAL_FLASH_GET_LATENCY())
  {
    /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
    __HAL_FLASH_SET_LATENCY(FLatency);

    /* Check that the new number of wait states is taken into account to access the Flash
    memory by polling the FLASH_ACR register */
    tickstart = HAL_GetTick();

    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLatency)
    {
      if ((HAL_GetTick() - tickstart) > CLOCKSWITCH_TIMEOUT_VALUE)
      {
        return HAL_TIMEOUT;
      }
    }
  }

  /*-------------------------- HCLK Configuration --------------------------*/
  if (((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_HCLK) == RCC_CLOCKTYPE_HCLK)
  {
    /* Set the highest APB divider in order to ensure that we do not go through
       a non-spec phase whatever we decrease or increase HCLK. */
    if (((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_PCLK1) == RCC_CLOCKTYPE_PCLK1)
    {
      MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE, RCC_HCLK_DIV16);
    }

    /* Set the new HCLK clock divider */
    assert_param(IS_RCC_HCLK(RCC_ClkInitStruct->AHBCLKDivider));
    MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_ClkInitStruct->AHBCLKDivider);
  }

  /*------------------------- SYSCLK Configuration ---------------------------*/
  if (((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_SYSCLK) == RCC_CLOCKTYPE_SYSCLK)
  {
    assert_param(IS_RCC_SYSCLKSOURCE(RCC_ClkInitStruct->SYSCLKSource));

    /* HSE is selected as System Clock Source */
    if (RCC_ClkInitStruct->SYSCLKSource == RCC_SYSCLKSOURCE_HSE)
    {
      /* Check the HSE Enable */
      if (READ_BIT(RCC->CR, RCC_CR_HSEEN) == 0U)
      {
        return HAL_ERROR;
      }
    }
    /* HSI is selected as System Clock Source */
    else if (RCC_ClkInitStruct->SYSCLKSource == RCC_SYSCLKSOURCE_HSISYS)
    {
      /* Check the HSI ready flag */
      if (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
      {
        return HAL_ERROR;
      }
    }
    /* LSE is selected as System Clock Source */
    else if (RCC_ClkInitStruct->SYSCLKSource == RCC_SYSCLKSOURCE_LSE)
    {
      /* Check the LSE ready flag */
      if (READ_BIT(RCC->BDCR, RCC_BDCR_LSERDY) == 0U)
      {
        return HAL_ERROR;
      }
    }
    /* LSI is selected as System Clock Source */
    else 
    {
      /* Check the LSI ready flag */
      if (READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) == 0U)
      {
        return HAL_ERROR;
      }
    }
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_ClkInitStruct->SYSCLKSource);

    /* Get Start Tick*/
    tickstart = HAL_GetTick();

    while (__HAL_RCC_GET_SYSCLK_SOURCE() != (RCC_ClkInitStruct->SYSCLKSource << RCC_CFGR_SWS_Pos))
    {
      if ((HAL_GetTick() - tickstart) > CLOCKSWITCH_TIMEOUT_VALUE)
      {
        return HAL_TIMEOUT;
      }
    }
  }

  /* Decreasing the number of wait states because of lower CPU frequency */
  if (FLatency < __HAL_FLASH_GET_LATENCY())
  {
    /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
    __HAL_FLASH_SET_LATENCY(FLatency);

    /* Check that the new number of wait states is taken into account to access the Flash
    memory by polling the FLASH_ACR register */
    tickstart = HAL_GetTick();

    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLatency)
    {
      if ((HAL_GetTick() - tickstart) > CLOCKSWITCH_TIMEOUT_VALUE)
      {
        return HAL_TIMEOUT;
      }
    }
  }

  /*-------------------------- PCLK1 Configuration ---------------------------*/
  if (((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_PCLK1) == RCC_CLOCKTYPE_PCLK1)
  {
    assert_param(IS_RCC_PCLK(RCC_ClkInitStruct->APB1CLKDivider));
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE, RCC_ClkInitStruct->APB1CLKDivider);
  }

  /* Update the SystemCoreClock global variable */
  SystemCoreClock = (HAL_RCC_GetSysClockFreq() >> ((AHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos]) & 0x1FU));

  /* Configure the source of time base considering new system clocks settings*/
  return HAL_InitTick(uwTickPrio);
}

/**
  * @}
  */

/** @defgroup RCC_Exported_Functions_Group2 Peripheral Control functions
 *  @brief   RCC clocks control functions
 *
@verbatim
 ===============================================================================
                      ##### Peripheral Control functions #####
 ===============================================================================
    [..]
    This subsection provides a set of functions allowing to:

    (+) Ouput clock to MCO pin.
    (+) Retrieve current clock frequencies.
    (+) Enable the Clock Security System.

@endverbatim
  * @{
  */

/**
  * @brief  Select the clock source to output on MCO pin.
  * @param  RCC_MCOx  specifies the output direction for the clock source.
  *            @arg @ref RCC_MCO1  Clock source to output on MCO1 pin(PA07).
  *            @arg @ref RCC_MCO2  Clock source to output on MCO2 pin(PB01).
  * @param  RCC_MCOSource  specifies the clock source to output.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_MCOSOURCE_NOCLOCK  MCO output disabled, no clock on MCO
  *            @arg @ref RCC_MCOSOURCE_SYSCLK  system  clock selected as MCO source
  *            @arg @ref RCC_MCOSOURCE_HSI  HSI clock selected as MCO source
  *            @arg @ref RCC_MCOSOURCE_HSE  HSE clock selected as MCO sourcee
  *            @arg @ref RCC_MCOSOURCE_LSI  LSI clock selected as MCO source
  *            @arg @ref RCC_MCOSOURCE_LSE  LSE clock selected as MCO source
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  * @param  RCC_MCODiv  specifies the MCO prescaler.
  *          This parameter can be one of the following values:
  *            @arg @ref RCC_MCODIV_1  no division applied to MCO clock
  *            @arg @ref RCC_MCODIV_2  division by 2 applied to MCO clock
  *            @arg @ref RCC_MCODIV_4  division by 4 applied to MCO clock
  *            @arg @ref RCC_MCODIV_8  division by 8 applied to MCO clock
  *            @arg @ref RCC_MCODIV_16  division by 16 applied to MCO clock
  *            @arg @ref RCC_MCODIV_32  division by 32 applied to MCO clock
  *            @arg @ref RCC_MCODIV_64  division by 64 applied to MCO clock
  *            @arg @ref RCC_MCODIV_128  division by 128 applied to MCO clock
  * @retval None
  */
void HAL_RCC_MCOConfig(uint32_t RCC_MCOx, uint32_t RCC_MCOSource, uint32_t RCC_MCODiv)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Check the parameters */
  assert_param(IS_RCC_MCO(RCC_MCOx));
  assert_param(IS_RCC_MCODIV(RCC_MCODiv));
  assert_param(IS_RCC_MCO1SOURCE(RCC_MCOSource));

  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  switch(RCC_MCOx)
  {
  case RCC_MCO2 :  /* Configure PB01 as the clock output */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Alternate = GPIO_AF4_MCO;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    break;
  case RCC_MCO1 :  /* Configure PA07 as the clock output */
  default :
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Alternate = GPIO_AF4_MCO;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    break;
  }

  /* Mask MCOSEL[] and MCOPRE[] bits then set MCO1 clock source and prescaler */
  MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCOSEL | RCC_CFGR_MCOPRE), (RCC_MCOSource | RCC_MCODiv));
}

/**
  * @brief  Return the SYSCLK frequency.
  *
  * @note   The system frequency computed by this function is not the real
  *         frequency in the chip. It is calculated based on the predefined
  *         constant and the selected clock source:
  * @note     If SYSCLK source is HSI, function returns values based on HSI_VALUE/HSIDIV(*)
  * @note     If SYSCLK source is HSE, function returns values based on HSE_VALUE(**)
  * @note     If SYSCLK source is LSI, function returns values based on LSI_VALUE(***)
  * @note     If SYSCLK source is LSE, function returns values based on LSE_VALUE(****)
  * @note     (*) HSI_VALUE is a constant defined in py32f002b_hal_conf.h file (default value
  *                8 MHz) but the real value may vary depending on the variations
  *               in voltage and temperature.
  * @note     (**) HSE_VALUE is a constant defined in py32f002b_hal_conf.h file (default value
  *                8 MHz), user has to ensure that HSE_VALUE is same as the real
  *                frequency of the crystal used. Otherwise, this function may
  *                have wrong result.
  * @note     (***) LSE_VALUE is a constant defined in py32f002b_hal_conf.h file (default value
  *               32768 Hz).
  * @note     (****) LSI_VALUE is a constant defined in py32f002b_hal_conf.h file (default value
  *               32768 Hz).
  *
  * @note   Depending on devices and packages, some clocks may not be available.
  *         Refer to device datasheet for clocks availability.
  *
  * @note   The result of this function could be not correct when using fractional
  *         value for HSE crystal.
  *
  * @note   This function can be used by the user application to compute the
  *         baudrate for the communication peripherals or configure other parameters.
  *
  * @note   Each time SYSCLK changes, this function must be called to update the
  *         right SYSCLK value. Otherwise, any configuration based on this function will be incorrect.
  *
  *
  * @retval SYSCLK frequency
  */
uint32_t HAL_RCC_GetSysClockFreq(void)
{
  uint32_t hsidiv;
  uint32_t sysclockfreq;
#if defined(RCC_HSI48M_SUPPORT)
  const uint32_t hsiValue[8] = {0U, 0U, 0U, 0U, 24000000U, 48000000U, 0U, 0U};
#else
  const uint32_t hsiValue[8] = {0U, 0U, 0U, 0U, 24000000U, 0U, 0U, 0U};
#endif
  uint32_t hsiIndex;
  
  if (__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_CFGR_SWS_HSISYS)
  {
    /* HSISYS can be derived for HSI */
    hsidiv = (1UL << ((READ_BIT(RCC->CR, RCC_CR_HSIDIV)) >> RCC_CR_HSIDIV_Pos));

    /* HSISYS used as system clock source */
    hsiIndex = (RCC->ICSCR&RCC_ICSCR_HSI_FS_Msk)>>RCC_ICSCR_HSI_FS_Pos;

    sysclockfreq = (hsiValue[hsiIndex] / hsidiv);
  }
  else if (__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_CFGR_SWS_HSE)
  {
    /* HSE used as system clock source */
    sysclockfreq = HSE_VALUE;
  }
  else if (__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_CFGR_SWS_LSE)
  {
    /* LSE used as system clock source */
    sysclockfreq = LSE_VALUE;
  }
  else if (__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_CFGR_SWS_LSI)
  {
    /* LSI used as system clock source */
    if ((READ_BIT(RCC->ICSCR, RCC_ICSCR_LSI_TRIM) >> RCC_ICSCR_LSI_TRIM_Pos) == RCC_LSICALIBRATION_32768Hz)
    {
      sysclockfreq = 32768U;
    }
    else if ((READ_BIT(RCC->ICSCR, RCC_ICSCR_LSI_TRIM) >> RCC_ICSCR_LSI_TRIM_Pos) == RCC_LSICALIBRATION_38400Hz)
    {
      sysclockfreq = 38400U;
    }
    else
    {
      sysclockfreq = 0U;
    }
  }
  else
  {
    sysclockfreq = 0U;
  }

  return sysclockfreq;
}

/**
  * @brief  Return the HCLK frequency.
  * @note   Each time HCLK changes, this function must be called to update the
  *         right HCLK value. Otherwise, any configuration based on this function will be incorrect.
  *
  * @note   The SystemCoreClock CMSIS variable is used to store System Clock Frequency.
  * @retval HCLK frequency in Hz
  */
uint32_t HAL_RCC_GetHCLKFreq(void)
{
  return SystemCoreClock;
}

/**
  * @brief  Return the PCLK1 frequency.
  * @note   Each time PCLK1 changes, this function must be called to update the
  *         right PCLK1 value. Otherwise, any configuration based on this function will be incorrect.
  * @retval PCLK1 frequency in Hz
  */
uint32_t HAL_RCC_GetPCLK1Freq(void)
{
  /* Get HCLK source and Compute PCLK1 frequency ---------------------------*/
  return (uint32_t)((HAL_RCC_GetHCLKFreq()) >> (APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE) >> RCC_CFGR_PPRE_Pos] & 0x1FU));
}

/**
  * @brief  Configure the RCC_OscInitStruct according to the internal
  *         RCC configuration registers.
  * @param  RCC_OscInitStruct  pointer to an RCC_OscInitTypeDef structure that
  *         will be configured.
  * @retval None
  */
void HAL_RCC_GetOscConfig(RCC_OscInitTypeDef  *RCC_OscInitStruct)
{
  /* Check the parameters */
  assert_param(RCC_OscInitStruct != (void *)NULL);

  /* Set all possible values for the Oscillator type parameter ---------------*/
  RCC_OscInitStruct->OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | \
                                      RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_LSI;

  /* Get the HSE configuration -----------------------------------------------*/
  if ((RCC->CR & RCC_CR_HSEEN) == RCC_CR_HSEEN)
  {
    RCC_OscInitStruct->HSEState = RCC_HSE_BYPASS_ENABLE;
  }
  else
  {
    RCC_OscInitStruct->HSEState = RCC_HSE_BYPASS_DISABLE;
  }
  
  /* Get the HSI configuration -----------------------------------------------*/
  if ((RCC->CR & RCC_CR_HSION) == RCC_CR_HSION)
  {
    RCC_OscInitStruct->HSIState = RCC_HSI_ON;
  }
  else
  {
    RCC_OscInitStruct->HSIState = RCC_HSI_OFF;
  }

  RCC_OscInitStruct->HSICalibrationValue = (RCC->ICSCR & (RCC_ICSCR_HSI_FS | RCC_ICSCR_HSI_TRIM));
  RCC_OscInitStruct->HSIDiv = (RCC->CR & RCC_CR_HSIDIV);

  /* Get the LSI configuration -----------------------------------------------*/
  if ((RCC->CSR & RCC_CSR_LSION) == RCC_CSR_LSION)
  {
    RCC_OscInitStruct->LSIState = RCC_LSI_ON;
  }
  else
  {
    RCC_OscInitStruct->LSIState = RCC_LSI_OFF;
  }
  
  RCC_OscInitStruct->LSICalibrationValue = ((RCC->ICSCR & RCC_ICSCR_LSI_TRIM) >> RCC_ICSCR_LSI_TRIM_Pos);

  /* Get the LSE configuration -----------------------------------------------*/
  if ((RCC->BDCR & RCC_BDCR_LSEBYP) == RCC_BDCR_LSEBYP)
  {
    RCC_OscInitStruct->LSEState = RCC_LSE_BYPASS;
  }
  else if ((RCC->BDCR & RCC_BDCR_LSEON) == RCC_BDCR_LSEON)
  {
    RCC_OscInitStruct->LSEState = RCC_LSE_ON;
  }
  else
  {
    RCC_OscInitStruct->LSEState = RCC_LSE_OFF;
  }
  
  RCC_OscInitStruct->LSEDriver = (RCC->ECSCR & RCC_ECSCR_LSE_DRIVER);
}

/**
  * @brief  Configure the RCC_ClkInitStruct according to the internal
  *         RCC configuration registers.
  * @param  RCC_ClkInitStruct Pointer to a @ref RCC_ClkInitTypeDef structure that
  *                           will be configured.
  * @param  pFLatency         Pointer on the Flash Latency.
  * @retval None
  */
void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef  *RCC_ClkInitStruct, uint32_t *pFLatency)
{
  /* Check the parameters */
  assert_param(RCC_ClkInitStruct != (void *)NULL);
  assert_param(pFLatency != (void *)NULL);

  /* Set all possible values for the Clock type parameter --------------------*/
  RCC_ClkInitStruct->ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1;

  /* Get the SYSCLK configuration --------------------------------------------*/
  RCC_ClkInitStruct->SYSCLKSource = (uint32_t)(RCC->CFGR & RCC_CFGR_SW);

  /* Get the HCLK configuration ----------------------------------------------*/
  RCC_ClkInitStruct->AHBCLKDivider = (uint32_t)(RCC->CFGR & RCC_CFGR_HPRE);

  /* Get the APB1 configuration ----------------------------------------------*/
  RCC_ClkInitStruct->APB1CLKDivider = (uint32_t)(RCC->CFGR & RCC_CFGR_PPRE);


  /* Get the Flash Wait State (Latency) configuration ------------------------*/
  *pFLatency = (uint32_t)(FLASH->ACR & FLASH_ACR_LATENCY);
}

/**
  * @brief  Enable the LSE Clock Security System.
  * @note   If a failure is detected on the LSE oscillator clock, this oscillator
  *         is automatically disabled and an interrupt is generated to inform the
  *         software about the failure (Clock Security System Interrupt, CSSI),
  *         allowing the MCU to perform rescue operations. The CSSI is linked to
  *         the Cortex-M0+ NMI (Non-Maskable Interrupt) exception vector.
  * @note   The LSE Clock Security System Detection bit (LSECSSD in BDCR) can only be
  *         cleared by a backup domain reset.
  * @retval None
  */
void HAL_RCC_EnableLSECSS(void)
{
  SET_BIT(RCC->BDCR, RCC_BDCR_LSECSSON) ;
}

/**
  * @brief  Disable the LSE Clock Security System.
  * @note   After LSE failure detection, the software must disable LSECSSON
  * @note   The Clock Security System can only be cleared by reset otherwise.
  * @retval None
  */
void HAL_RCC_DisableLSECSS(void)
{
  CLEAR_BIT(RCC->BDCR, RCC_BDCR_LSECSSON) ;
}

/**
  * @brief Handle the RCC Clock Security System interrupt request.
  * @note  This API should be called under the NMI_Handler().
  * @retval None
  */
void HAL_RCC_NMI_IRQHandler(void)
{
  uint32_t itflag = RCC->CIFR;

  /* Clear interrupt flags related to CSS */
  RCC->CICR = (itflag & RCC_CIFR_LSECSSF);

  /* Check RCC LSECSSF interrupt flag  */
  if ((itflag & RCC_CIFR_LSECSSF) != 0x00u)
  {
    /* RCC Clock Security System interrupt user callback */
    HAL_RCC_LSECSSCallback();
  }
}

/**
  * @brief Handle the RCC HSE Clock Security System interrupt callback.
  * @retval none
  */
__weak void HAL_RCC_CSSCallback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the @ref HAL_RCC_CSSCallback should be implemented in the user file
   */
}

#if defined(RCC_LSE_SUPPORT)
/**
  * @brief  RCC LSE Clock Security System interrupt callback.
  * @retval none
  */
__weak void HAL_RCC_LSECSSCallback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_RCC_LSECSSCallback should be implemented in the user file
   */
}
#endif
/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_RCC_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
