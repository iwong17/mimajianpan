#ifndef _IC_H_
#define _IC_H_

#include "Delay.h"
#include "UART.h"
#include "HEAD.h"

sbit IO_IC=P1^2;
sbit RST_IC=P1^3;
sbit VCC_IC=P1^0;
sbit CLK0_IC=P1^1;
sbit CLK1_IC=P1^1;	

char IC_Reset(unsigned char *res);
int IC_Apdu_T0(unsigned char *cmd_send,unsigned short cmd_len,unsigned char *res);
char IC_ReceiveChar(unsigned char *receive_data,unsigned short wait_time);
char IC_SendChar(unsigned char send_data,unsigned short wait_time);
void IC_SafeCard(void);


#endif