#include "CANA.h"
#include "IAP.h"
extern CanRxMsg CAN1_RxMessage;
extern volatile uint8_t CAN1_CanRxMsgFlag;
extern Boot_CMD_LIST cmd_list;
void CANA_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_init;
	RCC_AHB1PeriphClockCmd(CAN_Tx_Port_CLK|CAN_Rx_Port_CLK,ENABLE);
	GPIO_PinAFConfig(CAN_Tx_Port,GPIO_PinSource13,GPIO_AF_CAN1);
	GPIO_PinAFConfig(CAN_Rx_Port,GPIO_PinSource9,GPIO_AF_CAN1);
	GPIO_init.GPIO_Mode = GPIO_Mode_AF;
	GPIO_init.GPIO_OType  = GPIO_OType_PP;
	GPIO_init.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_init.GPIO_Pin = CAN_Tx_Pin;
	GPIO_Init(CAN_Tx_Port,&GPIO_init);
	GPIO_init.GPIO_Pin = CAN_Rx_Pin;
	GPIO_Init(CAN_Rx_Port,&GPIO_init);
	
}
void CANA_Config(void)
{
	CAN_InitTypeDef        CAN_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,ENABLE);
	CANA_IT_Config();
	CANA_GPIO_Config();
	CAN_DeInit(CAN1); 
	CAN_StructInit(&CAN_InitStructure); 
	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = ENABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; 
	CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_Prescaler = 4;//500kbps

	CAN_Init(CAN1,&CAN_InitStructure);
	//设置CAN接收过滤器
	CAN_ConfigFilter(2,cmd_list.CmdSuccess);
	CAN_ConfigFilter(0,cmd_list.CmdFaild);
	//使能接收中断
	CAN_ITConfig(CAN1,CAN_IT_FMP0, ENABLE);
}
/**
  * @brief  配置CAN接收过滤器
  * @param  FilterNumber 过滤器号
  * @param  can_addr CAN节点地址，该参数非常重要，同一个CAN总线网络其节点地址不能重复
  * @retval None
  */
void CAN_ConfigFilter(uint8_t FilterNumber,uint16_t can_addr)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure; 
	u32 addr_temp; 
	addr_temp = can_addr;
	//设置CAN接收过滤器
	CAN_FilterInitStructure.CAN_FilterNumber=FilterNumber;//过滤器1
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;//屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;//32bit模式

	CAN_FilterInitStructure.CAN_FilterIdHigh=addr_temp>>13;
	CAN_FilterInitStructure.CAN_FilterIdLow=((addr_temp<<3)&0xFFFF)|0x04;

	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x1FFFFFFF>>(13); 
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=(((((u16)0x1FFFFFFF)<<3))&0x78)|0x04;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=0;
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;//使能过滤器
	CAN_FilterInit(&CAN_FilterInitStructure);	
}
void CANA_IT_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
uint8_t CAN_WriteData(CanTxMsg *TxMessage)
{
	uint8_t   TransmitMailbox;   
	uint32_t  TimeOut=0;
	TransmitMailbox = CAN_Transmit(CAN1,TxMessage);
	while(CAN_TransmitStatus(CAN1,TransmitMailbox)!=CAN_TxStatus_Ok)
	{
		TimeOut++;
		if(TimeOut > 10000000)
		{
			return 1;
		}
	}
	return 0;
}
/**
  * @brief  CAN接收中断处理函数
  * @param  None
  * @retval None
  */
void CAN1_RX0_IRQHandler(void)
{
	if(CAN_GetITStatus(CAN1,CAN_IT_FMP0) == SET)
	{
		CAN_Receive(CAN1,CAN_FIFO0, &CAN1_RxMessage);
		CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);
		CAN1_CanRxMsgFlag = 1;
		
	}

}
