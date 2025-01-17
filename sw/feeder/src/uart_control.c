#include "uart_control.h"
#include "gparser.h"
#include "string.h"
#include "configuration.h"

// for uart control
UART_HandleTypeDef UartHandle;

//for 1wire control
UART_HandleTypeDef UartOwHandle;

ADC_HandleTypeDef             AdcHandle;
ADC_ChannelConfTypeDef        sConfig;

volatile uint8_t aRxBuffer[2];
char aMsg[5];
char bMsg[100];
volatile char rxBuf[1];
volatile uint8_t idx=0, idxA=0;
const uint8_t maxLength = 100;
int8_t res;

/**
  * @brief  USART GPIO Config,Mode Config,115200 8-N-1
  * @param  None
  * @retval None
  */
 /*
void USART2_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  USART2_CLK_ENABLE();

  UartHandle.Instance          = CONTROL_USART;

  UartHandle.Init.BaudRate     = CONTROL_USART_BAUDRATE;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;

  HAL_UART_Init(&UartHandle);

  USART_RX_GPIO_CLK_ENABLE();
  USART_TX_GPIO_CLK_ENABLE();

  //USART GPIO Configuration
  //  PA0     ------> USART2_TX
  //  PA1     ------> USART2_RX  
  GPIO_InitStruct.Pin = USART_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = USART_TX_AF;
  HAL_GPIO_Init(USART_TX_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = USART_RX_PIN;
  GPIO_InitStruct.Alternate = USART_RX_AF;

  HAL_GPIO_Init(USART_RX_GPIO_PORT, &GPIO_InitStruct);

  // ENABLE NVIC
  HAL_NVIC_SetPriority(USART_IRQ,0,1);
  HAL_NVIC_EnableIRQ(USART_IRQ );
}
*/

/**
  * @brief  USART GPIO Config,Mode Config,115200 8-N-1 for 
  * @param  None
  * @retval None
  */
void USART1_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  USART1_CLK_ENABLE();

  UartOwHandle.Instance          = OW_USART;

  UartOwHandle.Init.BaudRate     = CONTROL_USART_BAUDRATE;
  UartOwHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartOwHandle.Init.StopBits     = UART_STOPBITS_1;
  UartOwHandle.Init.Parity       = UART_PARITY_NONE;
  UartOwHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartOwHandle.Init.Mode         = UART_MODE_TX_RX;
  

  HAL_HalfDuplex_Init(&UartOwHandle);
  UartOwHandle.RxState = HAL_UART_STATE_READY;

  USART1_TX_GPIO_CLK_ENABLE();

  /**USART GPIO Configuration
    PA0     ------> USART2_TX
    PA1     ------> USART2_RX
    */
  GPIO_InitStruct.Pin = PIN_OW;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = USART1_TX_AF;
  HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);

  /* ENABLE NVIC */
  HAL_NVIC_SetPriority(USART1_IRQ,0,1);
  HAL_NVIC_EnableIRQ(USART1_IRQ );
  idx=0;
}


HAL_StatusTypeDef UART_Begin_Receive(UART_HandleTypeDef *huart, uint8_t *pData)
{
  uint8_t  *pdata8bits;
  uint16_t *pdata16bits;

  /* Check that a Rx process is not already ongoing */
  if (huart->RxState == HAL_UART_STATE_READY)
  {
    if (pData == NULL)
    {
      return  HAL_ERROR;
    }
    /* In case of 9bits/No Parity transfer, pData buffer provided as input parameter
       should be aligned on a u16 frontier, as data to be received from RDR will be
       handled through a u16 cast. */
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
      if ((((uint32_t)pData) & 1U) != 0U)
      {
        return  HAL_ERROR;
      }
    }
    //when rxne is not set, that means nothing to receive, just return
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) == RESET){
      return HAL_ERROR;
    }
    //something to recceive, now can start the process

    /* Process Locked */
    __HAL_LOCK(huart);

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    huart->RxXferSize = 1;
    huart->RxXferCount = 1;

    /* In case of 9bits/No Parity transfer, pRxData needs to be handled as a uint16_t pointer */
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
      pdata8bits  = NULL;
      pdata16bits = (uint16_t *) pData;
    }
    else
    {
      pdata8bits  = pData;
      pdata16bits = NULL;
    }
    if (pdata8bits == NULL)
    {
      *pdata16bits = (uint16_t)(huart->Instance->DR & 0x1FF);
      pdata16bits++;
    }
    else
    {
      *pdata8bits = (uint8_t)(huart->Instance->DR & (uint8_t)0xFF);
      pdata8bits++;
    }
    /* At end of Rx process, restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(huart);
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

void process_ow_data()
{
  if(UART_Begin_Receive(&UartOwHandle,(uint8_t *)aRxBuffer) == HAL_OK){
    aMsg[idxA] = *aRxBuffer;
    bMsg[idx] = aMsg[idxA];
    bMsg[++idx] = 0;
    if(++idxA>4) idxA=0;

    if(bMsg[idx-1] == 13){
      //HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
      parse_gcode(bMsg,&UartOwHandle);
      idx=0;
    }
  }
}
/**
  * @brief This function handles USART2 Interrupt .
  */
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartHandle);
}

/**
  * @brief This function handles USART2 Interrupt .
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartOwHandle);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
}

void UART_ErrorHandler(void)
{ 
  while (1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartCBHandle)
{
   

}

void APP_AdcConfig(void)
{
  __HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET();                                                  /* Reset ADC */
  __HAL_RCC_ADC_CLK_ENABLE();                                                     /* Enable ADC clock */

  AdcHandle.Instance = ADC1;
  if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK)                         
  {
    UART_ErrorHandler();
  }
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV1;                /* ADC clock no division */
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;                      /* 12bit */
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;                     /* Right alignment */
  AdcHandle.Init.ScanConvMode          = ADC_SCAN_DIRECTION_FORWARD;             /* Backward */
  AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;                     /* End flag */
  AdcHandle.Init.LowPowerAutoWait      = ENABLE;
  AdcHandle.Init.ContinuousConvMode    = ENABLE;
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;
  AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;            /* External trigger: TIM1_TRGO */
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_SOFTWARE_START;  /* Triggered by both edges */
  //AdcHandle.Init.DMAContinuousRequests = DISABLE;                                 /* No DMA */
  AdcHandle.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;
  AdcHandle.Init.SamplingTimeCommon    = ADC_SAMPLETIME_28CYCLES_5;
  if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
  {
    UART_ErrorHandler();
  }
  //config row channel
  sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
  sConfig.Channel      = ADC_CHANNEL_3;
  if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)                      
  {
    UART_ErrorHandler();
  }
  //config column channel
  sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
  sConfig.Channel      = ADC_CHANNEL_4;
  if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)                      
  {
    UART_ErrorHandler();
  }

  /* Start ADC with Interrupt */
  if (HAL_ADC_Start(&AdcHandle) != HAL_OK)                                     
  {
    UART_ErrorHandler();
  }
}

void PollPos(uint32_t * rpos, uint32_t *cpos){
  if(HAL_ADC_PollForConversion(&AdcHandle,1) == HAL_OK){
    *rpos = HAL_ADC_GetValue(&AdcHandle);
  }else{
    *rpos=0;
  }
  if(HAL_ADC_PollForConversion(&AdcHandle,1) == HAL_OK){
    *cpos = HAL_ADC_GetValue(&AdcHandle);
  }else{
    *cpos=0;
  }
  if(HAL_ADC_PollForConversion(&AdcHandle,1) == HAL_OK){
    *rpos = HAL_ADC_GetValue(&AdcHandle);
  }else{
    *rpos=0;
  }
  if(HAL_ADC_PollForConversion(&AdcHandle,1) == HAL_OK){
    *cpos = HAL_ADC_GetValue(&AdcHandle);
  }else{
    *cpos=0;
  }  

}
