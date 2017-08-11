#include "MCU.h"

unsigned char bdata  DATA;     //ͨ�Žӡ��������ݴ����
sbit  BIT0 = DATA^0;
sbit  BIT7 = DATA^7;

/**************��Ƭ�����ͺ���****************
���ã�	  ����һ��Ƭ������һ�ֽ�����
��ڲ���������������
���ز�������
********************************************/
void Putbytespi(unsigned char ch)
{
    unsigned char i;
    
    if(!MCLK) return; //  
	  
    MCLK = 0;    //����ͨѶ
    DATA = ch;

    for(i = 4;i != 0;i--)
    {
        while(SCLK); //���ӻ���Ӧ
    
        SDATA = BIT0;   //��һ��bit��SDATA
        MCLK = 1;        
        DATA = DATA>>1; //׼����һ��bit

        while(!SCLK); //���ӻ���Ӧ
    
        SDATA = BIT0;   //��һ��bit��SDATA
        MCLK = 0;        
        DATA = DATA>>1; //׼����һ��bit
    }

    while(SCLK); //���ӻ�����

    MCLK = 1;

    while(!SCLK);//�ȴ��ӻ�׼����
    
	SDATA = 1;
} 


/**************��Ƭ�����պ���****************
���ã�	  ������һ��Ƭ��������һ�ֽ�����
��ڲ�������
���ز�������
********************************************/
unsigned char Getbytespi(void)
{
    unsigned char i;

    //while(MCLK);
	if(MCLK)return 0;	//�Ķ���

    SCLK = 0;    //����ͨѶ����

    for(i = 4;i != 0;i--)
    {
        DATA = DATA>>1; //׼������һ��bit

        while(!MCLK); //�������������

        BIT7 = SDATA;//��һ��bit
        SCLK = 1;



        DATA = DATA>>1; //׼������һ��bit

        while(MCLK); //�������������

        BIT7 = SDATA;  //��һ��bit
        SCLK = 0;    
    }
    
    while(!MCLK); //�����������
    
	SCLK = 1;
    return(DATA);   //�Ķ��� 
}