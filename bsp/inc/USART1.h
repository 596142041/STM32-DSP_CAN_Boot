#ifndef _USART1_H__
#define _USART1_H__
#include "user_config.h"
#include "stdio.h"
static void USART1_GPIO_Config(void);
static void USART1_Config(u16 baudrate);
static void USART1_NVIC_Config(void);
static void USART1_send_data(u8 data);
static u8 USART1_Get_data(void);
extern Sys_USART_structure USART1_struct;
#endif
