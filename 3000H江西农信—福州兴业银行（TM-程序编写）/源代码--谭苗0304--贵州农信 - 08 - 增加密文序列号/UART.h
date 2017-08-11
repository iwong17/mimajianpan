#ifndef _UART_H_
#define _UART_H_

#include <STC89C51RC_RD_PLUS.H>
#include <string.h>
#include "Delay.h"

//ͷ�ļ��İ���һ��Ҫ��EXTERN֮ǰ
#ifdef _UART_C_
#define EXTERN 
#else
#define EXTERN extern
#endif


sbit Beep=P3^4;
sbit WT_CLK = P3^3;    //����оƬRESET
sbit WT_DI = P1^4;    //����оƬDATA, ��pulse
sbit WT_DO = P1^5;    //����оƬDATA, ��pulse

extern unsigned char xdata command;


void InitSC(unsigned long baud);
void SendChar(unsigned char msg);
void SendMsg(unsigned char *msg,unsigned char length);
void Warning(unsigned char num);
void SendPack(unsigned char errcode,unsigned char *msg,unsigned short length);
void WTH_2L(unsigned long int dat);
unsigned char Play_voice(unsigned char addr);


#undef EXTERN
#endif