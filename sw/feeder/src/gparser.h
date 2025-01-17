#ifndef _G_PARSER_H
#define _G_PARSER_H
extern char msgBuf[255];

int8_t parse_gcode(char * toParse,UART_HandleTypeDef *UartHandle);
char *trimwhitespace(char *str);
void send_test_string();

void ok(UART_HandleTypeDef *UartHandle);
void parse_parameters(char * param, void (*processor)(char*,char*), void (*err)());
void just_ok();
#endif