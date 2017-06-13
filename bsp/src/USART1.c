#include "USARt1.h"
Sys_USART_structure USART1_struct = 
{
	.GPIO_init = USART1_GPIO_Config,
	.USART_init = USART1_Config,
	.USART_IT_Config = NULL,
	.send_data = USART1_send_data,
	.receive_data = USART1_Get_data,
	.USART = USART1,
};
static void USART1_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_init;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//??IO????
       
	GPIO_init.GPIO_Mode = GPIO_Mode_AF;
	GPIO_init.GPIO_OType = GPIO_OType_PP;
	GPIO_init.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
	GPIO_init.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_init);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
}
static void USART1_Config(u16 baudrate)
{
	USART_InitTypeDef  USART_init;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	USART1_GPIO_Config();
	USART_init.USART_BaudRate = baudrate;
	USART_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_init.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	USART_init.USART_Parity = USART_Parity_No;//???
	USART_init.USART_StopBits = USART_StopBits_1;
	USART_init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1_struct.USART,&USART_init);
	USART1_NVIC_Config();
	USART_Cmd(USART1_struct.USART,ENABLE);
}
static void USART1_send_data(u8 data)
{
	USART_SendData(USART1_struct.USART, (unsigned char) data);
	while (USART_GetFlagStatus(USART1_struct.USART, USART_FLAG_TXE) == RESET);
//	return (ch);
}
static void USART1_NVIC_Config(void)
{
	/*
	NVIC_InitTypeDef USART1_NVIC_init;
	USART_ITConfig(UART4,USART_IT_RXNE,ENABLE);
	USART1_NVIC_init.NVIC_IRQChannel = USART1_IRQn;
	USART1_NVIC_init.NVIC_IRQChannelCmd = ENABLE;
	USART1_NVIC_init.NVIC_IRQChannelPreemptionPriority = 10;
	USART1_NVIC_init.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&USART1_NVIC_init);
	*/
} 
static u8 USART1_Get_data(void)
{
	u8 temp;
	temp = USART_ReceiveData(USART1_struct.USART);
	return temp;
}
