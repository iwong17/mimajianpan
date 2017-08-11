#ifndef _MCU_H_
#define _MCU_H_

#include <STC89C51RC_RD_PLUS.H>

sbit  MCLK  = P3^2;	   //����˫��Ƭ��ͨ����
sbit  SCLK  = P3^3;	   //����˫��Ƭ��ͨ����
sbit  SDATA = P3^4;	   //����˫��Ƭ��ͨ��������

void Putbytespi(unsigned char ch);
unsigned char Getbytespi(void);

#endif