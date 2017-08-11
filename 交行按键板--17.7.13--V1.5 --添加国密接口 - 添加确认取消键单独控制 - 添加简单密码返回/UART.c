#define _UART_C_
#include "UART.h"


/**************串口中断初始化函数****************
作用：	  串口中断初始化 定时器1
入口参数：无
返回参数：无
************************************************/
void InitSC(unsigned long baud)
{
    EA=0;
	//IP=0x00;//不设优先级
	//TCON=0x00; //0x05;//下降沿触发
	TMOD |= 0x20; //设置定时器1 八位自动装载模式
	SCON |= 0X50; //八位可变波特率 无奇偶校验
	PCON |= 0X80; //波特率加倍
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



