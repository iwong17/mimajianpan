#ifndef _HEAD_H_	
#define _HEAD_H_

#include <STC89C51RC_RD_PLUS.H>
#include <string.h>

#define uchar unsigned char 
#define uint  unsigned int
#define BOOL  bit
//#define NULL (void *)0

typedef union 
{
	unsigned char		data_byte;
	struct
	{
		unsigned char	bit_1:1;//ตอฮป
		unsigned char 	bit_2:1;
		unsigned char 	bit_3:1;
		unsigned char	bit_4:1;
		unsigned char 	bit_5:1;
		unsigned char	bit_6:1;
		unsigned char 	bit_7:1;
		unsigned char 	bit_8:1;
	}data_bit;		
}FLAG;

//sbit test=P2^7;



#endif