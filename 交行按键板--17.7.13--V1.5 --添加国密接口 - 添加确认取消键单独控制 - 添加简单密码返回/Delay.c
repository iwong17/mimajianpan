#include "Delay.h"

static unsigned short idata flag; //定时标志

void InitDelay(void) //定时器2
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