#include "MCU.h"

unsigned char bdata  DATA;     //通信接、发数据暂存变量
sbit  BIT0 = DATA^0;
sbit  BIT7 = DATA^7;

/**************单片机发送函数****************
作用：	  向另一单片机发送一字节数据
入口参数：待发送数据
返回参数：无
********************************************/
void Putbytespi(unsigned char ch)
{
    unsigned char i;
    
    if(!MCLK) return; //  
	  
    MCLK = 0;    //申请通讯
    DATA = ch;

    for(i = 4;i != 0;i--)
    {
        while(SCLK); //检测从机响应
    
        SDATA = BIT0;   //放一个bit到SDATA
        MCLK = 1;        
        DATA = DATA>>1; //准备下一个bit

        while(!SCLK); //检测从机响应
    
        SDATA = BIT0;   //放一个bit到SDATA
        MCLK = 0;        
        DATA = DATA>>1; //准备下一个bit
    }

    while(SCLK); //检测从机收完

    MCLK = 1;

    while(!SCLK);//等待从机准备好
    
	SDATA = 1;
} 


/**************单片机接收函数****************
作用：	  接收另一单片机传来的一字节数据
入口参数：无
返回参数：无
********************************************/
unsigned char Getbytespi(void)
{
    unsigned char i;

    //while(MCLK);
	if(MCLK)return 0;	//改动处

    SCLK = 0;    //接受通讯请求

    for(i = 4;i != 0;i--)
    {
        DATA = DATA>>1; //准备收下一个bit

        while(!MCLK); //检测主机放数据

        BIT7 = SDATA;//收一个bit
        SCLK = 1;



        DATA = DATA>>1; //准备收下一个bit

        while(MCLK); //检测主机放数据

        BIT7 = SDATA;  //收一个bit
        SCLK = 0;    
    }
    
    while(!MCLK); //检测主机发完
    
	SCLK = 1;
    return(DATA);   //改动处 
}