#ifndef __CAN_APP_H
#define __CAN_APP_H 
#include "user_config.h"
#include "can_driver.h"
#include "delay.h" 
#define CMD_WIDTH   4
#define CMD_MASK    0xF
#define ADDR_MASK   0x1FFFFFF

#define APP_EXE_FLAG_START_ADDR    ((uint32_t)0x08004000)
#define APP_START_ADDR             ((uint32_t)0x08008000)
#define APP_EXE_FLAG_ADDR          ((uint32_t)0x08007800)

#define CAN_BL_BOOT     0x55555555
#define CAN_BL_APP      0xAAAAAAAA
#define FW_TYPE         CAN_BL_APP

typedef struct _CBL_CMD_LIST{
  //Bootloader�������
  unsigned char Erase;        //����APP������������
  unsigned char WriteInfo;    //���ö��ֽ�д������ز�����д��ʼ��ַ����������
  unsigned char Write;        //�Զ��ֽ���ʽд����
  unsigned char Check;        //���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
  unsigned char SetBaudRate;  //���ýڵ㲨����
  unsigned char Excute;       //ִ�й̼�
  //�ڵ㷵��״̬
  unsigned char CmdSuccess;   //����ִ�гɹ�
  unsigned char CmdFaild;     //����ִ��ʧ��
} CBL_CMD_LIST;  
//-------------------------------
typedef struct
{
	union
	{
		u32 all;
		struct
		{
			u16 cmd:4;
			u16 addr:12;
			u16 reserve:16;
		}bit;
	}ExtId;
	unsigned char IDE;   
	unsigned char DLC;   
	u8 data[8];
}bootloader_data;
uint32_t GetSector(uint32_t Address);
FLASH_Status CAN_BOOT_ErasePage(uint32_t StartPageAddr,uint32_t EndPageAddr);
uint16_t CAN_BOOT_GetAddrData(void);
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage);
void CAN_BOOT_JumpToApplication(__IO uint32_t Addr);
FLASH_Status CAN_BOOT_ProgramDatatoFlash(uint32_t StartAddress,uint8_t *pData,uint32_t DataNum);
#endif
/*********************************END OF FILE**********************************/

