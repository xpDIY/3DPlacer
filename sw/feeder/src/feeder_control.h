#ifndef _FEEDER_CONTROL_
#define _FEEDER_CONTROL_

#include "py32f0xx_hal.h"

extern void process_feeder();
extern void advance_feeder();
extern void read_feeder_data_from_flash();
extern void start_motor();
extern void stop_motor();
extern void led_on();
extern void led_off();
extern void led_ind();

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
    uint16_t st; //sub type of feeder, 0 - strip, 1 - auto feeder, 2 - loose feeder
    char partId[30]; //part id
    char name[20]; //name
    // Calibration parameters
    uint16_t vpr;   // Voltage Per Row (default 140)
    uint16_t vpc;   // Voltage Per Column (default 320)
    uint16_t vprth; // Row threshold offset (default 20)
    uint16_t vpcth; // Column threshold offset (default 50)
}Feeder_Data_Def;

extern Feeder_Data_Def feeder_data;

#endif