#include "feeder_control.h"
#include "py32f0xx_hal.h"
#include "common.h"
#include "py32f0xx_bsp_clock.h"
#include "configuration.h"


#define SW_RELEASE_DEBOUNCE_MS 80U
#define PULL_PD_TOGGLE_MS 1000U

Feeder_State feeder_state = FEEDER_IDLE;
int send_response=1;
uint8_t is_button_pressed=0;
uint32_t debug_motor_start_tick=0;
uint32_t advance_start_tick=0;
uint32_t led_blink_last_tick=0;
uint8_t motor_dir_forward=1; // 1=forward, 0=reverse
uint8_t swc_was_clear=0; // tracks SWC transition: only stop when SWC goes from clear -> triggered
uint8_t tape_was_blocked=0; // deterministic: tracks blocked state during advance (non-blocked→blocked→non-blocked=stop)
uint8_t advance_hole_phase=0; // advancing hole-detect phase machine (see advance_feeder).
                              // 0 = seek leading blocked edge (only when started unblocked),
                              // 1 = seek clear (hole), 2 = seek final blocked edge (-> stop)
uint32_t hole_intr_start_tick=0; // debounce: require interrupt to persist before stop
uint32_t last_button_release_tick=0;
uint8_t pull_pd_led_on=0;
uint32_t pull_pd_last_tick=0;
uint8_t advance_pulling_back=0; // 0=advancing forward, 1=pulling back cover

const int LIGHT_COUNTER=10000;
int indicateCounter=0;
//feeder data structure
Feeder_Data_Def feeder_data;

void (*finished_feeding)();

void start_motor(){
    if(motor_dir_forward){
        HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_SET);
    }
}

void stop_motor(){
    HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_SET);
    for(int i=0;i<5;++i);
    HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_RESET);
}

void led_on(){
    HAL_GPIO_WritePin(GPIOA,PIN_LED1,GPIO_PIN_RESET);
}
void led_off(){
    HAL_GPIO_WritePin(GPIOA,PIN_LED1,GPIO_PIN_SET);
}

void part_led_on(){
    HAL_GPIO_WritePin(GPIOB, PIN_PART_LED, GPIO_PIN_RESET); // active-low
}
void part_led_off(){
    HAL_GPIO_WritePin(GPIOB, PIN_PART_LED, GPIO_PIN_SET);
}

GPIO_PinState got_hole(){
    // PA12: HIGH = not interrupted, LOW = interrupted (internal pulldown)
    return (HAL_GPIO_ReadPin(GPIOA, PIN_PART_DET) == GPIO_PIN_RESET)
        ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void led_ind(){
    led_on();
    indicateCounter=LIGHT_COUNTER;
}

void advance_feeder(void (*processor)()){
    feeder_state = FEEDER_ADVANCING;
    finished_feeding = processor;
    send_response=1;
    advance_start_tick = HAL_GetTick();
    tape_was_blocked = 0;
    advance_pulling_back = 0;
    motor_dir_forward = 1;
    // Seed the detection phase from the current sensor reading so advancing
    // works whether the gear rests blocked or unblocked. After a feed the gear
    // sometimes reverses a little and ends up unblocked; in that case we must
    // first seek a blocked edge, then a clear edge, then stop on the next
    // blocked edge. If already blocked we just seek clear -> blocked.
    //   blocked at start    -> phase 1 (seek clear, then stop at blocked)
    //   unblocked at start  -> phase 0 (seek blocked -> seek clear -> stop)
    advance_hole_phase = (HAL_GPIO_ReadPin(GPIOA, PIN_PART_DET) == GPIO_PIN_RESET) ? 1 : 0;
    hole_intr_start_tick = 0;
    start_motor();
}

void read_feeder_data_from_flash(){
    read_flash((uint32_t*)&feeder_data, sizeof(feeder_data));   
}

void process_feeder(){
  GPIO_PinState isButton = HAL_GPIO_ReadPin(GPIOA,PIN_SW_B);

  // LED1 always reflects photo-interrupter status (inverted: blocked=OFF, clear=ON)
  if(HAL_GPIO_ReadPin(GPIOA, PIN_PART_DET) == GPIO_PIN_SET){
      led_off();  // PD1 HIGH = blocked
  } else {
      led_on();   // PD1 LOW = clear
  }

  if(!is_button_pressed && !isButton){  // active-low: pressed = LOW
    is_button_pressed = 1;
  }

  // Disabled for debugging: LED1 always shows photo-interrupter status
  // if(indicateCounter > 0){
  //   indicateCounter--;
  //   if(indicateCounter == 0){
  //       led_off();
  //   }
  // }

    // Detect button release with debounce
    uint8_t button_released_raw = is_button_pressed && isButton;
    uint8_t button_released = 0;
    if(button_released_raw){
        is_button_pressed = 0;
        uint32_t now = HAL_GetTick();
        if((now - last_button_release_tick) >= SW_RELEASE_DEBOUNCE_MS){
            button_released = 1;
            last_button_release_tick = now;
        }
    }

  switch(feeder_state){
    case FEEDER_IDLE:
        part_led_on();
        if(button_released){
#if FEEDER_TYPE == FEEDER_TYPE_AUTO
            motor_dir_forward = !motor_dir_forward;
            if(!motor_dir_forward){
                // Pull-back requested. If the cover is already pulled back to
                // position (SWC active-low = RESET), there is nothing to pull —
                // skip and stay idle. Leave motor_dir_forward at 0 so the next
                // press alternates back to advance.
                if(HAL_GPIO_ReadPin(GPIOA, PIN_SWC_IN) == GPIO_PIN_RESET){
                    break;
                }
            }
            debug_motor_start_tick = HAL_GetTick();
            swc_was_clear = 0;
            tape_was_blocked = 0;
            hole_intr_start_tick = 0;
            advance_pulling_back = 0;
            start_motor();
            feeder_state = FEEDER_AUTO_DEBUG;
#else
            feeder_state = FEEDER_ADVANCING;
            send_response = 0;
#endif
        }
        break;

    case FEEDER_ADVANCING:
        if(!advance_pulling_back){
            // Phase 1: Advance forward - deterministic detection that tolerates
            // either starting sensor state. After a feed the gear sometimes
            // reverses a little and ends up unblocked; see advance_feeder().
            //   start blocked    : seek clear (phase 1) -> STOP at blocked (phase 2)
            //   start unblocked  : seek blocked (phase 0) -> seek clear -> STOP at blocked
            part_led_on();
            {
                // pin LOW = photo-interrupter interrupted = blocked
                uint8_t blocked = (HAL_GPIO_ReadPin(GPIOA, PIN_PART_DET) == GPIO_PIN_RESET);
                if(advance_hole_phase == 0){
                    // started unblocked: wait for the leading blocked edge
                    if(blocked) advance_hole_phase = 1;
                } else if(advance_hole_phase == 1){
                    // saw blocked: wait for clear (hole)
                    if(!blocked) advance_hole_phase = 2;
                } else { // advance_hole_phase == 2: saw clear, wait for blocked -> STOP
                    if(blocked){
                        if(hole_intr_start_tick == 0){
                            // clear->blocked edge detected: brake immediately to shed momentum
                            hole_intr_start_tick = HAL_GetTick();
                            HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A1, GPIO_PIN_SET);
                            HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_SET);
                            for(volatile int i=0;i<20000;++i);  // brake hold
                            HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A1, GPIO_PIN_RESET);
                            HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_RESET);
                            motor_dir_forward = 1;
                            start_motor();
                        }
                        if(HAL_GetTick() - hole_intr_start_tick >= 50){
                            stop_motor();
                            hole_intr_start_tick = 0;
                            advance_hole_phase = 0;
                            // Switch to Phase 2: pull back cover
                            advance_pulling_back = 1;
                            motor_dir_forward = 0;
                            advance_start_tick = HAL_GetTick();
                            swc_was_clear = 0;
                            HAL_Delay(100);
                            start_motor();
                        }
                    } else {
                        hole_intr_start_tick = 0;
                    }
                }
            }
        } else {
            // Phase 2: Pull back tape cover until SWC triggers
            part_led_off();
            {
                if(HAL_GPIO_ReadPin(GPIOA, PIN_SWC_IN) == GPIO_PIN_SET){
                    swc_was_clear = 1;
                }
                if(swc_was_clear && HAL_GPIO_ReadPin(GPIOA, PIN_SWC_IN) == GPIO_PIN_RESET){
                    stop_motor();
                    feeder_state = FEEDER_IDLE;
                    advance_pulling_back = 0;
                    motor_dir_forward = 1;
                    if(send_response) finished_feeding();
                    break;
                }
                if(HAL_GetTick() - advance_start_tick >= 5000){
                    stop_motor();
                    feeder_state = FEEDER_IDLE;
                    advance_pulling_back = 0;
                    motor_dir_forward = 1;
                    if(send_response) finished_feeding();
                }
            }
        }
        break;

    case FEEDER_AUTO_DEBUG:
        if(button_released){
            motor_dir_forward = !motor_dir_forward;
            advance_pulling_back = motor_dir_forward ? 0 : 1; // forward=advancing, reverse=pulling back
            debug_motor_start_tick = HAL_GetTick();
            swc_was_clear = 0;
            tape_was_blocked = 0;
            hole_intr_start_tick = 0;
            if(motor_dir_forward){
                // Seed detection phase from current sensor state (see advance_feeder).
                advance_hole_phase = (HAL_GPIO_ReadPin(GPIOA, PIN_PART_DET) == GPIO_PIN_RESET) ? 1 : 0;
            }
            start_motor();
        }
        if(!motor_dir_forward){
            // Pulling tape: toggle PD LED, stop on SWC transition
            // Already in position (SWC active-low = RESET, never saw clear)?
            // Then there is nothing to pull — stop immediately.
            if(!swc_was_clear && HAL_GPIO_ReadPin(GPIOA, PIN_SWC_IN) == GPIO_PIN_RESET){
                stop_motor();
                feeder_state = FEEDER_IDLE;
                motor_dir_forward = 1;
                break;
            }
            if((HAL_GetTick() - pull_pd_last_tick) >= PULL_PD_TOGGLE_MS){
                pull_pd_last_tick = HAL_GetTick();
                pull_pd_led_on = !pull_pd_led_on;
                if(pull_pd_led_on) part_led_on();
                else part_led_off();
            }
            if(HAL_GPIO_ReadPin(GPIOA, PIN_SWC_IN) == GPIO_PIN_SET){
                swc_was_clear = 1;
            }
            if(swc_was_clear && HAL_GPIO_ReadPin(GPIOA, PIN_SWC_IN) == GPIO_PIN_RESET){
                stop_motor();
                feeder_state = FEEDER_IDLE;
                break;
            }
            if(HAL_GetTick() - debug_motor_start_tick >= 5000){
                stop_motor();
                feeder_state = FEEDER_IDLE;
            }
        } else {
            // Advancing tape: Phase 1 - deterministic detection, Phase 2 - pull back cover
            if(!advance_pulling_back){
                // Phase 1: Advance forward - deterministic detection (tolerates
                // either starting sensor state; see advance_feeder).
                part_led_on();
                // pin LOW = photo-interrupter interrupted = blocked
                uint8_t blocked = (HAL_GPIO_ReadPin(GPIOA, PIN_PART_DET) == GPIO_PIN_RESET);
                if(advance_hole_phase == 0){
                    if(blocked) advance_hole_phase = 1;
                } else if(advance_hole_phase == 1){
                    if(!blocked) advance_hole_phase = 2;
                } else { // advance_hole_phase == 2: wait for blocked -> STOP
                    if(blocked){
                        if(hole_intr_start_tick == 0){
                            // clear->blocked edge: brake to shed momentum
                            hole_intr_start_tick = HAL_GetTick();
                            HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A1, GPIO_PIN_SET);
                            HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_SET);
                            for(volatile int i=0;i<20000;++i);  // brake hold
                            HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A1, GPIO_PIN_RESET);
                            HAL_GPIO_WritePin(GPIOA,PIN_MOTOR_A2, GPIO_PIN_RESET);
                            motor_dir_forward = 1;
                            start_motor();
                        }
                        if(HAL_GetTick() - hole_intr_start_tick >= 50){
                            stop_motor();
                            hole_intr_start_tick = 0;
                            advance_hole_phase = 0;
                            // Switch to Phase 2: pull back cover
                            advance_pulling_back = 1;
                            motor_dir_forward = 0;
                            debug_motor_start_tick = HAL_GetTick();
                            swc_was_clear = 0;
                            HAL_Delay(100);
                            start_motor();
                        }
                    } else {
                        hole_intr_start_tick = 0;
                    }
                }
            } else {
                // Phase 2: Pull back tape cover until SWC triggers
                part_led_off();
                if(HAL_GPIO_ReadPin(GPIOA, PIN_SWC_IN) == GPIO_PIN_SET){
                    swc_was_clear = 1;
                }
                if(swc_was_clear && HAL_GPIO_ReadPin(GPIOA, PIN_SWC_IN) == GPIO_PIN_RESET){
                    stop_motor();
                    feeder_state = FEEDER_IDLE;
                    advance_pulling_back = 0;
                    motor_dir_forward = 1;
                    break;
                }
                if(HAL_GetTick() - debug_motor_start_tick >= 5000){
                    stop_motor();
                    feeder_state = FEEDER_IDLE;
                    advance_pulling_back = 0;
                    motor_dir_forward = 1;
                }
            }
        }
        break;

    default:
        break;
  }
}
