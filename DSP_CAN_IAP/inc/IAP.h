#ifndef __IAP_H__
#define __IAP_H__
#include "user_config.h"
#include "delay.h"
#include "ff.h"
#include "CANA.h"
//--���º궨���Ŷ�ǵ�ǰ�豸���еĹ̼�����
#define CAN_BL_BOOT     0x55555555  //���е���Boot
#define CAN_BL_APP      0xAAAAAAAA //���е���APP
//--------------------
//��hex�ļ�����ʹ�õ����ݽṹ
#define NUM_OFFSET   48
#define CHAR_OFFSET  55
//---------------
//����hex�ļ��е������Ϣ
/*
��һ���ֽ� ��ʾ�������ݵĳ��ȣ�
�ڶ������ֽڱ�ʾ�������ݵ���ʼ��ַ��
�����ֽڱ�ʾ�������ͣ����������У�0x00��0x01��0x02��0x03��0x04��0x05��
'00' Data Rrecord��������¼���ݣ�HEX�ļ��Ĵ󲿷ּ�¼�������ݼ�¼
'01' End of File Record:������ʶ�ļ������������ļ�����󣬱�ʶHEX�ļ��Ľ�β
'02' Extended Segment Address Record:������ʶ��չ�ε�ַ�ļ�¼
'03' Start Segment Address Record:��ʼ�ε�ַ��¼
'04' Extended Linear Address Record:������ʶ��չ���Ե�ַ�ļ�¼
'05' Start Linear Address Record:��ʼ���Ե�ַ��¼
Ȼ�������ݣ����һ���ֽ� ΪУ��͡�
У��͵��㷨Ϊ������У���ǰ����16��������ۼӺ�(���ƽ�λ)������� = 0x100 - �ۼӺ� 
*/
//���ڵ��ĸ��ֽڵ��������Ͷ���
#define DATA_Rrecord 0x00
#define DATA_END     0x01
#define DATA_BASE_ADDR 0x04
//---------------����DSP����Ӧ����----------------
#define SECTORA   (Uint16)0x0001
#define SECTORB   (Uint16)0x0002
#define SECTORC   (Uint16)0x0004
#define SECTORD   (Uint16)0x0008
#define SECTORE   (Uint16)0x0010
#define SECTORF   (Uint16)0x0020
#define SECTORG   (Uint16)0x0040
#define SECTORH   (Uint16)0x0080
//------------------------------------------------
#define  FW_TYPE_APP  0xAAAAAAAA
#define  FW_TYPE_BOOT 0x55555555
typedef struct _PACK_INFO
{
    unsigned char      data_len;
    unsigned long  int data_addr;
	unsigned long  int data_base_addr;
    unsigned short int data_addr_offset;
    unsigned char      data_type;
    unsigned char      Data[64];
}PACK_INFO;

//----------------------
typedef struct _bootloader_data
{
	union
	{
		u32 all;
		struct
		{
			u16 cmd     :4; //����
			u16 addr    :12; //�豸��ַ
			u16 reserve :16; //����λ
		}bit;
	} ExtId; //��չ֡ID
	unsigned char IDE;   //֡����,��Ϊ��CAN_ID_STD(��׼֡),CAN_ID_EXT(��չ֡)
	unsigned char DLC;  //���ݳ��ȣ���Ϊ0��8;
	u8 data[8];
} bootloader_data;
typedef struct _Boot_CMD_LIST 
{
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
} Boot_CMD_LIST;
typedef struct _SEND_INFO
{
	unsigned char line_num;//��ʾ��ȡ���ݵ�����,���Ϊ2;
	unsigned char line_cnt;//��ʾ��ȡ�ļ��ĺ���,���ֵ���ܴ���line_num
	unsigned char read_start_flag;//��ʾ��ʼ��ȡ���ݱ�־λ 
	unsigned char send_state;//��ʾ�Ƿ���������Ҫ����
	unsigned long int data_addr;
	unsigned char data[68];//
	unsigned char data_cnt;//��ʾ��Ҫ���Ͷ�������,���ֵΪ66
	unsigned char data_len;//��ʾ��Ҫ���͵����ݳ���;
}SEND_INFO;
typedef struct _Device_INFO
{
	union
	{
		unsigned short int all;
		struct
		{
			unsigned short int Device_addr:	12;
			unsigned short int reserve:	4;
		}bits;//�豸��ַ
	}Device_addr;
	unsigned long int FW_TYPE;//�̼�����
	unsigned long int FW_Version;//�̼��汾
}Device_INFO;
static void Data_clear(unsigned char *data,unsigned long int len);
static void hex_to_bin(unsigned char *hex_src,unsigned char *bin_dst,unsigned char len);
static void IAP_delay(unsigned short int n);
static unsigned short int CRCcalc16(unsigned char *data, unsigned short int len);
static unsigned char convertion(unsigned char *hex_data);
static u8 CAN_Send_file_data(SEND_INFO *send_data,u16 DEVICE_ADDR);
u8 CAN_IAP_Bin(u16 DEVICE_ADDR);
u8 CAN_IAP_Hex(u16 DEVICE_ADDR);
extern Boot_CMD_LIST cmd_list;
extern bootloader_data Bootloader_data;
//-----------------------
extern CanRxMsg CAN1_RxMessage;
extern volatile uint8_t CAN1_CanRxMsgFlag;
#endif
