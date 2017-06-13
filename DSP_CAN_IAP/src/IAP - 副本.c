#include "IAP.h"
bootloader_data Bootloader_data;
FIL fp;
FATFS IAP_fatfs; 
FIL fp_hex;
FSIZE_t hex_size = 0x00;
FRESULT res;
PACK_INFO pack_info;
SEND_INFO send_data;
unsigned char read_buf[128];
unsigned char bin_buf[128];
UINT read_cnt;
FSIZE_t file_size = 0;
UINT calc_temp = 0;
unsigned char read_temp[66];
//int send_cnt; //��ʾCAN���߷���64�ֽڴ���; 
/*
Ŀǰ�ƻ���ÿ�ζ�ȡ2�з���һ������,��DSP��2������Ӧ����64�ֽ�,����STM32��2�иպ���32�ֽ�
*/
//---------------------------
CanRxMsg CAN1_RxMessage;
volatile uint8_t CAN1_CanRxMsgFlag=0;
Boot_CMD_LIST cmd_list =
{
	.SetBaudRate = 0x04,      //����APP��������
	.WriteInfo = 0x01,//���ö��ֽ�д������ز�����д��ʼ��ַ����������
	.Write = 0x02,//�Զ��ֽ���ʽд����
	.Check = 0x03,//���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
	.Erase = 0x00,//���ýڵ㲨����
	.Excute = 0x05,//ִ�й̼�
	.CmdSuccess = 0x08,//����ִ�гɹ�
	.CmdFaild = 0x09,//����ִ��ʧ��
	
};
void IAP_delay(unsigned short int n)
{
	while (n)
	{
		n--;
	}
}
/*******************************************
 *������:unsigned short int CRCcalc16 (unsigned char *data,unsigned short int len)
 *�������:unsigned char *data:CRCУ������,unsigned short int len:CRCУ�鳤��
 *����ֵ:unsigned short int crc_res:CRCУ����,��λ��ǰ
 *��ע:��CRCУ����õ���Modbus��CRCУ�鷽ʽ;
 *******************************************/
unsigned short int CRCcalc16(unsigned char *data, unsigned short int len)
{

	int i;
	unsigned short int crc_res = 0xFFFF;
	while (len--)
	{
		crc_res ^= *data++;
		for (i = 0; i < 8; i++)
		{
			if (crc_res & 0x01)
			{
				crc_res = ( crc_res >> 1 ) ^ 0xa001;
			}
			else
			{
				crc_res = ( crc_res >> 1 );
			}
		}
	}
	return crc_res;
}
/*********************************************
 *������:u8 File_send(void);
 *�������:��;
 *����ֵ:u8 res:�ɹ�����0;
 *
 ********************************************/

u8 CAN_IAP_Bin(void)
{
	int i = 0;   //��������
	UINT crc_value = 0x0000;
	CanTxMsg TxMessage;
	unsigned char cnt = 0;
	unsigned long int CAN_BL_temp;   //��ǰ�豸���еĹ̼�����
	FRESULT res;
	Bootloader_data.ExtId.bit.reserve = 0;
	unsigned short int time_out = 0x00;
	for (i = 0; i < 8; i++)
	{
	//	can_tx_msg.CAN_Tx_msg_data.msg_byte.data[i] = 0;
		//can_rx_msg.CAN_Rx_msg_data.msg_byte.data[i] = 0;
		TxMessage.Data[i] = 0;
	}
	//--��ѯ��ǰ���еĳ�����APP����BOOT;
	Bootloader_data.DLC = 0;
	Bootloader_data.ExtId.bit.cmd = cmd_list.Check;
	Bootloader_data.ExtId.bit.addr = 0x123;
	Bootloader_data.ExtId.bit.reserve = 0;
	Bootloader_data.IDE = CAN_ID_EXT;
	TxMessage.DLC = Bootloader_data.DLC;
	TxMessage.IDE = Bootloader_data.IDE;
	TxMessage.ExtId = Bootloader_data.ExtId.all;
//	can_tx_msg.MBox_num = 12;
	//CAN_Tx_Msg (&can_tx_msg);
	while (CAN1_CanRxMsgFlag != 1)
	{
		time_out++;
		IAP_delay(500);
		if (time_out % 30000 == 0)
		{
			//CAN_Tx_Msg(&can_tx_msg);
		}
		//	time_out %= 65000;
		if (time_out % 0xFFFF == 0)
		{
			return 0x00;
		}
	}
	//can_rx_msg.upflag = 0;
	if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 ) | cmd_list.CmdSuccess ))
	{

		return 12;   //�ӻ�Ӧ�����
	}
//	if (/*can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte4 == 0xAA*/)
	if (CAN1_RxMessage.Data[4] == 0xAA)
	{
		for (i = 4; i < 8; i++)
		{
			if (CAN1_RxMessage.Data[i] == 0xAA)
			{
				CAN_BL_temp = CAN_BL_APP;
			}
			else
			{
				CAN_BL_temp = 0x00;
			}
		}
	}
//	else if (can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte4 == 0x55)
	else if (CAN1_RxMessage.Data[4] == 0x55)
	{
		for (i = 4; i < 8; i++)
		{
			if (CAN1_RxMessage.Data[i] == 0x55)
			{
				CAN_BL_temp = CAN_BL_BOOT;
			}
			else
			{
				CAN_BL_temp = 0x00;
			}
		}
	}
	//�����APP������������,����Ҫ����һ����תָ��,��оƬ��λ
	//���ó�����ת��Boot
	if (CAN_BL_temp == CAN_BL_APP)
	{
		Bootloader_data.DLC = 4;
		Bootloader_data.ExtId.bit.cmd = cmd_list.Excute;
		Bootloader_data.ExtId.bit.addr = 0x123;
		Bootloader_data.ExtId.bit.reserve = 0;
		Bootloader_data.IDE = CAN_ID_EXT;
		TxMessage.DLC = Bootloader_data.DLC;
		TxMessage.IDE = Bootloader_data.IDE;
	//	can_tx_msg.ExtId.all = Bootloader_data.ExtId.all;
		TxMessage.ExtId = Bootloader_data.ExtId.all;
	//	can_tx_msg.MBox_num = 12;
		for (i = 0; i < TxMessage.DLC; i++)
		{
		//	can_tx_msg.CAN_Tx_msg_data.msg_byte.data[i] = 0x55;
			TxMessage.Data[i] = CAN_BL_BOOT&0xFF;
		}
	//	CAN_Tx_Msg(&can_tx_msg);
		CAN_WriteData(&TxMessage);
		//��ת֮��ע���ӳ�ʱ�䣬���ԼӴ�һЩ,���ӳ�ʱ����Ҫ����ΪоƬ��λʱ�����
		//500ms->250ms->100ms->50ms,���ѡ��50ms����;
		delay_ms(50);
		//--��ѯ��ǰ���еĳ�����APP����BOOT;
		Bootloader_data.DLC = 0;
		Bootloader_data.ExtId.bit.cmd = cmd_list.Check;
		Bootloader_data.ExtId.bit.addr = 0x123;
		Bootloader_data.ExtId.bit.reserve = 0;
		Bootloader_data.IDE = CAN_ID_EXT;
		TxMessage.DLC = Bootloader_data.DLC;
		TxMessage.IDE = Bootloader_data.IDE;
		//can_tx_msg.ExtId.all = Bootloader_data.ExtId.all;
		TxMessage.ExtId = Bootloader_data.ExtId.all;
	//	can_tx_msg.MBox_num = 12;
	//	CAN_Tx_Msg(&can_tx_msg);
		CAN_WriteData(&TxMessage);
		while (CAN1_CanRxMsgFlag != 1)
		{
			CAN_WriteData(&TxMessage);
			delay_us(1000);
		}
		//can_rx_msg.upflag = 0;
		CAN1_CanRxMsgFlag = 0;
		if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 )|cmd_list.CmdSuccess ))
		{

			return 12;		//���ش���
		}

	}
	//�����ļ�ϵͳ
	res = f_mount(&IAP_fatfs, "0:", 1);
	if (res != FR_OK)
	{
		return res;
	}
	//���ļ�app.bin
	res = f_open(&fp, "0:/update/app.bin", FA_READ);
	if (res != FR_OK)
	{
		return res;
	}
	//�ƶ��ļ�ָ�뵽�ļ�ͷ
	f_lseek(&fp, 0);
	if (res != FR_OK)
	{
		f_close(&fp);
		return res;
	}
	//��ȡ�ļ���С,׼�������ļ�����ָ��
	//�ļ���С����4�ֽ�����ʾ,������Ա�ʾ4Gb
	Bootloader_data.DLC = 4;
	Bootloader_data.ExtId.bit.cmd = cmd_list.Erase;
	Bootloader_data.ExtId.bit.addr = 0x123;
	Bootloader_data.ExtId.bit.reserve = 0;
	Bootloader_data.IDE = CAN_ID_EXT;
	Bootloader_data.data[0] = ( fp.obj.objsize & 0xFF000000 ) >> 24;
	Bootloader_data.data[1] = ( fp.obj.objsize & 0xFF0000 ) >> 16;
	Bootloader_data.data[2] = ( fp.obj.objsize & 0xFF00 ) >> 8;
	Bootloader_data.data[3] = ( fp.obj.objsize & 0x00FF );
	TxMessage.DLC = Bootloader_data.DLC;
	TxMessage.IDE = Bootloader_data.IDE;
//	can_tx_msg.ExtId.all = Bootloader_data.ExtId.all;
	TxMessage.ExtId = Bootloader_data.ExtId.all;
//	can_tx_msg.MBox_num = 12;
	for (i = 0; i < Bootloader_data.DLC; i++)
	{
		TxMessage.Data[i] = Bootloader_data.data[i];
	}
	CAN_WriteData(&TxMessage);
	while (CAN1_CanRxMsgFlag != 1)
	{
		;
	}
	CAN1_CanRxMsgFlag = 0;
	if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 ) | cmd_list.CmdSuccess ))
	{
		f_close(&fp);
		return 1;
	}
	//-----������,׼����ȡ����,������ȡ�����ݷ��͸��Է�
	/********************************************
	 * ����ǰ�ķ���һ��,ÿ�ζ���ȡ64�ֽ�,�Ա���STM32д������;
	 * �ڷ������ݰ�֮ǰ��Ҫ���͵�ǰ���ݰ��ĳ��ȼ�����Ҫд���ƫ�Ƶ�ַ
	 * */
	while (file_size < f_size(&fp))
	{
		//��ȡ����
		res = f_read(&fp, read_temp, 64, &read_cnt);
		if (res != FR_OK)
		{
			return res;
		}
		//����crc����,�����и�ֵ
		crc_value = CRCcalc16(read_temp, read_cnt);
		read_temp[read_cnt] = crc_value & 0xFF;
		read_temp[read_cnt + 1] = ( crc_value >> 8 ) & 0xFF;
		//���͵�ǰ���ݰ��������Ϣ,���ݰ������ݳ���,���ݰ���ƫ�Ƶ�ַ
		//����8�ֽ�,0-3�ֽڱ�ʾ��ַƫ��
		//4-7��ʾ���ݰ��Ĵ�С
		Bootloader_data.DLC = 8;
		Bootloader_data.ExtId.bit.cmd = cmd_list.WriteInfo;
		Bootloader_data.ExtId.bit.addr = 0x123;
		Bootloader_data.ExtId.bit.reserve = 0;
		Bootloader_data.IDE = CAN_ID_EXT;
		Bootloader_data.data[0] = ( file_size & 0xFF000000 ) >> 24;
		Bootloader_data.data[1] = ( file_size & 0xFF0000 ) >> 16;
		Bootloader_data.data[2] = ( file_size & 0xFF00 ) >> 8;
		Bootloader_data.data[3] = ( file_size & 0x00FF );

		Bootloader_data.data[4] = ( ( read_cnt + 2 ) & 0xFF000000 ) >> 24;
		Bootloader_data.data[5] = ( ( read_cnt + 2 ) & 0xFF0000 ) >> 16;
		Bootloader_data.data[6] = ( ( read_cnt + 2 ) & 0xFF00 ) >> 8;
		Bootloader_data.data[7] = ( ( read_cnt + 2 ) & 0x00FF );
		TxMessage.DLC = Bootloader_data.DLC;
		TxMessage.IDE = Bootloader_data.IDE;
		TxMessage.ExtId  = Bootloader_data.ExtId.all;
	//	can_tx_msg.MBox_num = 12;
		for (i = 0; i < Bootloader_data.DLC; i++)
		{
			 TxMessage.Data[i] = Bootloader_data.data[i];
		}
		CAN_WriteData(&TxMessage);
		while (CAN1_CanRxMsgFlag != 1)
		{
			;
		}
		CAN1_CanRxMsgFlag = 0;
		if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 )| cmd_list.CmdSuccess ))
		{
			f_close(&fp);
			return 2;
		}
		//--�������ݰ�
		//�˴�Ϊ�μ�2,��Ϊ�����λ��CRC�ļ���ֵ
		while (cnt < read_cnt + 2)
		{
			int temp;
			temp = read_cnt + 2 - cnt;
			if (temp >= 8)
			{
				Bootloader_data.DLC = 8;
				Bootloader_data.ExtId.bit.cmd = cmd_list.Write;
				Bootloader_data.ExtId.bit.addr = 0x123;
				Bootloader_data.ExtId.bit.reserve = 0;
				Bootloader_data.IDE = CAN_ID_EXT;
				//--------------------------------------------------------
				TxMessage.DLC = Bootloader_data.DLC;
				TxMessage.IDE = Bootloader_data.IDE;
				TxMessage.ExtId = Bootloader_data.ExtId.all;
			//	can_tx_msg.MBox_num = 12;
				for (i = 0; i < Bootloader_data.DLC; i++)
				{
					TxMessage.Data[i] = read_temp[cnt];
					cnt++;
				}
			}
			else
			{
				Bootloader_data.DLC = temp;
				Bootloader_data.ExtId.bit.cmd = cmd_list.Write;
				Bootloader_data.ExtId.bit.addr = 0x123;
				Bootloader_data.ExtId.bit.reserve = 0;
				Bootloader_data.IDE = CAN_ID_EXT;
				//--------------------------------------------------------
				TxMessage.DLC = Bootloader_data.DLC;
				TxMessage.IDE = Bootloader_data.IDE;
				TxMessage.ExtId = Bootloader_data.ExtId.all;
				//can_tx_msg.MBox_num = 12;
				for (i = 0; i < Bootloader_data.DLC; i++)
				{
					TxMessage.Data[i] = read_temp[cnt];
					cnt++;
				}
			}
		CAN_WriteData(&TxMessage);
		}
		//�������һ�����ݵȴ���Ӧ
		while (CAN1_CanRxMsgFlag != 1)
		{
			;
		}
		CAN1_CanRxMsgFlag = 0;
		//��������������ȫ������
		for (i = 0; i < read_cnt + 2; i++)
		{
			read_temp[cnt] = 0x00;
		}
		if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 ) | cmd_list.CmdSuccess ))
		{
			f_close(&fp);
			return 3;
		}
		file_size = file_size + read_cnt;
		//�˴�һ���ǵý�cnt�������ڶ��η�������ʱ��������
		cnt = 0;
	}
	//--�������ת����
	//��תָ�����ó����Boot��ת��APP��,ִ��APP����
	Bootloader_data.DLC = 4;
	Bootloader_data.ExtId.bit.cmd = cmd_list.Excute;
	Bootloader_data.ExtId.bit.addr = 0x123;
	Bootloader_data.ExtId.bit.reserve = 0;
	Bootloader_data.IDE = CAN_ID_EXT;
	TxMessage.DLC = Bootloader_data.DLC;
	TxMessage.IDE = Bootloader_data.IDE;
	TxMessage.ExtId = Bootloader_data.ExtId.all;
//	can_tx_msg.MBox_num = 12;
	for (i = 0; i < Bootloader_data.DLC; i++)
	{
		TxMessage.Data[i] = 0xAA;
	}
	CAN_WriteData(&TxMessage);
CAN_WriteData(&TxMessage);
	return 0xFF;
}
//-----------------------------------------------------------------------------------
//���´����Ϊ��hex�ļ�������Ҫ�Ĵ���
void Data_clear(unsigned char *data,unsigned long int len)
{
     unsigned long int i;
     for(i = 0;i < len;i++)
     {
         *data = 0;
         data++;
     }
}
unsigned char convertion(unsigned char *hex_data)
{
    unsigned char bin_data = 0xFF;
    if(*hex_data >= '0'&&*hex_data <= '9')
    {

        bin_data = *hex_data-NUM_OFFSET;
    }
    else if(*hex_data >= 'A'&&*hex_data <= 'F')
    {
        bin_data = *hex_data-CHAR_OFFSET;
    }
    return bin_data;
}
void hex_to_bin(unsigned char *hex_src,unsigned char *bin_dst,unsigned char len)
{
    unsigned char i;
    for(i = 0;i <len;i++)
    {
        *bin_dst = convertion(hex_src);
        bin_dst++;
        hex_src++;
    }
} 
/**********************************************
*������:u8 CAN_IAP_Hex(u16 DEVICE_ADDR);
*�������:u16 DEVICE_ADDR �豸��ַ;
*����ֵ:
*��ע:�ú����Ƕ�hex�ļ����н���
************************************************/
u8 CAN_IAP_Hex(u16 DEVICE_ADDR)
{
  
	int i = 0;   //��������
	CanTxMsg TxMessage; 

	FRESULT res;
	unsigned char status = 0xFF;
	Bootloader_data.ExtId.bit.reserve = 0x0000; 
	Bootloader_data.IDE = CAN_ID_EXT;
	Bootloader_data.ExtId.bit.addr = DEVICE_ADDR;
	TxMessage.ExtId = 0x1FFFFFFF;
	unsigned short int time_out = 0x00;
	for (i = 0; i < 8; i++)
	{ 
		TxMessage.Data[i] = 0;
	}
	unsigned long int CAN_BL_temp;   //��ǰ�豸���еĹ̼�����
	//--��ѯ��ǰ���еĳ�����APP����BOOT;
	Bootloader_data.DLC = 0;
	Bootloader_data.ExtId.bit.cmd = cmd_list.Check;
	TxMessage.DLC = Bootloader_data.DLC;
	TxMessage.IDE = Bootloader_data.IDE;
	TxMessage.ExtId &= Bootloader_data.ExtId.all; 
	CAN_WriteData(&TxMessage);
	while (CAN1_CanRxMsgFlag != 1)
	{
		time_out++; 
		if(time_out%200 == 0)
		{
			CAN_WriteData(&TxMessage);
		}	
		if (time_out % 50000 == 0)
		{
			return 0x01;
		}
		delay_ms(800);
	} 
	time_out = 0;
	CAN1_CanRxMsgFlag = 0;
	if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 ) | cmd_list.CmdSuccess ))
	{

		return 0x02;   //�ӻ�Ӧ�����
	} 
	if (CAN1_RxMessage.Data[4] == 0xAA)
	{
		for (i = 4; i < 8; i++)
		{
			if (CAN1_RxMessage.Data[i] == 0xAA)
			{
				CAN_BL_temp = CAN_BL_APP;
			}
			else
			{
				CAN_BL_temp = 0x00;
				return 0xFA;
			}
		}
	} 
	else if (CAN1_RxMessage.Data[4] == 0x55)
	{
		for (i = 4; i < 8; i++)
		{
			if (CAN1_RxMessage.Data[i] == 0x55)
			{
				CAN_BL_temp = CAN_BL_BOOT;
			}
			else
			{
				CAN_BL_temp = 0x00;
				return 0xFB;
			}
		}
	}
	else
	{
			return 0xFC;
	}
	//CAN_BL_temp = CAN_BL_BOOT;
	//�����APP������������,����Ҫ����һ����תָ��,��оƬ��λ
	//���ó�����ת��Boot
	if (CAN_BL_temp == CAN_BL_APP)
	{
		Bootloader_data.DLC = 4;
		Bootloader_data.ExtId.bit.cmd = cmd_list.Excute;
		TxMessage.DLC = Bootloader_data.DLC;
		TxMessage.IDE = Bootloader_data.IDE; 
		TxMessage.ExtId = Bootloader_data.ExtId.all; 
		for (i = 0; i < TxMessage.DLC; i++)
		{ 
			TxMessage.Data[i] = CAN_BL_BOOT&0xFF;
		} 
		CAN_WriteData(&TxMessage);//�������޷�����Ϣ
		//��ת֮��ע���ӳ�ʱ�䣬���ԼӴ�һЩ,���ӳ�ʱ����Ҫ����ΪоƬ��λʱ�����
		//500ms->250ms->100ms->50ms,���ѡ��50ms����;
		delay_ms(500);
		//--��ѯ��ǰ���еĳ�����APP����BOOT;
		Bootloader_data.DLC = 0;
		Bootloader_data.ExtId.bit.cmd = cmd_list.Check;
		TxMessage.DLC = Bootloader_data.DLC;
		TxMessage.IDE = Bootloader_data.IDE; 
		TxMessage.ExtId = Bootloader_data.ExtId.all; 
		CAN_WriteData(&TxMessage);
		while (CAN1_CanRxMsgFlag != 1)
		{
			time_out++; 
			if(time_out%200 == 0)
			{
				CAN_WriteData(&TxMessage);
			}	
			if (time_out % 50000 == 0)
			{
				time_out = 0;
				return 0x15;
			}
			delay_ms(800);
		} 
		if (CAN1_RxMessage.Data[4] == 0x55)
		{
			for (i = 4; i < 8; i++)
			{
				if (CAN1_RxMessage.Data[i] == 0x55)
				{
					CAN_BL_temp = CAN_BL_BOOT;
				}
				else
				{
					CAN_BL_temp = 0x00;
					return 0xFE;
				}
			}
		}
		else
		{
			return 0xFD;
		}
		time_out = 0;
		CAN1_CanRxMsgFlag = 0;//����������ݱ�־λ
		if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 )|cmd_list.CmdSuccess ))
		{

			return 0x03;		//���ش���
		}
		

	}
	if(CAN_BL_temp == CAN_BL_BOOT)//���Ͳ���ָ��
	{
		Bootloader_data.DLC = 0;
		Bootloader_data.ExtId.bit.cmd =cmd_list.WriteInfo;
		TxMessage.DLC = Bootloader_data.DLC;
		TxMessage.IDE = Bootloader_data.IDE; 
		TxMessage.ExtId = 0x00000000;
		TxMessage.ExtId |= DEVICE_ADDR<<4|0x0F; 
		CAN_WriteData(&TxMessage);
		while (CAN1_CanRxMsgFlag != 1)
		{
			time_out++; 
			if (time_out % 10000 == 0)
			{
				time_out = 0;
				return 0x04;
			}
			delay_ms(800);
		} 
		time_out = 0;
		CAN1_CanRxMsgFlag = 0;//����������ݱ�־λ
		if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 )|cmd_list.CmdSuccess ))
		{ 
			return 0x05;		//���ش���
		}
	}
	//�ڶ���д�����ݵ�DSP
	//�����ļ�ϵͳ
	res = f_mount(&IAP_fatfs, "0:", 1);
	if (res != FR_OK)
	{
		return 0x06;
	}
	res = f_open(&fp_hex,"0:/bootloader_file/TMS320F28335/hex/28335__APP.hex",FA_READ);
	if(res != FR_OK)
	{ 
		return 0x07;//��28335SCI_Update_Flash.hex�ļ�ʧ��,
	}
	res  = f_lseek(&fp_hex,0);
	if(res != FR_OK)
	{ 
		return 0x08; 
	}
	while(hex_size < f_size(&fp_hex))
	{
		res =  f_read(&fp_hex,read_buf,9,&read_cnt);//�ȶ�ȡ9���ֽ�,
		if(res != FR_OK)
		{ 
			return 0x09; 
		}
		if(read_buf[0] == ':')//��ʾ����ʼ��־,�жϸղŶ�ȡ�������еĵ�һ���ֽ��Ƿ�����ʼ��־
		{	
			hex_to_bin(&read_buf[1],bin_buf,8);//����ȡ��9���ֽں���8�ֽ���ASC_IIת��Ϊhex(16��������)
			pack_info.data_type = bin_buf[6]<<4|bin_buf[7]; 
			if(send_data.read_start_flag == 0)//����ñ�־λΪ0,��ʾ���ǵ�һ�ζ�ȡ����,��ʱ����־λ��һ
			{
				send_data.read_start_flag = 1;
				send_data.send_state = 0;
				send_data.line_num = 2;
				send_data.line_cnt = 0;
				send_data.data_cnt = 0;
				send_data.data_len = 0;
			}
			else
			{
				if(pack_info.data_type == DATA_BASE_ADDR||pack_info.data_type == DATA_END)//�жϸ��е�������,����Ǳ�ʾ����ַ
				{
					status = CAN_Send_file_data(&send_data,DEVICE_ADDR);
					if(status != 0xAF)
					{
						return status;
					}
					status = 0xFF;
					send_data.data_len = 0;
					send_data.data_cnt = 0;
					send_data.data_addr = 0x00;
					send_data.line_cnt = 0;
					for(i = 0;i < 68;i++)
					{
						send_data.data[0] = 0x00;
					} 
				}
				else if(send_data.line_cnt == send_data.line_num)//����ָ���������������ݷ���
				{ 
					status = CAN_Send_file_data(&send_data,DEVICE_ADDR);
					if(status != 0xAF)
					{
						return status;
					}
					status = 0xFF; 
					send_data.data_len = 0;
					send_data.data_cnt = 0;
					send_data.data_addr = 0x00;
					send_data.line_cnt = 0;
					for(i = 0;i < 68;i++)
					{
						send_data.data[0] = 0x00;
					} 
				}
				else
				{
					
				}
			}
			pack_info.data_len = bin_buf[0]<<4|bin_buf[1];
			if(pack_info.data_type == DATA_Rrecord)//�жϸ��е�������,����Ǳ�ʾ����ַ
			{
				pack_info.data_addr_offset = bin_buf[2]<<12|bin_buf[3]<<8|bin_buf[4]<<4|bin_buf[5];
			}
			else
			{
				pack_info.data_addr_offset = 0x0000;
			}
		
			Data_clear(read_buf,128);  
			res = f_read(&fp_hex,read_buf,(pack_info.data_len*2+3),&read_cnt);//�ڶ��ζ�ȡ�ļ�,��ʱ��ȡ��������ʵ������
			if(res != FR_OK)
			{ 
				return 0x0A; 
			}
			hex_to_bin(&read_buf[0],bin_buf,pack_info.data_len*2);//����ȡ������ת��Ϊhex;
			if(pack_info.data_type == DATA_BASE_ADDR)
			{ 
				pack_info.data_base_addr =bin_buf[0]<<12|bin_buf[1]<<8|bin_buf[2]<<4|bin_buf[3];
				pack_info.data_base_addr = pack_info.data_base_addr<<16; 
			}
			 else if(pack_info.data_type == 0x00)
			{
				pack_info.data_addr = pack_info.data_base_addr+pack_info.data_addr_offset;//��ʾ��������Ӧ��д�����ʵ��ַ
				for(i = 0;i <pack_info.data_len;i++)
				{
				   pack_info.Data[i] = bin_buf[i*2]<<4|bin_buf[2*i+1];
				}
			} 
			//��������ӵĴ���,����hex�ļ�������ش���
			//���ڵ��������߼�˳����û�������
			if(pack_info.data_type == DATA_Rrecord)
			{
				if(send_data.line_cnt == 0)//�����������Ϊ0,��ʾ���ǵ�һ�ζ�ȡ,�����Ҫ����д�����ݵĵ�ַ
				{
					send_data.data_addr = pack_info.data_addr;//����һ�е����ݵ�ַ��Ϊ�����ݰ���д���ַ 		
				}
				//�����ǽ��ղŶ�ȡ������д��send_data.data������
				for(i = 0;i < pack_info.data_len;i++)
				{
					send_data.data_cnt = i;
					send_data.data[i+send_data.data_len] = pack_info.Data[i];			
				}
				send_data.data_cnt = pack_info.data_len;
				send_data.data_len = send_data.data_len+send_data.data_cnt;
				send_data.line_cnt++;
			}
			Data_clear(read_buf,128);
			Data_clear(bin_buf,128); 
			Data_clear(&pack_info.Data[0],64);
			res  = f_lseek(&fp_hex,(f_tell(&fp_hex)+1));
			if(res != FR_OK)
			{ 
				return 0x0B; 
			}
		}
		hex_size =hex_size+(pack_info.data_len*2+12);
	}
	f_close(&fp_hex);
	Bootloader_data.DLC = 4;
	Bootloader_data.ExtId.bit.cmd = cmd_list.Excute;
	TxMessage.DLC = Bootloader_data.DLC;
	TxMessage.IDE = Bootloader_data.IDE; 
	TxMessage.ExtId = Bootloader_data.ExtId.all; 
	TxMessage.Data[0] = (CAN_BL_APP>>24)&0xFF;
	TxMessage.Data[1] = (CAN_BL_APP>>16)&0xFF;
	TxMessage.Data[2] = (CAN_BL_APP>>8)&0xFF;
	TxMessage.Data[3] = (CAN_BL_APP>>0)&0xFF;
	CAN_WriteData(&TxMessage);

	return 0x0FF;
}
u8 CAN_Send_file_data(SEND_INFO *send_data,u16 DEVICE_ADDR)
{
	unsigned char i;
	unsigned char cnt = 0;
	UINT crc_temp = 0x0000;
	bootloader_data bootloader_data;
	//bootloader_data���ֳ�Ա���и�ֵ,����ĳ��������ע
	bootloader_data.ExtId.bit.addr = DEVICE_ADDR;
	bootloader_data.ExtId.bit.reserve = 0;
	bootloader_data.IDE = CAN_ID_EXT;
	CanTxMsg TxMessage;
	//����crc����,���ҽ��и�ֵ
	crc_temp = CRCcalc16(&send_data->data[0],send_data->data_len);
	//��crc���������и�ֵ;
	send_data->data[send_data->data_len] = crc_temp&0xFF;
	send_data->data[send_data->data_len+1] = (crc_temp>>8)&0xFF;
	//׼����������
	//����DSP д�����ݵĵ�ַ 
	//���͵�ǰ���ݰ��������Ϣ,���ݰ������ݳ���,���ݰ���ƫ�Ƶ�ַ
	//����8�ֽ�:
	//0-3�ֽڱ�ʾ��ַƫ�� 4-7��ʾ���ݰ��Ĵ�С
	bootloader_data.DLC = 8;
	bootloader_data.ExtId.bit.cmd = cmd_list.WriteInfo;
	Bootloader_data.data[0] = ( send_data->data_addr & 0xFF000000 ) >> 24;
	bootloader_data.data[1] = ( send_data->data_addr & 0xFF0000 ) >> 16;
	bootloader_data.data[2] = ( send_data->data_addr & 0xFF00 ) >> 8;
	bootloader_data.data[3] = ( send_data->data_addr & 0x00FF );

	bootloader_data.data[4] = ( ( send_data->data_len + 2 ) & 0xFF000000 ) >> 24;
	bootloader_data.data[5] = ( ( send_data->data_len + 2 ) & 0xFF0000 ) >> 16;
	bootloader_data.data[6] = ( ( send_data->data_len + 2 ) & 0xFF00 ) >> 8;
	bootloader_data.data[7] = ( ( send_data->data_len + 2 ) & 0x00FF );
	TxMessage.DLC = bootloader_data.DLC;
	TxMessage.IDE = bootloader_data.IDE;
	TxMessage.ExtId  = bootloader_data.ExtId.all;
	for (i = 0; i < bootloader_data.DLC; i++)
	{
		 TxMessage.Data[i] = bootloader_data.data[i];
	}
	CAN_WriteData(&TxMessage);
	while (CAN1_CanRxMsgFlag != 1)
	{
		;
	}
	CAN1_CanRxMsgFlag = 0;
	if (CAN1_RxMessage.ExtId != ( ( bootloader_data.ExtId.all & 0xFFF0 )| cmd_list.CmdSuccess ))
	{
		 
		return 0xA0;
	}
	//��������
	while(cnt < send_data->data_len+2)
	{
		int temp;
		temp = send_data->data_len + 2 - cnt;
		if (temp >= 8)
		{
			bootloader_data.DLC = 8;
			bootloader_data.ExtId.bit.cmd = cmd_list.Write; 
			//--------------------------------------------------------
			TxMessage.DLC = bootloader_data.DLC;
			TxMessage.IDE = bootloader_data.IDE;
			TxMessage.ExtId = bootloader_data.ExtId.all; 
			for (i = 0; i < bootloader_data.DLC; i++)
			{
				TxMessage.Data[i] = send_data->data[cnt];
				cnt++;
			}
		}
		else
		{
			bootloader_data.DLC = temp;
			bootloader_data.ExtId.bit.cmd = cmd_list.Write; 
			//--------------------------------------------------------
			TxMessage.DLC = bootloader_data.DLC;
			TxMessage.IDE = bootloader_data.IDE;
			TxMessage.ExtId = bootloader_data.ExtId.all; 
			for (i = 0; i < bootloader_data.DLC; i++)
			{
				TxMessage.Data[i] = send_data->data[cnt];
				cnt++;
			}
		}
		CAN_WriteData(&TxMessage);
	}
	//�����������,�ȴ���Ӧ
	while (CAN1_CanRxMsgFlag != 1)
	{
		;
	}
	CAN1_CanRxMsgFlag = 0;
	//����������׼����һ�����ݷ��͹���
	for (i = 0; i < send_data->data_len + 2; i++)
	{
		send_data->data[cnt] = 0x00;
	}
	if (CAN1_RxMessage.ExtId != ( ( bootloader_data.ExtId.all & 0xFFF0 ) | cmd_list.CmdSuccess ))
	{ 
		return 0xA1;
	}
	cnt = 0;
	return 0xAF;
}
