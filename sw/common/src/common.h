#ifndef _PARAMETER_PARSER_H_
#define _PARAMETER_PARSER_H_
#include "string.h"
#include "stdint.h"
#include "stdio.h"
#include "py32f0xx_hal.h"


#define FLASH_USER_START_ADDR     0x08004F00

void parse_parameters(char * param, void (*processor)(char*,char*), void (*err)());
char* uid_to_string(uint32_t w1, uint32_t w2, uint32_t w3);
void write_flash(uint32_t * data, uint32_t size);
void read_flash(uint32_t * data, uint32_t size);
#endif