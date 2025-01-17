#ifndef _FEEDER_CONTROL_
#define _FEEDER_CONTROL_

#include "py32f0xx_hal.h"

extern void process_feeder();
extern void advance_feeder();
extern void read_feeder_data_from_flash();

typedef enum{
    FEEDER_IDLE=0U,
    FEEDER_ADVANCING,
    FEEDER_CHECK_ADVANCING_STARTING,
    FEEDER_HOLE_SEARCHING_FAST,
    FEEDER_HOLE_SEARCHING_SLOW,
    FEEDER_HOLE_FOUND
}Feeder_State;

//make sure its size is multiple of 4 bytes, for flash read/write
typedef struct {
    int8_t contentFlag; //if flash having content or not, after erased, the value is -1
    int16_t offsetXx10; //10 times offset X
    int16_t offsetYx10; //10 times offset Y 
    int16_t offsetZx10; //10 times offset Z
    uint8_t pitch;//pitch in mm
    int16_t h;//height in mm
    int16_t rt;//rotation in tape
    char partId[30]; //part id
    char name[20]; //name
}Feeder_Data_Def;

extern Feeder_Data_Def feeder_data;

#endif