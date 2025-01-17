#include "string.h"
#include "uart_control.h"
#include "gparser.h"

#define MESSAGE_BUF_SIZE 70
// for uart control
UART_HandleTypeDef UartHandle;

//for 1wire control
UART_HandleTypeDef UartOwHandle;


uint8_t aRxBuffer[2] = {0x00};
uint8_t bRxBuffer[2] = {0x00};
char aMsg[100];
char msgBufA[MESSAGE_BUF_SIZE];
char *pMsg = msgBufA;

uint8_t aIdx=0;
volatile uint8_t bIdx=0;
volatile uint8_t sendIdx=0;
uint8_t receiveOw=0;
const uint8_t maxLength = 255;
int8_t res;

/**
  * @brief  USART GPIO Config,Mode Config,115200 8-N-1
  * @param  None
  * @retval None
  */
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

  /**USART GPIO Configuration
    PA0     ------> USART2_TX
    PA1     ------> USART2_RX
    */
  GPIO_InitStruct.Pin = USART_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = USART_TX_AF;
  HAL_GPIO_Init(USART_TX_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = USART_RX_PIN;
  GPIO_InitStruct.Alternate = USART_RX_AF;

  HAL_GPIO_Init(USART_RX_GPIO_PORT, &GPIO_InitStruct);

  /* ENABLE NVIC */
  HAL_NVIC_SetPriority(USART_IRQ,0,1);
  HAL_NVIC_EnableIRQ(USART_IRQ );
}

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

  USART1_TX_GPIO_CLK_ENABLE();

  /**USART GPIO Configuration
    PA0     ------> USART2_TX
    PA1     ------> USART2_RX
    */
  GPIO_InitStruct.Pin = USART1_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = USART1_TX_AF;
  HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);

  /* ENABLE NVIC */
  HAL_NVIC_SetPriority(USART1_IRQ,1,2);
  HAL_NVIC_EnableIRQ(USART1_IRQ );
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
  //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);  
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
}

void start_receiving_one_wire(UART_HandleTypeDef *UartHandle){
  //reset message idx
  bIdx=0;
  sendIdx=0;
  HAL_HalfDuplex_EnableReceiver(UartHandle);
}

void start_sending_one_wire(UART_HandleTypeDef *UartHandle){
  receiveOw=0;
  HAL_HalfDuplex_EnableTransmitter(UartHandle);
}

void UART_ErrorHandler(void)
{ 
  while (1);
}

/**
  * @brief  Sends an amount of data in blocking mode.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M=1),
  *         the sent data is handled as a set of u16. In this case, Size must indicate the number
  *         of u16 provided through pData.
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART module.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef Uart_Begin_Send_1Byte(UART_HandleTypeDef *huart, uint8_t *pData)
{
  uint8_t  *pdata8bits;
  uint16_t *pdata16bits;

  /* Check that a Tx process is not already ongoing */
  if (huart->gState == HAL_UART_STATE_READY)
  {
    if (pData == NULL)
    {
      return  HAL_ERROR;
    }
    /* In case of 9bits/No Parity transfer, pData buffer provided as input parameter
       should be aligned on a u16 frontier, as data to be filled into DR will be
       handled through a u16 cast. */
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
    {
      if ((((uint32_t)pData) & 1U) != 0U)
      {
        return  HAL_ERROR;
      }
    }


    //if send register not empty, return 
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) == RESET)
    {
      return HAL_BUSY;
    }
    /* Process Locked */
    __HAL_LOCK(huart);

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;

    huart->TxXferSize = 1;
    huart->TxXferCount = 1;

    /* In case of 9bits/No Parity transfer, pData needs to be handled as a uint16_t pointer */
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
        huart->Instance->DR = (uint16_t)(*pdata16bits & 0x01FFU);
        pdata16bits++;
      }
      else
      {
        huart->Instance->DR = (uint8_t)(*pdata8bits & 0xFFU);
        pdata8bits++;
      }

    /* Process Unlocked */
    __HAL_UNLOCK(huart);
    huart->gState = HAL_UART_STATE_READY;
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}
HAL_StatusTypeDef Uart_Send_Finished(UART_HandleTypeDef *huart){
      //if send register not empty, return 
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET)
    {
      return HAL_BUSY;
    }
   /* At end of Tx process, restore huart->gState to Ready */
    huart->gState = HAL_UART_STATE_READY;
    return HAL_OK;
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

uint8_t process_uart_data(uint8_t toReceive,uint8_t forward)
{
  uint8_t received=0;
  if(toReceive){
    if(UART_Begin_Receive(&UartOwHandle,(uint8_t *)bRxBuffer) == HAL_OK){
      pMsg[bIdx++] = *bRxBuffer;
      pMsg[bIdx] = 0;
      if(bIdx>=MESSAGE_BUF_SIZE-1)
        bIdx=0;
      received=1;
    }
    if(sendIdx!=bIdx && forward){
      if(Uart_Send_Finished(&UartHandle) == HAL_OK){      
        if(Uart_Begin_Send_1Byte(&UartHandle,(uint8_t *)pMsg+sendIdx) == HAL_OK){
          sendIdx++;
          if(sendIdx>=MESSAGE_BUF_SIZE-1) sendIdx=0;
        }
      }
    }else{
      sendIdx = bIdx;
    }
  }
  return received;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartCBHandle)
{
  if(UartCBHandle == &UartHandle){
    
    if(aIdx < maxLength){
      aMsg[aIdx++] = *aRxBuffer;
      aMsg[aIdx] = 0;
    }

    //a new line, that means receiving is over
    if(*aRxBuffer==13){
      res = parse_gcode(aMsg, UartCBHandle);
      aIdx=0;
    }

    if (HAL_UART_Receive_IT(UartCBHandle, (uint8_t *)aRxBuffer, 1) != HAL_OK)
    {
      UART_ErrorHandler();
    }
  }
}