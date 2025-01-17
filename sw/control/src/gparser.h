#ifndef _G_PARSER_H
#define _G_PARSER_H

typedef struct {
  int8_t contentFlag; //if flash having content or not, after erased, the value is -1
  int16_t offsetXx10; //10 times offset X
  int16_t offsetYx10; //10 times offset Y 
  int16_t offsetZx10; //10 times offset Z
}Control_Data_Def;


int8_t parse_gcode(char * toParse,UART_HandleTypeDef *UartHandle);
void parser_init();
void set_control_channel(uint16_t pin, int val);
#endif