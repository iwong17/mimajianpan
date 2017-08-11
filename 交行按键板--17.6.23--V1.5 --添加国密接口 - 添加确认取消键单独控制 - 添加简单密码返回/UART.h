#ifndef _UART_H_
#define _UART_H_

#include <STC89C51RC_RD_PLUS.H>
#include <string.h>

//头文件的包含一定要在EXTERN之前
#ifdef _UART_C_
#define EXTERN 
#else
#define EXTERN extern
#endif


void InitSC(unsigned long baud);
void SendChar(unsigned char msg);
void SendMsg(unsigned char *msg,unsigned char length);



#undef EXTERN
#endif