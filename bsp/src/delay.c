#include "delay.h"
static u8  fac_us=0;
static u16 fac_ms=0;
/********************************************
*函数名:void delay_init(u8 SYSCLK)
*输入参数:u8 SYSCLK 系统时钟频率
*返回值:无;
*备注:systick 定时器的时钟是系统时钟的8分频,
* 因此最大的计数时间为780ms
*********************************************/
void delay_init(u8 SYSCLK)
{
	Sys_Tick->STK_CTRL.bit.CLKSOURCE = 0;
	Sys_Tick->STK_CTRL.bit.TICK_INT = 0;
	Sys_Tick->STK_CTRL.bit.ENABLE = 0;
	Sys_Tick->STK_VAL.bit.CURRENT = 0;//时钟为21MHz
	fac_us = SYSCLK>>3;
	fac_ms = fac_us*1000;
	 
}
/*************************************************
*函数名:void delay_ms(u16 nms)
*输入参数:u16 nms 延时时间(不能大于780)
*返回值:无; 
**************************************************/
void delay_ms(u16 nms)
{
	//	u16 n;
	Sys_Tick->STK_LOAD.bit.RELOAD = nms*fac_ms;
	Sys_Tick->STK_CTRL.bit.ENABLE = 1;
	while(Sys_Tick->STK_CTRL.bit.COUNTFLAG == 0)
	{
		;
	}	
	Sys_Tick->STK_CTRL.bit.ENABLE = 0; 
	Sys_Tick->STK_VAL.bit.CURRENT = 0;
}
void delay_us(u32 nus)
{
 	
	Sys_Tick->STK_LOAD.bit.RELOAD = nus*fac_us;
	Sys_Tick->STK_CTRL.bit.ENABLE = 1;
	while(Sys_Tick->STK_CTRL.bit.COUNTFLAG == 0)
	{
		;
	}	
	Sys_Tick->STK_CTRL.bit.ENABLE = 0; 
	Sys_Tick->STK_VAL.bit.CURRENT = 0; 
}

