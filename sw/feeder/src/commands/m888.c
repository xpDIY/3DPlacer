#include "string.h"
#include "stdlib.h"
#include "py32f0xx_hal.h"
#include "error_codes.h"
#include "gparser.h"
#include "uart_control.h"
#include "feeder_control.h"
#include "common.h"
#include "configuration.h"

const char M888[]="M888";
/// @brief Const for parameters
//parameter for Column
const char C[]="C";
//parameter for Row
const char R[]="R";
//parameter for total col
const char TC[]="TC";
//parameter for total row
const char TR[]="TR";
//parameter for feeder advancing
const char AD[]="AD";
//parameter for feeder name
const char N[]="N";
//parameter for pitch
const char PI[]="PI";
//parameter for rotation in tape
const char RT[]="RT";


// const for parameters
const uint16_t VPR=140;
const uint16_t VPC=320;
const uint16_t VPCTH=50;
const uint16_t VPRTH=20;
const uint8_t MAX_NAME_LEN=20;

/// command parameters
int col=0;
int row=0;
int tcol=0;
int trow=0;
int ad=0;
int n=0;
int pi=0;
int rt=0;
char *name_buf;

void process_param(char* key, char* val){
    //check for column command
    if(strcmp(key, C)==0){
        col=atoi(val);
        return;
    }
    if(strcmp(key,R)==0){
        row=atoi(val);
        return;
    }
    if(strcmp(key,TC)==0){
        tcol=atoi(val);
        return;
    }
    if(strcmp(key,TR)==0){
        trow=atoi(val);
        return;
    }
    if(strcmp(key,AD)==0){
        ad=atoi(val);
        return;
    }
    if(strcmp(key,N)==0){
        //flag name change has been requested
        name_buf = val;
        n=1;
        return;
    }
    if(strcmp(key,PI)==0){
        pi=atoi(val);
        return;
    }
    if(strcmp(key,RT)==0){
        rt=atoi(val);
        return;
    }     
}



void done(){
    HAL_HalfDuplex_EnableTransmitter(&UartOwHandle);
    HAL_UART_Transmit(&UartOwHandle, (uint8_t *)"done\r\n",6,10);
    HAL_HalfDuplex_EnableReceiver(&UartOwHandle);
}

void on_advance_finished(){
    done();
}

void reset_param(){
    row=0;
    col=0;
    tcol=0;
    trow=0;
    ad=0;
    n=0;
    pi=0;
    rt=0;
}

void silent(){
    HAL_HalfDuplex_EnableReceiver(&UartOwHandle);
}

int get_row(uint32_t vRow){
    if(trow>0)
        return trow - (vRow+VPRTH)/(4096/trow);
    return 29-(vRow+VPRTH)/VPR;
}

int get_col(uint32_t vCol){
    if(tcol>0)
        return (vCol+VPCTH)/(4096/(tcol+1));
    return (vCol+VPCTH)/VPC;
}

int8_t m888(char * noSpaceMsg, UART_HandleTypeDef *UartHandle)
{
    int dirty=0;
    if(strstr(noSpaceMsg,M888) == noSpaceMsg){
        //M888 detected, get parameters
        if(strlen(noSpaceMsg) == strlen(M888))
        {
            //when no parameter, don't return
            return TDP_OK;
        }
        //with parameters
        char * param =trimwhitespace(noSpaceMsg+strlen(M888));
        if(strlen(param)>0){
            reset_param();
            parse_parameters(param, process_param, silent);
            //only M800, means reading all the feeder info
            char *toret = "t:fed,id:%s,pi:%d,h:20,ox:0.2,oy:-13,rt:%d,r:%d,c:%d,n:%s;";
            uint32_t rpos,cpos;
            PollPos(&rpos,&cpos);
            if((row == get_row(rpos)) && (col ==get_col(cpos))){
                //advance feeder requested
                if(ad!=0){
                    advance_feeder(on_advance_finished);
                    return TDP_OK;
                }
                //set name requested, need to write flash
                if(n>0){
                    strncpy(&feeder_data.name[0],name_buf,MAX_NAME_LEN-1);
                    dirty=1;
                }
                if(pi>0){
                    feeder_data.pitch=pi;
                    dirty=1;
                }
                if(rt!=0){
                    feeder_data.rt = rt;
                    dirty=1;
                }

                if(!dirty){
                    HAL_HalfDuplex_EnableTransmitter(UartHandle);
                    sprintf(msgBuf,toret,uid_to_string(HAL_GetUIDw0(),HAL_GetUIDw1(),HAL_GetUIDw2()),
                        feeder_data.pitch!=0xff?feeder_data.pitch:40,feeder_data.rt!=-1?feeder_data.rt:0,get_row(rpos),get_col(cpos),
                        feeder_data.name[0]==0xff?uid_to_string(HAL_GetUIDw0(),HAL_GetUIDw1(),HAL_GetUIDw2()):feeder_data.name);
                    HAL_UART_Transmit(UartHandle, (uint8_t *)msgBuf, strlen(msgBuf),10);
                    HAL_HalfDuplex_EnableReceiver(UartHandle);
                }else{
                    write_flash((uint32_t*)&feeder_data, sizeof(feeder_data));
                    done();
                }
                return TDP_OK;
            }
            //either row is the same or col is the same
            //return ack
            if((row == get_row(rpos) && col == -2) || 
            (col == get_col(cpos) && row == -2)){
                HAL_HalfDuplex_EnableTransmitter(UartHandle);
                HAL_UART_Transmit(UartHandle, (uint8_t *)"ok", 2,10);
                HAL_HalfDuplex_EnableReceiver(UartHandle);
                return TDP_OK;
            }
            silent();            
            return TDP_OK;
        }
        return TDP_OK;
    }
    return TDP_ERR;
}