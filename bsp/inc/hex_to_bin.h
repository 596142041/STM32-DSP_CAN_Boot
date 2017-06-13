/*
 * hex_to_bin.h
 *
 *  Created on: 2017?1?17?
 *      Author: lpr
 */

#ifndef HEX_TO_BIN_INC_HEX_TO_BIN_H_
#define HEX_TO_BIN_INC_HEX_TO_BIN_H_
#define USE_DEBUG 1
#include "user_config.h"
#include "ff.h"
//#include "printf.h"

#define NUM_OFFSET   48
#define CHAR_OFFSET  55
typedef struct
{
    unsigned char data_len;
    unsigned long int data_addr;
		unsigned long int data_base_addr;
    unsigned int data_addr_offset;
    unsigned char data_type;
    unsigned char Data[64];
}PACK_INFO;
void Data_clear(unsigned char *data,unsigned long int len);
unsigned char convertion(unsigned char *hex_data);
void hex_to_bin(unsigned char *hex_src,unsigned char *bin_dst,unsigned char len);
u8 Hex_to_Bin(void);
u8 Bin_test(void);
extern PACK_INFO pack_info;
extern unsigned char read_buf[128];
extern unsigned char bin_buf[128];

#endif /* HEX_TO_BIN_INC_HEX_TO_BIN_H_ */
