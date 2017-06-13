#include "delay.h" 
#include "stdio.h"
#include "USARt1.h"
#include "sdio_sd.h"
#include "ff.h" 
#include "CANA.h"
#include "IAP.h"
CanTxMsg can_tx_msg;
u8  temp = 0x01;
u8 Status = 0xDE;
SD_Error status = SD_OK;
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init(168);
	CANA_Config();
	can_tx_msg.DLC = 1;
	can_tx_msg.IDE = CAN_ID_EXT;
	can_tx_msg.RTR = CAN_RTR_DATA;
	status = SD_Init(); 
	 
	if(status == SD_OK)
	{
		
		Status             = CAN_IAP_Hex(0x134);
		CAN1_CanRxMsgFlag  = 0;
		can_tx_msg.Data[0] = Status; 
		CAN_WriteData(&can_tx_msg);
	}
	else
	{
		can_tx_msg.ExtId   = 0x1235;

		CAN1_CanRxMsgFlag  = 0;
		can_tx_msg.Data[0] = Status; 
		CAN_WriteData(&can_tx_msg);
	}

	while (1)
	{ 
		if(CAN1_CanRxMsgFlag)
		{
			CAN1_CanRxMsgFlag = 0;
			can_tx_msg.Data[0] = Status;
			temp++;
			temp%=5;
			CAN_WriteData(&can_tx_msg);
			
		}
	}
}
