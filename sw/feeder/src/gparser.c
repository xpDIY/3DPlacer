#include "string.h"
#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "py32f0xx_hal.h"
#include "error_codes.h"
#include "uart_control.h"
#include "m888.h"


const char M115[]="M115";
const char M114[]="M114";

// message buffer, max 255 in length
char msgBuf[255];

void ok(UART_HandleTypeDef *UartHandle){
    HAL_UART_Transmit(UartHandle, (uint8_t *)"ok\r\n", 4,10);
}
void just_ok(){
    HAL_HalfDuplex_EnableTransmitter(&UartOwHandle);
    HAL_UART_Transmit(&UartOwHandle, (uint8_t *)"ok\r\n", 4,10);
    HAL_HalfDuplex_EnableReceiver(&UartOwHandle);
}

char *trimwhitespace(char *str)
{
  char *end;
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;
  if(*str == 0)  // All spaces?
    return str;
  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  // Write new null terminator character
  end[1] = '\0';
  return str;
}

void send_test_string(){
    char* toret = "abcdefghijkl";
    //only M800, means reading all the feeder info
    HAL_UART_Transmit(&UartOwHandle, (uint8_t *)toret, strlen(toret),10);
}


int8_t parse_gcode(char * toParse,UART_HandleTypeDef *UartHandle)
{
    char* noSpaceMsg= trimwhitespace(toParse);
    //check for m888 command
    m888(noSpaceMsg,UartHandle);

    if(strstr(noSpaceMsg,M115) == noSpaceMsg){
        //M115 detected, get parameters
        if(strlen(noSpaceMsg) == strlen(M115))
        {
            //only M115, means reading all the feeder info
            char* toret="\r\n3DPlacer FIRMWARE v0.1\r\nok\r\n";
            HAL_UART_Transmit(UartHandle,(uint8_t*)toret,strlen(toret),10);
            return TDP_OK;
        }
        //with parameters
        char * param =trimwhitespace(noSpaceMsg+strlen(M115));
        if(strlen(param) > 0){
            sprintf(msgBuf,"\r\n%s\r\nok\r\n",param);
            msgBuf[strlen(param)+8]=0;
            HAL_UART_Transmit(UartHandle,(uint8_t*)msgBuf,strlen(msgBuf),10);

            return TDP_OK;
        }
        ok(UartHandle);
        return TDP_OK;
    }

    if(strstr(noSpaceMsg,M114) == noSpaceMsg){
        //M114 detected, get parameters
        if(strlen(noSpaceMsg) == strlen(M114))
        {
            //only M114, means reading all the feeder info
            char* toret= "\r\nX:0.0 Y:0.0 Z:0.0\r\nok\r\n";
            HAL_UART_Transmit(UartHandle,(uint8_t*)toret,strlen(toret),10);
            return TDP_OK;
        }
        //with parameters
        char * param =trimwhitespace(noSpaceMsg+strlen(M114));
        if(strlen(param) > 0){
            sprintf(msgBuf,"\r\n%s\r\nok\r\n",param);
            msgBuf[strlen(param)+8]=0;
            HAL_UART_Transmit(UartHandle,(uint8_t*)msgBuf,strlen(msgBuf),10);
            return TDP_OK;
        }
        ok(UartHandle);
        return TDP_OK;
    }

    sprintf(msgBuf,"\r\n%s\r\nok\r\n",noSpaceMsg);
    msgBuf[strlen(noSpaceMsg)+8]=0;
 
    return TDP_OK;
}