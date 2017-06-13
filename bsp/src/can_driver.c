/**
  ******************************************************************************
  * @file    can_driver.c
  * $Author: wdluo $
  * $Revision: 17 $
  * $Date:: 2012-07-06 11:16:48 +0800 #$
  * @brief   CAN�����շ���غ���.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, ViewTool</center>
  *<center><a href="http:\\www.viewtool.com">http://www.viewtool.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can_app.h"
#define CAN_Tx_Port      GPIOH
#define CAN_Tx_Pin       GPIO_Pin_13
#define CAN_Tx_Port_CLK  RCC_AHB1Periph_GPIOH 
#define CAN_Rx_Port      GPIOI
#define CAN_Rx_Pin       GPIO_Pin_9
#define CAN_Rx_Port_CLK  RCC_AHB1Periph_GPIOI
/* Private typedef -----------------------------------------------------------*/
typedef  struct {
  unsigned char   SJW;
  unsigned char   BS1;
  unsigned char   BS2;
  unsigned short  PreScale;
} tCAN_BaudRate;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
CanRxMsg CAN1_RxMessage;
volatile uint8_t CAN1_CanRxMsgFlag=0;//���յ�CAN���ݺ�ı�־

tCAN_BaudRate  CAN_BaudRateInitTab[]= {      // CLK=84MHz
   {CAN_SJW_1tq,CAN_BS1_2tq,CAN_BS2_1tq,21},     // 1M
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_1tq,21},     // 500K
   {CAN_SJW_1tq,CAN_BS1_4tq,CAN_BS2_1tq,70},    // 200K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_1tq,105},    // 100K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_1tq,210},    // 50K
   {CAN_SJW_1tq,CAN_BS1_6tq,CAN_BS2_1tq,525},   // 20K
};
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  ͨ�������ʵ�ֵ��ȡ�����ʲ���������ֵ
  * @param  BaudRate CAN���߲����ʣ���λΪbps
  * @retval �����ʲ���������ֵ
  */
uint32_t CAN_GetBaudRateNum(uint32_t BaudRate)
{
  switch(BaudRate){
    case 1000000 :return 0;
    case 500000 :return 1;
    case 200000 :return 2;
    case 100000 :return 3;
    case 50000 :return 4;
    case 20000 :return 5;
    default:return 0;
  }
}



/**
  * @brief  CAN��������
  * @param  None
  * @retval None
  */
void CAN_GPIO_Configuration(void)
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
/**
  * @brief  CAN�����ж�����
  * @param  None
  * @retval None
  */
void CAN_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
 

  // Enable CAN1 RX0 interrupt IRQ channel
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
/**
  * @brief  ����CAN���չ�����
  * @param  FilterNumber ��������
  * @param  can_addr CAN�ڵ��ַ���ò����ǳ���Ҫ��ͬһ��CAN����������ڵ��ַ�����ظ�
  * @retval None
  */
void CAN_ConfigFilter(uint8_t FilterNumber,uint16_t can_addr)
{
  CAN_FilterInitTypeDef  CAN_FilterInitStructure; 
	u32 addr_temp;
	addr_temp = can_addr<<CMD_WIDTH;//�������ID������λ��������ԭ�������յ�IDλ��
  //����CAN���չ�����
  CAN_FilterInitStructure.CAN_FilterNumber=FilterNumber;//������1
  CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;//����λģʽ
  CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;//32bitģʽ
	
  CAN_FilterInitStructure.CAN_FilterIdHigh=addr_temp>>13;
  CAN_FilterInitStructure.CAN_FilterIdLow=((addr_temp<<3)&0xFFFF)|0x04;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=ADDR_MASK>>(13);
  CAN_FilterInitStructure.CAN_FilterMaskIdLow=((ADDR_MASK<<3)&0xFF87)|0x04;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment=0;
  CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;//ʹ�ܹ�����
  CAN_FilterInit(&CAN_FilterInitStructure);	
}
/**
  * @brief  ��ʼ��CAN
  * @param  BaudRate CAN���߲�����
  * @retval None
  */
void CAN_Configuration(uint32_t BaudRate)
{
  CAN_InitTypeDef        CAN_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,ENABLE);
  /* CAN register init */
	
  CAN_NVIC_Configuration();
  CAN_GPIO_Configuration();
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
	/*
  CAN_InitStructure.CAN_SJW = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].SJW;//���ò�����Ϊ1M
  CAN_InitStructure.CAN_BS1 = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].BS1;
  CAN_InitStructure.CAN_BS2 = CAN_BaudRateInitTab[CAN_GetBaudRateNum(BaudRate)].BS2;
	*/
	CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_Prescaler = 4;

  CAN_Init(CAN1,&CAN_InitStructure);
  //����CAN���չ�����
  CAN_ConfigFilter(0,0x00);//�㲥��ַ�����ܹ㲥����
  CAN_ConfigFilter(1,CAN_BOOT_GetAddrData());//���ڵ���ʵ��ַ

  //ʹ�ܽ����ж�
  CAN_ITConfig(CAN1,CAN_IT_FMP0, ENABLE);
}


/**
  * @brief  ����һ֡CAN����
  * @param  CANx CANͨ����
	* @param  TxMessage CAN��Ϣָ��
  * @retval None
  */
uint8_t CAN_WriteData(CanTxMsg *TxMessage)
{
  uint8_t   TransmitMailbox;   
  uint32_t  TimeOut=0;
  TransmitMailbox = CAN_Transmit(CAN1,TxMessage);
  while(CAN_TransmitStatus(CAN1,TransmitMailbox)!=CAN_TxStatus_Ok){
    TimeOut++;
    if(TimeOut > 10000000){
      return 1;
    }
  }
  return 0;
}
/**
  * @brief  CAN�����жϴ�����
  * @param  None
  * @retval None
  */
void CAN1_RX0_IRQHandler(void)
{
  CAN_Receive(CAN1,CAN_FIFO0, &CAN1_RxMessage);
  CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);
  CAN1_CanRxMsgFlag = 1;
}

/**
  * @brief  ��ȡCAN�ڵ��ַ���ú��������Լ���ʵ����������޸�
  * @param  None
  * @retval None
  */
uint16_t Read_CAN_Address(void)
{
  return 0x123;//���صĵ�ֵַ��Ҫ����ʵ����������޸�
}

/**
  * @brief  ��ʼ�����û�ȡCAN���ߵ�ַ��GPIO����
  * @param  None
  * @retval None
  */
void CAN_Address_GPIO_Config(void)
{
  //�����Լ��İ�����ɶ�Ӧ�ĳ���
}

/*********************************END OF FILE**********************************/
