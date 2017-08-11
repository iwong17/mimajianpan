#ifndef _DES_H_
#define _DES_H_

#include <STC89C51RC_RD_PLUS.H>
#include "UART.h"
#include "HEAD.h"


void des(uchar *source,uchar *dest,uchar *inkey,bit flg);
void des_2(uchar *source,uchar *dest,uchar *inkey,bit flg);
void des_3(uchar *source,uchar *dest,uchar *inkey,bit flg);


#endif