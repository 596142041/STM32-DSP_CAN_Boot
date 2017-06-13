#include "can_app.h" 
typedef  void (*pFunction)(void);
//��������������λ��������������һ��
CBL_CMD_LIST CMD_List = 
{
  .Erase         = 0x00,      //����APP��������
  .WriteInfo     = 0x01,      //���ö��ֽ�д������ز�����д��ʼ��ַ����������
  .Write         = 0x02,      //�Զ��ֽ���ʽд����
  .Check         = 0x03,      //���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
  .SetBaudRate   = 0x04,      //���ýڵ㲨����
  .Excute        = 0x05,      //ִ�й̼�
  .CmdSuccess    = 0x08,      //����ִ�гɹ�
  .CmdFaild      = 0x09,      //����ִ��ʧ��
};
/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;  
  }

  return sector;
}
/**
  * @brief  ��������д��ָ����ַ��Flash�� ��
  * @param  Address Flash��ʼ��ַ��
  * @param  Data ���ݴ洢����ʼ��ַ��
  * @param  DataNum �����ֽ�����
  * @retval ������д״̬��
  */
FLASH_Status CAN_BOOT_ProgramDatatoFlash(uint32_t StartAddr,uint8_t *pData,uint32_t DataNum) 
{
  FLASH_Status FLASHStatus=FLASH_COMPLETE;

  uint32_t *pDataTemp=(uint32_t *)pData;
  uint32_t i;

  if(StartAddr<APP_EXE_FLAG_START_ADDR)
	{
    return FLASH_ERROR_PGS;
  }
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

  for(i=0;i<(DataNum>>2);i++)
  {
    FLASHStatus = FLASH_ProgramWord(StartAddr, *pDataTemp);
    if (FLASHStatus == FLASH_COMPLETE)
		{
      StartAddr += 4;
      pDataTemp++;
    }
		else
		{ 
      return FLASHStatus;
    }
  }
  return	FLASHStatus;
}
/**
  * @brief  ����ָ�����������Flash���� ��
  * @param  StartPage ��ʼ������ַ
  * @param  EndPage ����������ַ
  * @retval ��������״̬  
  */
FLASH_Status CAN_BOOT_ErasePage(uint32_t StartAddr,uint32_t EndAddr)
{
  FLASH_Status FLASHStatus=FLASH_COMPLETE;
  uint32_t StartSector, EndSector;
  uint32_t SectorCounter=0;
  FLASH_Unlock();
  /* Get the number of the start and end sectors */
  StartSector = GetSector(StartAddr);
  EndSector = GetSector(EndAddr);

  if(StartAddr<APP_EXE_FLAG_START_ADDR)
	{
    return FLASH_ERROR_PGS;
  }
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
  for (SectorCounter = StartSector; SectorCounter <= EndSector; SectorCounter += 8)
  { 
    FLASHStatus = FLASH_EraseSector(SectorCounter, VoltageRange_3);
    if (FLASHStatus != FLASH_COMPLETE)
    {
      FLASH_Lock();
      return FLASHStatus;
    }
  }
  FLASH_Lock();
  return FLASHStatus;
}



/**
  * @brief  ��ָ����Flash����ȡһ���ֵ�����
  * @param  Address ��ʼ��ȡ���ݵĵ�ַ��
	* @param  pData ���ݴ����ַ��
  * @retval ������ת״̬��
  */
uint16_t CAN_BOOT_GetAddrData(void)
{
  return Read_CAN_Address();
}
/**
  * @brief  ���Ƴ�����ת��ָ��λ�ÿ�ʼִ�� ��
  * @param  Addr ����ִ�е�ַ��
  * @retval ������ת״̬��
  */
void CAN_BOOT_JumpToApplication(__IO uint32_t Addr)
{
	pFunction Jump_To_Application;
	__IO uint32_t JumpAddress; 
	if (((*(__IO uint32_t*)Addr) & 0x2FFE0000 ) == 0x20000000)
	{  
	  JumpAddress = *(__IO uint32_t*) (Addr + 4);
	  Jump_To_Application = (pFunction) JumpAddress;
		__set_PRIMASK(1);//�ر������ж� 
	  __set_MSP(*(__IO uint32_t*)Addr);
	  Jump_To_Application();
	}
}


/**
  * @brief  ִ�������·�������
  * @param  pRxMessage CAN������Ϣ
  * @retval ��
  */
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage)
{
	CanTxMsg TxMessage;
	uint8_t can_cmd = (pRxMessage->ExtId)&CMD_MASK;//ID��bit0~bit3λΪ������
	uint16_t can_addr = (pRxMessage->ExtId >> CMD_WIDTH);//ID��bit4~bit15λΪ�ڵ��ַ
	uint32_t BaudRate;
	uint32_t exe_type;
	//�жϽ��յ����ݵ�ַ�Ƿ�ͱ��ڵ��ַƥ�䣬����ƥ����ֱ�ӷ��أ������κ�����
	if((can_addr!=CAN_BOOT_GetAddrData())&&(can_addr!=0))
	{
		return;
	}
	TxMessage.DLC = 0;
	TxMessage.ExtId = 0;
	TxMessage.IDE = CAN_Id_Extended;
	TxMessage.RTR = CAN_RTR_Data; 
  //CMD_List.SetBaudRate�����ýڵ㲨���ʣ����岨������Ϣ�洢��Data[0]��Data[3]��
  //���Ĳ����ʺ�������Ҳ��Ҫ����Ϊ��ͬ�Ĳ����ʣ�����������ͨ��
	if(can_cmd == CMD_List.SetBaudRate)
	{
		BaudRate = (pRxMessage->Data[0]<<24)|(pRxMessage->Data[1]<<16)|(pRxMessage->Data[2]<<8)|(pRxMessage->Data[3]<<0);
		CAN_Configuration(BaudRate);
		if(can_addr != 0x00)
		{
			TxMessage.ExtId = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdSuccess;
			TxMessage.DLC = 0;
			delay_ms(20);
			CAN_WriteData(&TxMessage);
		}
		return;
	}
  //CMD_List.Check���ڵ����߼��
  //�ڵ��յ�������󷵻ع̼��汾��Ϣ�͹̼����ͣ���������Bootloader�����APP���򶼱���ʵ��
	if(can_cmd == CMD_List.Check)
	{
		if(can_addr != 0x00)
		{
			TxMessage.ExtId = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|CMD_List.CmdSuccess;
			TxMessage.Data[0] = 0;//���汾�ţ����ֽ�
			TxMessage.Data[1] = 1;
			TxMessage.Data[2] = 0;//�ΰ汾�ţ����ֽ�
			TxMessage.Data[3] = 0;
			TxMessage.Data[4] = (uint8_t)(FW_TYPE>>24);
			TxMessage.Data[5] = (uint8_t)(FW_TYPE>>16);
			TxMessage.Data[6] = (uint8_t)(FW_TYPE>>8);
			TxMessage.Data[7] = (uint8_t)(FW_TYPE>>0);
			TxMessage.DLC = 8;
			CAN_WriteData(&TxMessage);
		}
		return;
	}
  //CMD_List.Excute�����Ƴ�����ת��ָ����ִַ��
  //��������Bootloader��APP�����ж�����ʵ��
  if(can_cmd == CMD_List.Excute)
  {
    exe_type = (pRxMessage->Data[0]<<24)|(pRxMessage->Data[1]<<16)|(pRxMessage->Data[2]<<8)|(pRxMessage->Data[3]<<0);
    if(exe_type == CAN_BL_BOOT)
	{
      FLASH_Unlock();
      CAN_BOOT_ErasePage(APP_EXE_FLAG_ADDR,APP_EXE_FLAG_ADDR);//����д�뵽Flash�е�APPִ�б�־����λ���к󣬼���ִ��Bootloader����
      FLASH_Lock();
      __set_PRIMASK(1);//�ر������ж�
      NVIC_SystemReset();
    }
    return;
  }
}
/*********************************END OF FILE**********************************/

