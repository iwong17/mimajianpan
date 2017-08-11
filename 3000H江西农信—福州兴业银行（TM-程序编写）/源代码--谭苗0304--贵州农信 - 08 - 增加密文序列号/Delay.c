#include "Delay.h"

static unsigned short idata flag; //定时标志

void InitDelay(void)
{
	T2CON=0;
	T2MOD=0;
	flag=0;
	TF2=0;
	TR2=0;
	ET2=1;//允许
	EA=1;//总允许
}

void Delay_ms(unsigned short time)
{
	RCAP2H=0xf8;
	RCAP2L=0xcd;

	TR2=0;
	flag=time;
	TR2=1;
	while(flag);
	TR2=0;
}

void Delay_100us(unsigned short time)
{
	RCAP2H=0xFF;        //		100us执行指令：184.32条，等比缩小		//P.230
	RCAP2L=0x48;				//    寄存器值：RCAP2=65536-184.32=0xff48

	TR2=0;
	flag=time;
	TR2=1;
	while(flag);
	TR2=0;
}

bool GetDelayFlag(void)
{
	return flag;
}

void InterruptTime2(void) interrupt 5
{
	TF2=0;
	if(flag)flag--;
}

void OpenDelay(unsigned short time)
{
	RCAP2H=0xf8;
	RCAP2L=0xcd;

	TR2=0;
	flag=time;
	TR2=1;
} 

void CloseDelay(void)
{
	TR2=0;
	flag=0;
} 