#ifndef _DELAY_H_
#define _DELAY_H_

#include <STC89C51RC_RD_PLUS.H>
#include "HEAD.h"
#include<intrins.h>

#define bool bit 

void InitDelay(void);
void OpenDelay(unsigned short time);
bool GetDelayFlag(void);
void CloseDelay(void);
void Delay_ms(unsigned short time);


#define ETU_1_4() 		{volatile unsigned char idata i=7;while(i--);_nop_();_nop_();_nop_();}	//25us
//#define ETU_1_2()		{volatile unsigned char idata i=17;while(i--);}	//50us
#define ETU()			{volatile unsigned char idata i=34;while(i--);}	//100us

#endif