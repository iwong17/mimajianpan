#define _UART_C_
#include "UART.h"


/**************串口中断初始化函数****************
作用：	  串口中断初始化
入口参数：无
返回参数：无
************************************************/
void InitSC(unsigned long baud)
{
    EA=0;

	IP=0x00;//不设优先级
	TCON=0x00; //0x05;//下降沿触发
	TMOD |= 0x22;
	SCON |= 0X50;
	PCON |= 0X80;


	TL1 = 256 - (22118400L/192L)/baud;
	TH1 = 256 - (22118400L/192L)/baud;
	
	TR1 = 1;
	EA=1;

}
									  

/*单字节发送*/
void SendChar(unsigned char ch)									 	                                                                  
{
	SBUF=ch;
	while(!TI);
	TI=0;
}
			
																
/*字符串发送*/
void SendMsg(unsigned char *msg,unsigned char length)
{
	while(length--)
	{
		SBUF=*msg++;
		while(!TI);							   
		TI=0;
	}
}

void SendPack(unsigned char errcode,unsigned char *msg,unsigned short length)
{
	SendChar(0x02);
	SendChar(length+2);
	SendChar(command+1);
	SendChar(errcode);
	SendMsg(msg,length);
	SendChar(0x03);
}

void Warning(unsigned char num)
{
	unsigned char i;
	unsigned int j;

	if(num==1)
	{
		for(i = 0; i < 255; i++)    
		{
			for(j = 0;j <44;j++); 
			Beep=~Beep;  
		} 
		Beep=0;
		return;
	}

	while(num--)
	{
		for(i = 0; i < 100; i++)
			for(j = 0;j <400;j++);
		
		for(i = 0; i < 255; i++)    
		{
			for(j = 0;j <44;j++); 
			Beep=~Beep;  
		} 
	}
	Beep=0;
}

/*--------------------------------------
;模块名称:WTH_2L
;功 能:WTH 二线串口通信函数,高位在前,发送24 位数据
;入 参:发送的24 位数据
;出 参:芯片返回的数据
;-------------------------------------*/
void WTH_2L(unsigned long int dat)
{
	unsigned char i;
	WT_CLK = 0;     //反向
	Delay_100us(2);
	for (i = 0; i < 24; i++)
	{
		WT_CLK = 0;      //反向
		if (dat & 0x800000) WT_DI = 1;
		else WT_DI = 0;
		dat <<= 1;
		Delay_100us(1);
		WT_CLK = 1;      //反向
		Delay_100us(1); //100us
	}
}

/*--------------------------------------
;模块名称: Play_voice
;功 能:WTH 二线串口通信,WTH 芯片播放语音
;入 参:播放的地址0~255
;出 参:0：播放失败；1：播放成功
;-------------------------------------*/
unsigned char Play_voice(unsigned char addr)
{
	unsigned long int dat; 
	dat = 0x1800c8 + (addr << 5);
	WTH_2L(dat);
	return 1; //播放失败
}



