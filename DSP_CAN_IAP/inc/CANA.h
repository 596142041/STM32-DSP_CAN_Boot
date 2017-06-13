#ifndef __CANA_H__
#define __CANA_H__
#include "user_config.h"
#define CAN_Tx_Port      GPIOH
#define CAN_Tx_Pin       GPIO_Pin_13
#define CAN_Tx_Port_CLK  RCC_AHB1Periph_GPIOH 
#define CAN_Rx_Port      GPIOI
#define CAN_Rx_Pin       GPIO_Pin_9
#define CAN_Rx_Port_CLK  RCC_AHB1Periph_GPIOI
void CANA_GPIO_Config(void);
void CANA_Config(void);
void CANA_RX_Config(void);
void CANA_IT_Config(void);
void CAN_ConfigFilter(uint8_t FilterNumber,uint16_t can_addr);
uint8_t CAN_WriteData(CanTxMsg *TxMessage);
#endif
