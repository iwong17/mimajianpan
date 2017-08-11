#ifndef _DES_H_
#define _DES_H_

#include <STC89C51RC_RD_PLUS.H>
#include "UART.h"
#include "HEAD.h"


void des_1(unsigned char *input,unsigned char *output,unsigned char *key,unsigned char flag);
void des_2(unsigned char *input,unsigned char *output,unsigned char *key,unsigned char flag);
void des_3(unsigned char *input,unsigned char *output,unsigned char *key,unsigned char flag);


#endif