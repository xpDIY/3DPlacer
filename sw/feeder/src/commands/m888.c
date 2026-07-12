#include "string.h"
#include "stdlib.h"
#include "py32f0xx_hal.h"
#include "error_codes.h"
#include "gparser.h"
#include "uart_control.h"
#include "feeder_control.h"
#include "common.h"
#include "configuration.h"

/**
 * Format a value stored in 0.1mm units as a millimetre string with one decimal
 * (e.g. 2 -> "0.2", -130 -> "-13.0"). Avoids %f which is unavailable with
 * newlib-nano.
 */
static void format_mm_tenth(int16_t v10, char* out) {
    uint16_t abs_v = (v10 < 0) ? (uint16_t)(-v10) : (uint16_t)v10;
    if (v10 < 0) {
        sprintf(out, "-%u.%u", abs_v / 10, abs_v % 10);
    }
    else {
        sprintf(out, "%u.%u", abs_v / 10, abs_v % 10);
    }
}

const char M888[]="M888";
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
//parameter for height
const char H[]="H";
//parameter for rotation in tape
const char RT[]="RT";
//parameter for sub type of feeder
const char ST[]="ST";
//parameter for raw ADC query
const char RAW[]="RAW";
//parameter for per-feeder contact offset X (slot center to pick), in 0.1mm
const char OX[]="OX";
//parameter for per-feeder contact offset Y (slot center to pick), in 0.1mm
const char OY[]="OY";
//calibration parameters
const char VPR_P[]="VPR";
const char VPC_P[]="VPC";
const char VPRTH_P[]="VPRTH";
const char VPCTH_P[]="VPCTH";

// Default values for parameters
const uint16_t VPR_DEFAULT=137;
const uint16_t VPC_DEFAULT=320;
const uint16_t VPCTH_DEFAULT=70;
const uint16_t VPRTH_DEFAULT=40;
const uint8_t MAX_NAME_LEN=20;

/// command parameters
int col=0;
int row=0;
int tcol=0;
int trow=0;
int ad=0;
int n=0;
int pi=0;
int h=0;
int h_set=0; //flag to indicate H was explicitly set
int rt=0;
int st=0; //sub type of feeder, 0 - strip, 1 - auto feeder, 2 - loose feeder
int st_set=0;
int raw_query=0; //flag to query raw ADC values
int vpr_val=0;
int vpc_val=0;
int vprth_val=0;
int vpcth_val=0;
int ox_val=0;       // contact offset X in 0.1mm
int oy_val=0;       // contact offset Y in 0.1mm
int ox_set=0;       // flag: OX was explicitly set
int oy_set=0;       // flag: OY was explicitly set
char name_buf[32];   // persistent buffer for name (avoid stack pointer dangle)

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
        strncpy(name_buf, val, sizeof(name_buf)-1);
        name_buf[sizeof(name_buf)-1] = 0;
        n=1;
        return;
    }
    if(strcmp(key,PI)==0){
        pi=atoi(val);
        return;
    }
    if(strcmp(key,H)==0){
        h=atoi(val);
        h_set=1;
        return;
    }
    if(strcmp(key,RT)==0){
        rt=atoi(val);
        return;
    }     
    if(strcmp(key,ST)==0){
        st=atoi(val);
        st_set=1;
        return;
    }
    if(strcmp(key,OX)==0){
        ox_val=atoi(val);
        ox_set=1;
        return;
    }
    if(strcmp(key,OY)==0){
        oy_val=atoi(val);
        oy_set=1;
        return;
    }
    if(strcmp(key,RAW)==0){
        raw_query=atoi(val);  // Accept RAW:1 or RAW:0
        return;
    }
    if(strcmp(key,VPR_P)==0){
        vpr_val=atoi(val);
        return;
    }
    if(strcmp(key,VPC_P)==0){
        vpc_val=atoi(val);
        return;
    }
    if(strcmp(key,VPRTH_P)==0){
        vprth_val=atoi(val);
        return;
    }
    if(strcmp(key,VPCTH_P)==0){
        vpcth_val=atoi(val);
        return;
    }
}

void done(){
    HAL_HalfDuplex_EnableTransmitter(&UartOwHandle);
    HAL_UART_Transmit(&UartOwHandle, (uint8_t *)"ok\r\n",4,10);
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
    h=0;
    h_set=0;
    rt=0;
    st=0; //sub type of feeder, 0 - strip, 1 - auto feeder, 2 - loose feeder
    st_set=0;
    raw_query=0;
    vpr_val=0;
    vpc_val=0;
    vprth_val=0;
    vpcth_val=0;
    ox_val=0;
    oy_val=0;
    ox_set=0;
    oy_set=0;
}

void silent(){
    HAL_HalfDuplex_EnableReceiver(&UartOwHandle);
}

int get_row(uint32_t vRow){
    // Use calibrated values if available, otherwise use defaults
    uint16_t vpr = (feeder_data.vpr != 0 && feeder_data.vpr != 0xFFFF) ? feeder_data.vpr : VPR_DEFAULT;
    uint16_t vprth = (feeder_data.vprth != 0 && feeder_data.vprth != 0xFFFF) ? feeder_data.vprth : VPRTH_DEFAULT;
    
    if(trow>0)
        return trow - (vRow+VPRTH_DEFAULT)/(4096/(trow));
    return 29-(vRow+vprth)/vpr;
}

int get_col(uint32_t vCol){
    // Use calibrated values if available, otherwise use defaults
    uint16_t vpc = (feeder_data.vpc != 0 && feeder_data.vpc != 0xFFFF) ? feeder_data.vpc : VPC_DEFAULT;
    uint16_t vpcth = (feeder_data.vpcth != 0 && feeder_data.vpcth != 0xFFFF) ? feeder_data.vpcth : VPCTH_DEFAULT;
    
    if(tcol>0)
        return tcol - (vCol+vpcth)/(4096/(tcol));
    return (vCol+vpcth)/vpc;
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
            int tokCount = 0;  // DBG: count parsed tokens
            // Try colon format first; if no colon found, use space format.
            if (strchr(param, ':') != NULL) {
                parse_parameters(param, process_param, silent);
            } else {
                // Space format: KEYVALUE KEYVALUE;...
                char *p = param;
                char keyBuf[8], valBuf[64];
                while (*p) {
                    while (*p == ' ') p++;
                    if (*p == '\0' || *p == ';') break;
                    char *end = strchr(p, ' ');
                    char *semi = strchr(p, ';');
                    if (!end || (semi && semi < end)) end = semi;
                    if (!end) end = p + strlen(p);
                    int tokLen = end - p;
                    if (tokLen <= 0) { p = end; if (*p == ';') break; p++; continue; }
                    // Key: 2-letter if matches known, else 1-letter
                    int kLen = 1;
                    if (tokLen >= 2) {
                        char c0 = p[0], c1 = p[1];
                        if ((c0=='T'&&(c1=='C'||c1=='R'))||(c0=='P'&&c1=='I')
                         ||(c0=='S'&&c1=='T')||(c0=='R'&&c1=='T')
                         ||(c0=='O'&&(c1=='X'||c1=='Y'))||(c0=='A'&&c1=='D'))
                            kLen = 2;
                    }
                    int vLen = tokLen - kLen;
                    memcpy(keyBuf, p, kLen); keyBuf[kLen] = 0;
                    memcpy(valBuf, p+kLen, vLen); valBuf[vLen] = 0;
                    // DBG: count token, capture first char of key
                    tokCount++;
                    process_param(keyBuf, valBuf);
                    p = end;
                    if (*p == ';') break;
                    if (*p == ' ') p++;
                }
            }
            //only M800, means reading all the feeder info
            // ox/oy are returned as mm (one decimal) from the stored 0.1mm values; fall back
            // to the legacy defaults (0.2 / -13.0) when the feeder has never been calibrated.
            char *toret = "3DP t:fed,id:%s,pi:%d,h:%d,ox:%s,oy:%s,rt:%d,r:%d,c:%d,st:%d,n:%s;";
            int16_t ox10 = (feeder_data.offsetXx10 != -1) ? feeder_data.offsetXx10 : 2;   // 0.2mm
            int16_t oy10 = (feeder_data.offsetYx10 != -1) ? feeder_data.offsetYx10 : -130; // -13.0mm
            char ox_str[12];
            char oy_str[12];
            format_mm_tenth(ox10, ox_str);
            format_mm_tenth(oy10, oy_str);
            uint32_t rpos,cpos;
            PollPos(&rpos,&cpos);
            rpos = get_rpos();
            cpos = get_cpos();
            // Handle raw ADC query first (for calibration) - don't need position match
            if(raw_query){
                HAL_HalfDuplex_EnableTransmitter(UartHandle);
                sprintf(msgBuf,"3DP t:fed,id:%s,radc:%lu,cadc:%lu,r:%d,c:%d;",
                    uid_to_string(HAL_GetUIDw0(),HAL_GetUIDw1(),HAL_GetUIDw2()),
                    rpos, cpos, get_row(rpos), get_col(cpos));
                HAL_UART_Transmit(UartHandle, (uint8_t *)msgBuf, strlen(msgBuf),10);
                HAL_HalfDuplex_EnableReceiver(UartHandle);
                led_ind();
                return TDP_OK;
            }
            
            if((row == get_row(rpos)) && (col == get_col(cpos))) {
                if(ad!=0){
                    if(ad==1){
                        led_on();
                    }else if(ad==2){
                        led_off();
                    }
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
                if(h_set){
                    feeder_data.h=h;
                    dirty=1;
                }
                if(rt!=0){
                    feeder_data.rt = rt;
                    dirty=1;
                }
                if(st_set){
                    feeder_data.st = st;
                    dirty=1;
                }
                // Per-feeder contact offset (slot center to pick), in 0.1mm units.
                if(ox_set){
                    feeder_data.offsetXx10 = ox_val;
                    dirty=1;
                }
                if(oy_set){
                    feeder_data.offsetYx10 = oy_val;
                    dirty=1;
                }
                // Handle calibration parameter updates
                if(vpr_val>0){
                    feeder_data.vpr = vpr_val;
                    dirty=1;
                }
                if(vpc_val>0){
                    feeder_data.vpc = vpc_val;
                    dirty=1;
                }
                if(vprth_val>0){
                    feeder_data.vprth = vprth_val;
                    dirty=1;
                }
                if(vpcth_val>0){
                    feeder_data.vpcth = vpcth_val;
                    dirty=1;
                }

                if(!dirty){
                    HAL_HalfDuplex_EnableTransmitter(UartHandle);
                    sprintf(msgBuf,toret,uid_to_string(HAL_GetUIDw0(),HAL_GetUIDw1(),HAL_GetUIDw2()),
                        feeder_data.pitch!=0xff?feeder_data.pitch:40,feeder_data.h!=-9999?feeder_data.h:10,
                        ox_str, oy_str,
                        feeder_data.rt!=-1?feeder_data.rt:0,get_row(rpos),get_col(cpos),feeder_data.st!=0xffff?feeder_data.st:0,
                        feeder_data.name[0]==0xff?uid_to_string(HAL_GetUIDw0(),HAL_GetUIDw1(),HAL_GetUIDw2()):feeder_data.name);
                    HAL_UART_Transmit(UartHandle, (uint8_t *)msgBuf, strlen(msgBuf),10);
                    HAL_HalfDuplex_EnableReceiver(UartHandle);
                }else{
                    write_flash((uint32_t*)&feeder_data, sizeof(feeder_data));
                    done();
                }
                led_ind();
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