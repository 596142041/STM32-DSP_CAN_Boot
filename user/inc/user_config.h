#ifndef __user_config_H__
#define __user_config_H__
#include "stm32f4xx.h"
#include "arm_math.h"
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */ 
#define Channel_N   3//有几个采样通道
#define Channel_M   9//每个通道采集几次数据
typedef struct
{
	float sys_clk;
	float AHB_clk;
	float APB1_clk;
	float APB2_clk;
} System_Cloock;
//-----------------
//ADC的采样数据结构
typedef struct
{
	void (*GPIO_init)(void);
	void (*ADC_IT_init)(void);
	void (*ADC_init)(void);
	void (*DMA_init)(u16 *data,u16 size);
	//----------------------------------
	u16 AD_data[Channel_M][Channel_N];
	float adc_data[Channel_N];
	void (*adc_calc)(u16 data[Channel_M][Channel_N],float value[Channel_N]);
	float ADC_VERF;
} Sys_ADC_structure;
//-----------------------------------
//串口工作数据结构
typedef struct
{
	void (*GPIO_init)(void);
	void (*USART_init)(u16 baudrate);
	void (*USART_IT_Config)(void);
	void (*send_data)(u8 data);
	void (*Tx_EN)(void);
	void (*Rx_EN)(void);
	u8 (*receive_data)(void);
	USART_TypeDef* USART;
	GPIO_TypeDef*  GPIO;  
	uint32_t  Tx_Pin; 
	uint32_t  Rx_Pin;
	
  //uint32_t  Periph_RCC; 	
} Sys_USART_structure;
//----------------------------------------
//定时器数据结构
typedef struct
{
	void (*GPIO_init)(void);
	void (*TIM_base_init)(u16 psc,u16 period);
	void (*TIM_PWM_OUT)(void);
	void (*TIM_PWM_IN)(void);
	void (*TIM_IT_init)(void);
} Sys_TIM_structure;
//--------------
typedef struct
{
	u8 buffer[140];
	u16 receive_counter;
	u8 receive_flag;
	u16 packge_num;
} Xmodem_struct;
//Xmodem  宏定义
typedef struct 
{
	u16 Sin[400];
	u16 cnt;
} Sin_table;
#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CTRLZ 0x1A
void Get_System_Clock(System_Cloock *pointer);
void System_Clock_Config(void);
extern Sin_table sin_table;
typedef struct
{
	unsigned char data[8];
} CAN_MSG_byte;
typedef struct
{
	unsigned char byte0;
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
	unsigned char byte5;
	unsigned char byte6;
	unsigned char byte7;
}CAN_MSG_BYTE;
typedef struct
{
	unsigned long int MSG_MD_L;
	unsigned long int MSG_MD_H;
}CAN_MSG_DWORD;
typedef union
{
	CAN_MSG_byte msg_byte;
	CAN_MSG_BYTE msg_Byte;
	CAN_MSG_DWORD msg_Dword;
}CAN_MSG_DATA;
typedef struct
{
  unsigned short int StdId;  /*!< Specifies the standard identifier.
                        This parameter can be a value between 0 to 0x7FF. */

  unsigned long int ExtId;  /*!< Specifies the extended identifier.
                        This parameter can be a value between 0 to 0x1FFFFFFF. */

  unsigned char IDE;     /*!< Specifies the type of identifier for the message that
                        will be transmitted. This parameter can be a value
                        of @ref CAN_identifier_type */

  unsigned char DLC;     /*!< Specifies the length of the frame that will be
                        transmitted. This parameter can be a value between
                        0 to 8 */

  CAN_MSG_DATA CAN_Msg_data; /*!< Contains the data to be transmitted. It ranges from 0
                        to 0xFF. */
} Can_Tx_Msg;
typedef union
{
	CAN_MSG_BYTE CAN_msg_data_BYTE;
	CAN_MSG_byte CAN_msg_data_byte;
}CAN_msg_data;
#endif
