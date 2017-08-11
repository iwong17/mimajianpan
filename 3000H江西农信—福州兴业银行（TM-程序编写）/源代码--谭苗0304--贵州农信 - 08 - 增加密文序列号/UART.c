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
;ģ������:WTH_2L
;�� ��:WTH ���ߴ���ͨ�ź���,��λ��ǰ,����24 λ����
;�� ��:���͵�24 λ����
;�� ��:оƬ���ص�����
;-------------------------------------*/
void WTH_2L(unsigned long int dat)
{
	unsigned char i;
	WT_CLK = 0;     //����
	Delay_100us(2);
	for (i = 0; i < 24; i++)
	{
		WT_CLK = 0;      //����
		if (dat & 0x800000) WT_DI = 1;
		else WT_DI = 0;
		dat <<= 1;
		Delay_100us(1);
		WT_CLK = 1;      //����
		Delay_100us(1); //100us
	}
}

/*--------------------------------------
;ģ������: Play_voice
;�� ��:WTH ���ߴ���ͨ��,WTH оƬ��������
;�� ��:���ŵĵ�ַ0~255
;�� ��:0������ʧ�ܣ�1�����ųɹ�
;-------------------------------------*/
unsigned char Play_voice(unsigned char addr)
{
	unsigned long int dat; 
	dat = 0x1800c8 + (addr << 5);
	WTH_2L(dat);
	return 1; //����ʧ��
}



