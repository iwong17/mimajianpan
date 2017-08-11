#ifndef _MCU_H_
#define _MCU_H_

#include <STC89C51RC_RD_PLUS.H>

sbit  MCLK  = P3^2;	   //定义双单片机通信线
sbit  SCLK  = P3^3;	   //定义双单片机通信线
sbit  SDATA = P3^4;	   //定义双单片机通信数据线

void Putbytespi(unsigned char ch);
unsigned char Getbytespi(void);

#endif