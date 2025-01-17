#include "string.h"
#include "ctype.h"
#include "stdio.h"
#include "py32f0xx_hal.h"
#include "error_codes.h"
#include "uart_control.h"
#include "stdlib.h"
#include "common.h"
#include "gparser.h"

#define CC1_PIN GPIO_PIN_1
#define CC2_PIN GPIO_PIN_0

const char M888[]="M888";
const char M887[]="M887";
const char M115[]="M115";
const char M114[]="M114";
const char CC1[]="CC1";
const char CC2[]="CC2";
const char TC[]="TC"; //total column
const char TR[]="TR"; //total row
const char R[]="R";//row
const char C[]="C";//col

const int SCAN_TH=240;
const int DEFAULT_TOTAL_ROW=30;
const int DEFAULT_TOTAL_COL=12;
const int ANS_TH=1000000;


char msgBuf[255];
//for storing control data
Control_Data_Def theData;

int cc1=-1;
int cc2=-1;
int tr=DEFAULT_TOTAL_ROW;
int tc=DEFAULT_TOTAL_COL;
int8_t r=-1;
int8_t c=-1;

void ok(UART_HandleTypeDef *UartHandle){
    HAL_UART_Transmit(UartHandle, (uint8_t *)"\r\nok\r\n", 6,10);
}
void ok_3dp(UART_HandleTypeDef *UartHandle){
    HAL_UART_Transmit(UartHandle, (uint8_t *)"\r\n3DP ok\r\n", 10,10);
}
void crlf(UART_HandleTypeDef *UartHandle){
    HAL_UART_Transmit(UartHandle, (uint8_t *)"\r\n", 2,10);
}

void parser_init(){
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = CC1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = CC2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void set_control_channel(uint16_t pin, int val){
    if(val == 0){
        HAL_GPIO_WritePin(GPIOA,pin, GPIO_PIN_RESET);
    }
    if(val == 1){
        HAL_GPIO_WritePin(GPIOA,pin, GPIO_PIN_SET);
    }
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

void reset_param(){
    cc1=-1;
    cc2=-1;
    tr=DEFAULT_TOTAL_ROW;
    tc=DEFAULT_TOTAL_COL;
    r=-1;
    c=-1;    
}

void silent(){
}

void process_param(char* key, char* val){
    //cc1 - control channel 1
    if(strcmp(key,CC1)==0){
        cc1=atoi(val);
        return;
    }
    //cc2 - control channel 2
    if(strcmp(key,CC2)==0){
        cc2=atoi(val);
        return;
    }   
    if(strcmp(key,TC)==0){
        tc=atoi(val);
        return;
    }        
    if(strcmp(key,TR)==0){
        tr=atoi(val);
        return;
    }        
    if(strcmp(key,C)==0){
        c=atoi(val);
        return;
    }        
    if(strcmp(key,R)==0){
        r=atoi(val);
        return;
    }        
}

int wait_for_receiving_ow(int wait_tick, int forward){
    uint8_t receiving=0;
    int k=0;
    int received=0;
    while(k++<wait_tick || receiving){
        receiving = process_uart_data(1, forward);
        if(receiving){
            k=wait_tick - SCAN_TH/2;
            received++;
        }
    }
    return received;
}

void send_control_feeder_info(UART_HandleTypeDef *UartHandle){
    read_flash((uint32_t*)&theData,sizeof(theData));
    char *toret = "3DP t:fed,id:%s,w:16,l:64,h:43,ox:%d,oy:%d,oz:%d,r:0,c:0,n:control;";
    sprintf(msgBuf,toret,uid_to_string(HAL_GetUIDw0(),HAL_GetUIDw1(),HAL_GetUIDw2()),theData.offsetXx10,theData.offsetYx10,theData.offsetZx10);
    HAL_UART_Transmit(UartHandle, (uint8_t *)msgBuf, strlen(msgBuf),10);
}

void query_all_component(int tr, int tc,char* param, UART_HandleTypeDef *UartHandle){
    send_control_feeder_info(UartHandle);
    for(int i=0;i<tr;++i){ 
        sprintf(msgBuf,"M888 R:%d,C:-2,TR:%d,TC:%d;\r\n",i,tr,tc);
        start_sending_one_wire(&UartOwHandle);
        HAL_UART_Transmit(&UartOwHandle, (uint8_t *)msgBuf,strlen(msgBuf),10);
        start_receiving_one_wire(&UartOwHandle);
        //just check if answer exists
        int received=wait_for_receiving_ow(SCAN_TH,0);
        if(received){
            for(int j=0;j<tc;++j){
                //tc+1 due to hardware included 1 extra column in control board
                sprintf(msgBuf,"M888 R:%d,C:%d,TR:%d,TC:%d%s%s;\r\n",i,j,tr,tc,param==NULL?"":",",param==NULL?"":param);
                start_sending_one_wire(&UartOwHandle);
                HAL_UART_Transmit(&UartOwHandle, (uint8_t *)msgBuf,strlen(msgBuf),10);
                start_receiving_one_wire(&UartOwHandle);
                wait_for_receiving_ow(SCAN_TH,1); //now need to forward answer
            }
        }
    }
    ok(UartHandle);
    start_receiving_one_wire(&UartOwHandle);
}

int8_t parse_gcode(char * toParse,UART_HandleTypeDef *UartHandle)
{
    char* noSpaceMsg= trimwhitespace(toParse);
    if(strstr(noSpaceMsg,M888) == noSpaceMsg){
        //M888 detected, get parameters
        if(strlen(noSpaceMsg) == strlen(M888))
        {
            //first send base info for feeder.
            HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
            query_all_component(DEFAULT_TOTAL_ROW,DEFAULT_TOTAL_COL,NULL,UartHandle);
            return TDP_OK;
        }
        //with parameters
        char * param =trimwhitespace(noSpaceMsg+strlen(M888));
        if(strlen(param) > 0){
            reset_param();
            sprintf(msgBuf,"%s",param); //copy the command
            parse_parameters(msgBuf, process_param, silent);
            //controlling control channels
            if(cc1 >=0 || cc2 >=0){
                set_control_channel(CC1_PIN,cc1);
                set_control_channel(CC2_PIN,cc2);
                ok(UartHandle);
                return TDP_OK;
            }
            //specified row and column, query for specific component
            if(r!=-1 && c!=-1)
            {
                start_sending_one_wire(&UartOwHandle);
                HAL_UART_Transmit(&UartOwHandle, (uint8_t *)noSpaceMsg,strlen(noSpaceMsg),10);
                crlf(&UartOwHandle);
                start_receiving_one_wire(&UartOwHandle);
                wait_for_receiving_ow(ANS_TH,1);
                ok_3dp(UartHandle);
                ok(UartHandle);
                start_receiving_one_wire(&UartOwHandle);
                return TDP_OK;
            }
            //not specified both row and column, query for all
            //in here, user might specify some other parameter
            if(r==-1 || c==-1){
                query_all_component(tr,tc,param,UartHandle);
                return TDP_OK;
            }
            return TDP_OK;
        }
        return TDP_OK;
    }

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