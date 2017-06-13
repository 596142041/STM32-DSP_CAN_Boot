#ifndef __DELAY_H
#define __DELAY_H 
#include "stm32f4xx.h"
#include "user_config.h"
typedef struct
{
	union
	{
		__IO u32 all;
		struct
		{
			__IO u32 ENABLE:1;
			__IO u32 TICK_INT:1;
			__IO u32 CLKSOURCE:1;
			__IO u32 reserve:13;
			__IO u32 COUNTFLAG:1;
			__IO u32 reserve1:15;
		}bit;
	}STK_CTRL;
	union
	{
		__IO u32 all;
		struct
		{
			__IO u32 RELOAD:24;
			__IO u32 Reserved:8;
		}bit;
	}STK_LOAD;
	union
	{
		__IO u32 all;
		struct
		{ 
			__IO u32 CURRENT:24;
			__IO u32 Reserved:8;
		}bit;
	}STK_VAL;
	union
	{
		__I u32 all;
		struct
		{
			__I u32 TENMS:24;
			__I u32 Reserved:8;
		}bit;
	}STK_CALIB;
}Sys_Tick_Reg;
#define SYS_TICK_BASE ((u32)0xE000E010)
#define Sys_Tick  ((Sys_Tick_Reg *)SysTick_BASE)
void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus); 
#endif
