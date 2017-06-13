#include "delay.h"
static u8  fac_us=0;
static u16 fac_ms=0;
/********************************************
*������:void delay_init(u8 SYSCLK)
*�������:u8 SYSCLK ϵͳʱ��Ƶ��
*����ֵ:��;
*��ע:systick ��ʱ����ʱ����ϵͳʱ�ӵ�8��Ƶ,
* ������ļ���ʱ��Ϊ780ms
*********************************************/
void delay_init(u8 SYSCLK)
{
	Sys_Tick->STK_CTRL.bit.CLKSOURCE = 0;
	Sys_Tick->STK_CTRL.bit.TICK_INT = 0;
	Sys_Tick->STK_CTRL.bit.ENABLE = 0;
	Sys_Tick->STK_VAL.bit.CURRENT = 0;//ʱ��Ϊ21MHz
	fac_us = SYSCLK>>3;
	fac_ms = fac_us*1000;
	 
}
/*************************************************
*������:void delay_ms(u16 nms)
*�������:u16 nms ��ʱʱ��(���ܴ���780)
*����ֵ:��; 
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

