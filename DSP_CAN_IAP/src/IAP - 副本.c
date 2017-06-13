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
//int send_cnt; //表示CAN总线发送64字节次数; 
/*
目前计划是每次读取2行发送一次数据,在DSP中2行正常应该是64字节,而在STM32中2行刚好是32字节
*/
//---------------------------
CanRxMsg CAN1_RxMessage;
volatile uint8_t CAN1_CanRxMsgFlag=0;
Boot_CMD_LIST cmd_list =
{
	.SetBaudRate = 0x04,      //擦除APP区域数据
	.WriteInfo = 0x01,//设置多字节写数据相关参数（写起始地址，数据量）
	.Write = 0x02,//以多字节形式写数据
	.Check = 0x03,//检测节点是否在线，同时返回固件信息
	.Erase = 0x00,//设置节点波特率
	.Excute = 0x05,//执行固件
	.CmdSuccess = 0x08,//命令执行成功
	.CmdFaild = 0x09,//命令执行失败
	
};
void IAP_delay(unsigned short int n)
{
	while (n)
	{
		n--;
	}
}
/*******************************************
 *函数名:unsigned short int CRCcalc16 (unsigned char *data,unsigned short int len)
 *输入参数:unsigned char *data:CRC校验数据,unsigned short int len:CRC校验长度
 *返回值:unsigned short int crc_res:CRC校验结果,高位在前
 *备注:该CRC校验采用的是Modbus的CRC校验方式;
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
 *函数名:u8 File_send(void);
 *输入参数:无;
 *返回值:u8 res:成功返回0;
 *
 ********************************************/

u8 CAN_IAP_Bin(void)
{
	int i = 0;   //计数变量
	UINT crc_value = 0x0000;
	CanTxMsg TxMessage;
	unsigned char cnt = 0;
	unsigned long int CAN_BL_temp;   //当前设备运行的固件缓存
	FRESULT res;
	Bootloader_data.ExtId.bit.reserve = 0;
	unsigned short int time_out = 0x00;
	for (i = 0; i < 8; i++)
	{
	//	can_tx_msg.CAN_Tx_msg_data.msg_byte.data[i] = 0;
		//can_rx_msg.CAN_Rx_msg_data.msg_byte.data[i] = 0;
		TxMessage.Data[i] = 0;
	}
	//--查询当前运行的程序是APP还是BOOT;
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

		return 12;   //从机应答错误
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
	//如果是APP程序正在运行,则需要发送一次跳转指令,让芯片复位
	//则让程序跳转到Boot
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
		//跳转之后注意延迟时间，可以加大一些,这延迟时间主要是因为芯片复位时间决定
		//500ms->250ms->100ms->50ms,最后选择50ms即可;
		delay_ms(50);
		//--查询当前运行的程序是APP还是BOOT;
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

			return 12;		//返回错误
		}

	}
	//挂载文件系统
	res = f_mount(&IAP_fatfs, "0:", 1);
	if (res != FR_OK)
	{
		return res;
	}
	//打开文件app.bin
	res = f_open(&fp, "0:/update/app.bin", FA_READ);
	if (res != FR_OK)
	{
		return res;
	}
	//移动文件指针到文件头
	f_lseek(&fp, 0);
	if (res != FR_OK)
	{
		f_close(&fp);
		return res;
	}
	//获取文件大小,准备发送文件擦除指令
	//文件大小采用4字节来表示,最带可以表示4Gb
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
	//-----第三步,准备读取数据,并将读取的数据发送给对方
	/********************************************
	 * 和以前的方案一样,每次都读取64字节,以便于STM32写入数据;
	 * 在发送数据包之前需要发送当前数据包的长度及其需要写入的偏移地址
	 * */
	while (file_size < f_size(&fp))
	{
		//读取数据
		res = f_read(&fp, read_temp, 64, &read_cnt);
		if (res != FR_OK)
		{
			return res;
		}
		//进行crc计算,并进行赋值
		crc_value = CRCcalc16(read_temp, read_cnt);
		read_temp[read_cnt] = crc_value & 0xFF;
		read_temp[read_cnt + 1] = ( crc_value >> 8 ) & 0xFF;
		//发送当前数据包的相关信息,数据包的数据长度,数据包的偏移地址
		//包含8字节,0-3字节表示地址偏移
		//4-7表示数据包的大小
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
		//--发送数据包
		//此处为何加2,因为最后两位是CRC的检验值
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
		//发送完成一包数据等待回应
		while (CAN1_CanRxMsgFlag != 1)
		{
			;
		}
		CAN1_CanRxMsgFlag = 0;
		//将缓存数组数据全部清零
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
		//此处一定记得将cnt清零否则第二次发送数据时会有问题
		cnt = 0;
	}
	//--最后发送跳转命令
	//跳转指令是让程序从Boot跳转至APP区,执行APP程序
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
//以下代码均为对hex文件解码需要的代码
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
*函数名:u8 CAN_IAP_Hex(u16 DEVICE_ADDR);
*输入参数:u16 DEVICE_ADDR 设备地址;
*返回值:
*备注:该函数是对hex文件进行解码
************************************************/
u8 CAN_IAP_Hex(u16 DEVICE_ADDR)
{
  
	int i = 0;   //计数变量
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
	unsigned long int CAN_BL_temp;   //当前设备运行的固件缓存
	//--查询当前运行的程序是APP还是BOOT;
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

		return 0x02;   //从机应答错误
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
	//如果是APP程序正在运行,则需要发送一次跳转指令,让芯片复位
	//则让程序跳转到Boot
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
		CAN_WriteData(&TxMessage);//该命令无返回信息
		//跳转之后注意延迟时间，可以加大一些,这延迟时间主要是因为芯片复位时间决定
		//500ms->250ms->100ms->50ms,最后选择50ms即可;
		delay_ms(500);
		//--查询当前运行的程序是APP还是BOOT;
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
		CAN1_CanRxMsgFlag = 0;//清除接收数据标志位
		if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 )|cmd_list.CmdSuccess ))
		{

			return 0x03;		//返回错误
		}
		

	}
	if(CAN_BL_temp == CAN_BL_BOOT)//则发送擦除指令
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
		CAN1_CanRxMsgFlag = 0;//清除接收数据标志位
		if (CAN1_RxMessage.ExtId != ( ( Bootloader_data.ExtId.all & 0xFFF0 )|cmd_list.CmdSuccess ))
		{ 
			return 0x05;		//返回错误
		}
	}
	//第二步写入数据到DSP
	//挂载文件系统
	res = f_mount(&IAP_fatfs, "0:", 1);
	if (res != FR_OK)
	{
		return 0x06;
	}
	res = f_open(&fp_hex,"0:/bootloader_file/TMS320F28335/hex/28335__APP.hex",FA_READ);
	if(res != FR_OK)
	{ 
		return 0x07;//打开28335SCI_Update_Flash.hex文件失败,
	}
	res  = f_lseek(&fp_hex,0);
	if(res != FR_OK)
	{ 
		return 0x08; 
	}
	while(hex_size < f_size(&fp_hex))
	{
		res =  f_read(&fp_hex,read_buf,9,&read_cnt);//先读取9个字节,
		if(res != FR_OK)
		{ 
			return 0x09; 
		}
		if(read_buf[0] == ':')//表示是起始标志,判断刚才读取的数据中的第一个字节是否是起始标志
		{	
			hex_to_bin(&read_buf[1],bin_buf,8);//将读取的9个字节后面8字节由ASC_II转换为hex(16进制数据)
			pack_info.data_type = bin_buf[6]<<4|bin_buf[7]; 
			if(send_data.read_start_flag == 0)//如果该标志位为0,表示这是第一次读取数据,此时将标志位置一
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
				if(pack_info.data_type == DATA_BASE_ADDR||pack_info.data_type == DATA_END)//判断该行的数据是,如果是表示基地址
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
				else if(send_data.line_cnt == send_data.line_num)//到了指定的行数进行数据发送
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
			if(pack_info.data_type == DATA_Rrecord)//判断该行的数据是,如果是表示基地址
			{
				pack_info.data_addr_offset = bin_buf[2]<<12|bin_buf[3]<<8|bin_buf[4]<<4|bin_buf[5];
			}
			else
			{
				pack_info.data_addr_offset = 0x0000;
			}
		
			Data_clear(read_buf,128);  
			res = f_read(&fp_hex,read_buf,(pack_info.data_len*2+3),&read_cnt);//第二次读取文件,此时读取数据是真实的数据
			if(res != FR_OK)
			{ 
				return 0x0A; 
			}
			hex_to_bin(&read_buf[0],bin_buf,pack_info.data_len*2);//将读取的数据转换为hex;
			if(pack_info.data_type == DATA_BASE_ADDR)
			{ 
				pack_info.data_base_addr =bin_buf[0]<<12|bin_buf[1]<<8|bin_buf[2]<<4|bin_buf[3];
				pack_info.data_base_addr = pack_info.data_base_addr<<16; 
			}
			 else if(pack_info.data_type == 0x00)
			{
				pack_info.data_addr = pack_info.data_base_addr+pack_info.data_addr_offset;//表示该行数据应该写入的真实地址
				for(i = 0;i <pack_info.data_len;i++)
				{
				   pack_info.Data[i] = bin_buf[i*2]<<4|bin_buf[2*i+1];
				}
			} 
			//以下是添加的代码,关于hex文件解码相关代码
			//现在的问题是逻辑顺序还是没有理清楚
			if(pack_info.data_type == DATA_Rrecord)
			{
				if(send_data.line_cnt == 0)//如果计数器还为0,表示还是第一次读取,因此需要更新写入数据的地址
				{
					send_data.data_addr = pack_info.data_addr;//将第一行的数据地址作为该数据包的写入地址 		
				}
				//以下是将刚才读取的数据写入send_data.data数组中
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
	//bootloader_data部分成员进行赋值,后面的程序无需关注
	bootloader_data.ExtId.bit.addr = DEVICE_ADDR;
	bootloader_data.ExtId.bit.reserve = 0;
	bootloader_data.IDE = CAN_ID_EXT;
	CanTxMsg TxMessage;
	//进行crc计算,并且进行赋值
	crc_temp = CRCcalc16(&send_data->data[0],send_data->data_len);
	//对crc计算结果进行赋值;
	send_data->data[send_data->data_len] = crc_temp&0xFF;
	send_data->data[send_data->data_len+1] = (crc_temp>>8)&0xFF;
	//准备发送数据
	//计算DSP 写入数据的地址 
	//发送当前数据包的相关信息,数据包的数据长度,数据包的偏移地址
	//包含8字节:
	//0-3字节表示地址偏移 4-7表示数据包的大小
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
	//发送数据
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
	//发送数据完成,等待响应
	while (CAN1_CanRxMsgFlag != 1)
	{
		;
	}
	CAN1_CanRxMsgFlag = 0;
	//将数据清零准备下一次数据发送工作
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
