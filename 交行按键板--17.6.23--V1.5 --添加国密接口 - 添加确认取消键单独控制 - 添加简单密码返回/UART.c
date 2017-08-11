#define _UART_C_
#include "UART.h"


/**************�����жϳ�ʼ������****************
���ã�	  �����жϳ�ʼ��
��ڲ�������
���ز�������
************************************************/
void InitSC(unsigned long baud)
{
    EA=0;

	IP=0x00;//�������ȼ�
	TCON=0x00; //0x05;//�½��ش���
	TMOD |= 0x22;
	SCON |= 0X50;
	PCON |= 0X80;


	TL1 = 256 - (22118400L/192L)/baud;
	TH1 = 256 - (22118400L/192L)/baud;
	
	TR1 = 1;
	EA=1;

}
									  

/*���ֽڷ���*/
void SendChar(unsigned char ch)									 	                                                                  
{
	SBUF=ch;
	while(!TI);
	TI=0;
}
			
																
/*�ַ�������*/
void SendMsg(unsigned char *msg,unsigned char length)
{
	while(length--)
	{
		SBUF=*msg++;
		while(!TI);							   
		TI=0;
	}
}



