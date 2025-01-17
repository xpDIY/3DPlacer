#include "feeder_control.h"
#include "common.h"
#include "configuration.h"


#define FAST_HOLE_SEARCH_PERIOD 10000
#define ADVANCING_CHECK_PERIOD 80000

Feeder_State feeder_state = FEEDER_IDLE;
int state_counter=0;
int send_response=1;
uint8_t is_button_pressed=0;
float holeProb=0;
uint8_t prevHole=0;

//feeder data structure
Feeder_Data_Def feeder_data;

void (*finished_feeding)();

void start_motor(){
    HAL_GPIO_WritePin(GPIOB,PIN_MOTOR_A1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_RESET);    
}

void stop_motor(){
    HAL_GPIO_WritePin(GPIOB,PIN_MOTOR_A1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_SET);
    for(int i=0;i<5;++i);
    HAL_GPIO_WritePin(GPIOB,PIN_MOTOR_A1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_RESET);
}

void led_on(){
    //turn on LED
    HAL_GPIO_WritePin(GPIOA,PIN_LED2,GPIO_PIN_RESET);
}
void led_off(){
    //turn on LED
    HAL_GPIO_WritePin(GPIOA,PIN_LED2,GPIO_PIN_SET);
}

void advance_feeder(void (*processor)()){
    feeder_state = FEEDER_ADVANCING;
    finished_feeding = processor;
    send_response=1;//when done, send response to caller
}

void read_feeder_data_from_flash(){
    read_flash((uint32_t*)&feeder_data, 60);   
}

void process_feeder(){
  GPIO_PinState isHole = HAL_GPIO_ReadPin(GPIOA,PIN_PART_DET);
  GPIO_PinState isButton = HAL_GPIO_ReadPin(GPIOB,PIN_SW_B);
  if(!is_button_pressed && isButton){
    is_button_pressed = 1;
  }
  //holeProb = 0.8f*holeProb + 0.2f* holeVal?1:0;
  //uint8_t isHole= holeProb>0.8f;
  switch(feeder_state){
    case FEEDER_IDLE:
        if(is_button_pressed && (!isButton)){
            //button press and released, trigger advancing
            feeder_state = FEEDER_ADVANCING;
            send_response=0; //do not send response when job done
        }
        led_on();
        break;
    case FEEDER_ADVANCING:
        //start motor to run till current not in hole position
        led_off();
        if(isHole){
            start_motor();
            state_counter=ADVANCING_CHECK_PERIOD;
            feeder_state=FEEDER_CHECK_ADVANCING_STARTING;
        }else{
            start_motor();
            //past the hole position, transition to hole searching
            feeder_state=FEEDER_HOLE_SEARCHING_FAST;
            state_counter=FAST_HOLE_SEARCH_PERIOD;
        }
        break;
    case FEEDER_CHECK_ADVANCING_STARTING:
        state_counter--;
        if(!isHole)
        {
            start_motor();
            feeder_state=FEEDER_HOLE_SEARCHING_FAST;
            state_counter=FAST_HOLE_SEARCH_PERIOD;
        }else{
            //still in hole, if counter is zero, then means end strip or something wrong
            if(state_counter <= 0){
                stop_motor();
                feeder_state=FEEDER_IDLE;
            if(send_response){
                finished_feeding();                
            }
            }else{
                //counter is not zero, but still in hole, need to continue for non-hole
            }
        }

        break;
    case FEEDER_HOLE_SEARCHING_FAST:
        //if hole already there, just stop the motor
        if(isHole && state_counter < FAST_HOLE_SEARCH_PERIOD/2){

            stop_motor();
            feeder_state=FEEDER_IDLE;
            state_counter=0;
            if(send_response)
                finished_feeding();
            break;
        }

        //wait for counter to reach 0 while motor is running,
        //then stop motor and do slow motor searching      
        if(state_counter-- <= 0){
            stop_motor();
            feeder_state = FEEDER_HOLE_SEARCHING_SLOW;            
        }
        break;
    case FEEDER_HOLE_SEARCHING_SLOW:
        if(isHole){
            //hole found, go back to idle
            stop_motor();
            feeder_state=FEEDER_IDLE;
            state_counter=0;
            if(send_response){
                finished_feeding();
            }            
            break;
        }
        if(state_counter >1000){
            start_motor();
        }
        else if (state_counter>10)
        {
            stop_motor();
        }else if(state_counter<=0){
            state_counter = 2000;
        }
        state_counter--;
        break;
    default:
        break;
  }
  //clear states
  if(is_button_pressed && (!isButton)){
    is_button_pressed=0;
  }

}
