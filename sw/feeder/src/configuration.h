#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_


#define PIN_LED1 GPIO_PIN_0 
#define PIN_LED2 GPIO_PIN_0
#define PIN_MOTOR_A1 GPIO_PIN_3
#define PIN_MOTOR_A2 GPIO_PIN_4
#define PIN_PART_DET GPIO_PIN_12
#define PIN_PART_LED GPIO_PIN_1
#define PIN_OW GPIO_PIN_2
#define PIN_MOTOR_EN GPIO_PIN_1 //no motor en support on this board
#define PIN_RPOS GPIO_PIN_5
#define PIN_CPOS GPIO_PIN_6
#define PIN_RPOS1 GPIO_PIN_7
#define PIN_SW_B GPIO_PIN_8
#define PIN_SWC_IN GPIO_PIN_14   // SWCLK pin reused as active-low input after boot grace period

// Photo interrupter LED (PB1) drive tuning via software PWM
// Effective LED drive ~= PART_LED_SUPPLY_MV * duty
// Lower voltage reduces light bleed-through between tape holes
#define PART_LED_SUPPLY_MV 3300
#define PART_LED_TARGET_MV 800

// Feeder type selection: define one of the following
#define FEEDER_TYPE_LOOSE_PART  0
#define FEEDER_TYPE_AUTO        1

// Set the active feeder type here, or override at build time:
//   make FEEDER_TYPE=FEEDER_TYPE_LOOSE_PART
//   make FEEDER_TYPE=FEEDER_TYPE_AUTO
#ifndef FEEDER_TYPE
#define FEEDER_TYPE  FEEDER_TYPE_AUTO
#endif

#define IS_AUTO_FEEDER   (FEEDER_TYPE == FEEDER_TYPE_AUTO)
#define IS_LOOSE_FEEDER  (FEEDER_TYPE == FEEDER_TYPE_LOOSE_PART)

// LED mapping on the feeder board (PY32F002A, TQFN-16):
//   PIN_LED1   (PA0) -> net LED1   -> on-board LED3 via R6, exposed on PAD1/PAD17
//   PIN_LED2   (PB0) -> net LED2   -> on-board LED4 via R8
//   PIN_PART_LED(PB1)-> net PD_LED -> photo-interrupter emitter U10, exposed on PAD3/PAD25
//   PIN_PART_DET(PA12)-> net PD1   -> photo-interrupter output U10

#endif
