#include "UART.h"
#include "MCU.h"
#include "8820.h"
#include "EEPROM.h"

#define KEY_CLOSE	0x80
#define KEY_BEEP_ON	0x01

sbit OPEN=P3^5;		   //拆机自毁引脚

unsigned char xdata nowkey=0;


unsigned char xdata cmd[256];
unsigned char xdata cmdlength;
unsigned char xdata keyflag;
unsigned char xdata keycode[16];
unsigned char xdata keylength;
unsigned char xdata maxkeylen;

unsigned char xdata destoryflag=0;	   //拆机自毁标志
unsigned char xdata recoverflag=1;	   //拆机自毁恢复标志
unsigned char xdata enabledestory=0;

unsigned char code KEY[10][8]={	"\x23\x4d\xaa\xd2\xd0\x4e\x9a\xfa",
								"\x90\x9a\xd9\x23\xdf\xac\xcb\xe2",
								"\x43\x20\x0a\xff\xdf\x02\x3a\xbc",
								"\x86\x74\x89\xab\x23\x01\x23\x20",
								"\x02\xa3\xe4\x80\x02\xdf\xaf\xfa",
								"\x00\x1c\x11\x86\x00\x09\xff\xae",
								"\x23\xaa\x4c\xaf\x29\x10\x3f\xde",
								"\x01\x2e\xa0\xc2\xd4\x5d\xf2\x50",
								"\xee\xa2\xdf\x24\x50\x0f\xee\xab",
								"\x88\x79\x20\xd8\x3f\xcc\xa2\x35"};


void GetCode(void);
void RecDeal(void);
char TwoToOne(unsigned char *msg,unsigned char length);
char OneToTwo(unsigned char *msg,unsigned char length);
char InitDevice(void);
void Destory_Deal(void);


char TwoToOne(unsigned char *msg,unsigned char length)
{
	unsigned char data i;

	if(length & 1)return -1;

	for(i=0;i<length>>1;i++)
		msg[i]=(msg[i*2]<<4)&0xf0|msg[i*2+1]&0x0f;
	return 0;
}

char OneToTwo(unsigned char *msg,unsigned char length)
{
	unsigned char data i;

	for(i=length;i>0;i--)
	{
		msg[i*2-1]=msg[i-1]&0x0f|0x30;
		msg[i*2-2]=(msg[i-1]>>4)&0x0f|0x30;	
	}
	return 0;
}

void RecDeal(void)
{
	#define OVERTIME 10000
	unsigned int data  time;
						  
	do				 //接收无效数据，知道收到命令头
	{
		while(!RI)	//未接收到数据时   
		{						  //以下添加空闲时需反复调用的函数，如扫描键盘    
			Destory_Deal();
			GetCode();
		}
		RI=0;
		cmd[0]=SBUF;
	}while(cmd[0] != 0x1b);

	cmdlength=0;

	while(1)
	{
		time=OVERTIME;
		while(!RI && time--);
		if(RI)
		{
			RI=0;
			cmd[++cmdlength]=SBUF;
		}
		else
		{
			if(cmd[1] == 0x52 && cmdlength == 1)
			{
				//cmdflag=1;
				break;
			}
			if(cmd[1] == 0x5b && cmd[2] == 0x2f && cmdlength == 3)
			{
				//cmdflag=1;
				break;
			}
			return;
		}
		if(cmdlength != 0 && cmd[cmdlength] == 0x1b)
		{
			//cmdlength=0;
			continue;
		}
		if(cmd[cmdlength] == 0x0d)
		{
			//cmdflag=1;
			break;
		}
	}

	switch(cmd[1])
	{
		case 0x49:
		if(InitDevice() < 0)
			SendChar('f');
		else
			SendChar('k');
		return;
		case 0x44:
		if(recoverflag)
		{
			SendChar('K');
			SendChar('F');	
		}
		else
		{
			SendChar('K');
			SendChar('K');
		}
		return;
		case 0x03:
		Delay_ms(6000);
		ISP_CONTR=0x60;	
		return;
		case 0x56:
		#define Ver	"JHV1.50-GM"
		cmd[0]=0x6b;
		cmd[1]=strlen(Ver)*2;
		memcpy(cmd+2,Ver,strlen(Ver));
		OneToTwo(cmd+1,strlen(Ver)+1);
		SendMsg(cmd,strlen(Ver)*2+3);
		return;
		case 0x4a://J 
		if(Write_Flash(2,"\xA5\xA5\xA5\xA5\xA5") < 0)
		{
			SendChar('f');
		}
		else
		{
			SendChar('k');
			enabledestory=1;
		}
		break;
		case 0x4b://K
		if(Write_Flash(2,"\x00\x00\x00\x00\x00") < 0)
		{
			SendChar('f');
		}
		else
		{
			SendChar('k');
			enabledestory=0;
		}
		break;

		case 0x00:
		SendChar('k');SendChar(0x99);	 Putbytespi(0xff);
		break;	   
		case 0x01:
		cmd[10]=IC_Reset(cmd+11);
		SendChar('k');
		SendMsg(cmd+11,cmd[10]);
		break;
		case 0x02:
		SendChar('k');
		SendChar(Init8820(MODE_I));
		break;
		case 0x04:
		SendChar('k');
		SendChar(Erase());
		break;
		case 0x05:
		memset(cmd,0xff,5);
		Write_Flash(0,cmd);
		Write_Flash(1,cmd);
		SendChar('k');
		break;
		case 0x06:
		Read_Flash(cmd[2],cmd+10);
		SendChar('k');
		SendMsg(cmd+10,5);
		break;

		case 0x07:
		memcpy(cmd,"\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10",16);
		SendChar(LoadKey_SM4(0,cmd));
		//memcpy(cmd,"\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10",16);  
		memcpy(cmd,"\x06\x12\x26\x62\xA9\x87\x6F\xED\x00\x00\x00\x00\x00\x00\x00\x00",16);
		SendChar(Encrpty_SM4(0,cmd,16,MODE_E));
		SendMsg(cmd,16);
		SendChar(Encrpty_SM4(0,cmd,16,MODE_D));
		SendMsg(cmd,16);
		break;

		case 0x08://写EEPROM数据
		TwoToOne(cmd+2,cmdlength-2);
		if(cmd[2] >= 30)//判断地址是否正确
		{
			SendChar('f');
			break;
		}
		cmd[1] = cmd[2]+20;//cmd[1]存储实际记录地址
		cmd[2] = (cmdlength - 4)/2;
		if(cmd[2] > 30 || cmd[2] == 0)//判断记录长度是否正确
		{
			SendChar('f');
			break;
		}
		Write_Flash(cmd[1],cmd+2);
		SendChar('k');
		break;

		case 0x09://读EEPROM数据
		TwoToOne(cmd+2,2);
		if(cmd[2] >= 30)//判断地址是否正确
		{
			SendChar('f');
			break;
		}
		Read_Flash(cmd[2]+20,cmd+100);
		if(cmd[100] > 30 || cmd[100] == 0)//判断记录长度是否正确
		{
			SendChar('f');
			break;
		}
		cmd[99] = cmd[100];//暂存记录长度，因长度也要被拆分
		cmd[100] <<= 1;//记录长度改为拆分后长度
		OneToTwo(cmd+100,cmd[99]+1);
		SendChar('k');
		SendMsg(cmd+100,cmd[99]*2+2);
		break;
	}
	if(!recoverflag)
	{
		SendChar('K');
		SendChar('K');
		return; 
	}
	
	switch(cmd[1])
	{
		case 0x5b://'['
		if(cmd[3] == 0x31 || cmd[3] == 0x32)
		{
			Putbytespi(0xff);
			keyflag=1;	
		}
		else if(cmd[3] == 0x33)
		{
			Putbytespi(0x80);	
			keyflag=0;
		}
		else
		{}
		break;
		case 0x52://'R'
		Putbytespi(0x80);	
		keyflag=0;
		SendChar('k');
		break;
		case 0x4d://'M'
		Init8820(MODE_I);
		Delay_ms(10);
		TwoToOne(cmd+2,36);
		des(cmd+4,cmd+4,KEY[cmd[3]],0);
		////////////////............///////
		//des(cmd+12,cmd+100,cmd+4,1);
		////////..............////////////////
		cmd[3]=8;
		if(LoadKey_DES(cmd[2],cmd+3) < 0)
		{
			SendChar(0x66);
			break;
		}
		cmd[11]=8;
		if(Encrpty_DES(cmd[2],cmd+11,MODE_E) < 0)
		{
			SendChar(0x66);
			break;
		}
		////////////////............///////
		/*if(cmd[100]!=cmd[12]||cmd[101]!=cmd[13]||cmd[107]!=cmd[19])
		{
			cmd[150]=cmd[151]=cmd[152]=cmd[153]=0xA5;
			Write_Flash(100,cmd+150);  Putbytespi(0x01);	
		}
		else
		{
			cmd[150]=cmd[151]=cmd[152]=cmd[153]=0x5A;
			Write_Flash(100,cmd+150);
		} */
		////////////////............///////
		cmd[11]=16;
		OneToTwo(cmd+11,9);
		cmd[10]=0x6b;
		SendMsg(cmd+10,19);
		break;
		case 0x4f:
		if(cmd[2] != 0x30 || cmd[3] < 0x34 || cmd[3] >0x3e)
		{
			SendChar(0x66);
			break;
		} 
		maxkeylen=cmd[3] & 0x0f;
		SendChar(0x6B);
		break;
		case 0x46://'F'
		if(cmd[2] == 0x34 || cmd[2] == 0x35)
		{
			TwoToOne(cmd+3,cmdlength-3);
			memcpy(cmd+104,cmd+12,12);
			memset(cmd+100,0,4);
			cmd[2]=cmd[3];
			cmd[3]=8;
			if(Encrpty_DES(cmd[2],cmd+3,MODE_D) < 0)
			{
				SendChar(0x66);
				break;
			}
			if(LoadKey_DES(0x80,cmd+3) < 0)
			{
				SendChar(0x66);
				break;
			}
			Putbytespi(0xff);
			keyflag=2;	
			keylength=0;
		}
		else
		{}
		break;

		case 0x4e://'N' 明文下载SM4密钥
		TwoToOne(cmd+2,34);

		if(LoadKey_SM4(cmd[2],cmd+3) < 0)//下载SM4密钥
		{
			SendChar(0x66);
			break;
		}
		nowkey = cmd[2];
		SendChar('k');	
		break;
		case 0x47://'G'密文更新SM4密钥
		TwoToOne(cmd+2,36);

		if(Encrpty_SM4(cmd[2],cmd+4,16,MODE_D) < 0)//解密得到SM4密钥明文
		{
			SendChar(0x66);
			break;
		}
		if(LoadKey_SM4(cmd[3],cmd+4) < 0)//下载SM4密钥
		{
			SendChar(0x66);
			break;
		}
		nowkey = cmd[3];
		SendChar('k');
		break;
		case 0x48://'H' 激活SM4密钥
		TwoToOne(cmd+2,2);
		nowkey = cmd[2];

		memset(cmd+4,0,16);
		if(Encrpty_SM4(cmd[2],cmd+4,16,MODE_E) < 0)
		{
			SendChar(0x66);
			break;
		}
		cmd[3] = 32;
		OneToTwo(cmd+3,17);
		cmd[2] = 'k';
		SendMsg(cmd+2,35);
		break;

		case 0x59://	请输入密码（SM4）
		if(cmd[2] == 0x34 || cmd[2] == 0x35)
		{
			if(cmdlength != 27)
			{
				SendChar('f');
				break;
			}
			TwoToOne(cmd+3,24);
			memcpy(cmd+120,cmd+3,12);
			memset(cmd+100,0,20);
			//TwoToOne(cmd+100,32);
		
			Putbytespi(0xff);
			keyflag=2;	
			keylength=0;
		}
		else
		{}
		break;

		case 0x5a:// 加密数据
		TwoToOne(cmd+2,cmdlength-2);
		cmd[100] = (cmdlength-2)>>1;//数据实际长度
		memset(cmd+2+cmd[100],0,(16-cmd[100])&15);
		cmd[100] = (cmd[100]+15)&0xf0;//待加密数据长度
		if(Encrpty_SM4(nowkey,cmd+2,cmd[100],MODE_E) < 0)
		{
			SendChar('f');
			break;
		}
		cmd[0]='k';
		cmd[1]=cmd[100]>>1;
		OneToTwo(cmd+1,cmd[100]);
		SendMsg(cmd,cmd[100]*2+3);
		break;

		case 0x80://单独打开确认取消键
		Putbytespi(cmd[2]);
		keyflag=3;	
		keylength=0;
		break;


		default:
		break;
	}
	#undef OVERTIME	
}

/*拆机处理函数*/
void Destory_Deal(void)
{
	if(enabledestory == 0)return;

	OPEN = 1;
	//Delay_ms(1);
	if(RI == 0 && OPEN == 1 && destoryflag == 0 && recoverflag == 1)
	{ 
		Delay_ms(5);
		if(OPEN == 0)return;
		Delay_ms(5);
		if(OPEN == 0)return;

		destoryflag=1;	//已拆机
		recoverflag=0;	//拆机未恢复
		Write_Flash(0,"\xA5\xA5\xA5\xA5\xA5");	 
	}
	
	if(RI == 0 && destoryflag == 1 && recoverflag == 0)
	{
		destoryflag=0;
		SendChar('K');
		SendChar('K');

		Putbytespi(KEY_BEEP_ON);
		Delay_ms(500);
		Putbytespi(KEY_BEEP_ON);
		Delay_ms(500);
		Putbytespi(KEY_BEEP_ON);
	}
}

void GetCode(void)
{
	unsigned char data keydata;

	if(RI == 0 && keyflag == 0)return;

 	keydata=Getbytespi();
	if(RI == 0 && keydata == 0)return;

	if(keyflag == 1)
	{
		if(keydata == 0x3B)
		{	
			SendChar(keydata);
			Putbytespi(0x80);	
			keyflag=0;
		}
		else if(keydata != 0)
		{
			SendChar(keydata);	
		}
	}
	if(keyflag == 2)
	{
		if(keydata > 0x2f && keydata <0x3a && keylength < maxkeylen)
		{
			keycode[keylength++]=keydata;
			SendChar('K');
			SendChar(keydata+' ');//注意不要和其他值冲突了
			Putbytespi(0x23);//允许取消键按键音
			if(keylength == maxkeylen)Putbytespi(0x20);//不允许数字键按键音
			return;
		}
		if(keydata == 0x3a && keylength)
		{
			SendChar('K');
			SendChar(' ');
			keylength--;
			if(keylength == 0)Putbytespi(0x24);//不允许取消键按键音
			Putbytespi(0x1f);//允许数字键按键音
		}
		else if(keydata == 0x3b)
		{
			if(keylength == 0)
			{
				SendChar('K');
				SendChar('1');
			}
			else if(keylength < maxkeylen)
			{
				SendChar('K');
				SendChar('2');
				keylength=0;	
			}
			else
			{
				unsigned char data i;

				Putbytespi(0x80);
				keyflag=0;

				memcpy(cmd+52,keycode,keylength);
				cmd[50]=0;
				cmd[51]=keylength;
				memset(cmd+52+keylength,0xff,32-keylength);
				for(i=0;i<16;i++)
				{
					cmd[100+i]=(cmd[100+i*2]<<4)&0xf0|cmd[101+i*2]&0x0f;
					cmd[50+i]=(cmd[50+i*2]<<4)&0xf0|cmd[51+i*2]&0x0f; 
					cmd[100+i]^=cmd[50+i];
				}

				if(cmd[1] == 0x46)//DES 加密
				{
					cmd[99]=8;
					if(Encrpty_DES(0x80,cmd+99,MODE_E) < 0)
					{
						SendChar('f');
						return;	
					}
					cmd[98]=0x6b;
					cmd[99]=0x12;
					cmd[108]=keylength;
					OneToTwo(cmd+99,10);
					SendMsg(cmd+98,21);
				}
				else//SM4加密
				{
					cmd[99]=16;
					//SendMsg(cmd+100,16);
					if(Encrpty_SM4(nowkey,cmd+100,16,MODE_E) < 0)
					{
						SendChar('f');
						return;	
					}
					cmd[98]=0x6b;
					cmd[99]=0x22;
					cmd[116]=keylength;
					OneToTwo(cmd+99,18);
					SendMsg(cmd+98,37);	
				}
			}
		}
		else if(keydata == 0x40)
		{
			SendChar('K');
			SendChar('@');	
		}
		else  if(keydata == 0x41)
		{
			SendChar('K');
			SendChar('A');
		}
		else if(keydata == 0x44)
		{
			SendChar('K');
			SendChar('D');
		}
		else
		{}
	}
	if(keyflag == 3)
	{
		if(keydata != 0)
		{
			SendChar(keydata);	
		}
	}
}

char InitDevice(void)
{
	cmdlength=0;
	keylength=0;
	keyflag=0;
	maxkeylen=6;

	if(Write_Flash(0,"\x5a\x5a\x5a\x5a\x5a") < 0)return -2;	 
	
	destoryflag=0;	//未拆机
	recoverflag=1;	//拆机已恢复

	if(Init8820(MODE_I) < 0)
	{
		Erase();
		if(Init8820(MODE_I) < 0)return -1;
	}
	Erase();
}


/*EEPROM分配：
				0：拆机自毁
				1：首次下载自动更新加密芯片
				2：允许拆机自毁*/
void main(void)
{
	InitDelay();  //初始化延时函数
	VCC_IC=0;//给加密芯片上电，防止加密芯片第一次复位错误
	Delay_ms(50); //延时50毫秒，为了系统的稳定
	InitSC(115200);	//设置通讯波特率为115200
	InitEEPROM();

	//初始化变量
	cmdlength=0;
	keylength=0;
	keyflag=0;
	maxkeylen=6;  

	
	memset(cmd,0,5);//为了结果更精确，调用之前先清零
	Read_Flash(2,cmd);//读拆机自毁标志
	if(cmd[0] == 0xA5 && cmd[1] == 0xA5 && cmd[2] == 0xA5 && cmd[3] == 0xA5 && cmd[4] == 0xA5)
	{
		enabledestory=1;
	}
	else
	{
		enabledestory=0;
	}

	if(enabledestory == 1 && OPEN == 1)
	{
		Delay_ms(5);
		if(OPEN == 1)
		{
			Delay_ms(5);
			if(OPEN == 1)
			{
				destoryflag=1;	//已拆机
				recoverflag=0;	//拆机未恢复
				Write_Flash(0,"\xA5\xA5\xA5\xA5\xA5");	
			}
		}
	}
	memset(cmd,0,5);//为了结果更精确，调用之前先清零
	Read_Flash(0,cmd);//读拆机自毁标志
	if(enabledestory == 1 && cmd[0] == 0xA5 && cmd[1] == 0xA5 && cmd[2] == 0xA5 && cmd[3] == 0xA5 && cmd[4] == 0xA5) //严格判断拆机，其他都为未拆机
	{
		destoryflag=1;//置拆机
		recoverflag=0;//清拆机恢复
	}
	else
	{
		destoryflag=0;//清拆机
		recoverflag=1;//置恢复
	
		if(Init8820(MODE_R)<0)//8820复位，作为连通性测试，若这都不过，电路有问题
		{
			//蜂鸣器5声提示
			Putbytespi(KEY_BEEP_ON);
			Delay_ms(500);
			Putbytespi(KEY_BEEP_ON);
			Delay_ms(500);
			Putbytespi(KEY_BEEP_ON);
			Delay_ms(500);
			Putbytespi(KEY_BEEP_ON);
			Delay_ms(500);
			Putbytespi(KEY_BEEP_ON);
			Delay_ms(500);
		}
		else
		{	
			if(Init8820(MODE_I)<0)//8820认证		//！！！！！！！！
			{
				//蜂鸣器2声提示
				Putbytespi(KEY_BEEP_ON);
				Delay_ms(500);
				Putbytespi(KEY_BEEP_ON);
				Delay_ms(500);
			}
	
			memset(cmd,0,5);
			Read_Flash(1,cmd);//读取是否第一次进入程序，第一次进入需要对8820擦除，建立8820的格式
			if(cmd[0] == 0xff)//未做处理的EEPROM应为0xFF 
			{
				Erase();
				memset(cmd,0,5);
				Write_Flash(1,cmd);//完成擦除后应将标志清零，下次就不需要做擦除了	
			}
			Putbytespi(0x80); //密码键盘待机状态
			Putbytespi(KEY_BEEP_ON);//密码键盘蜂鸣器响一声
		}
	}

	

	while(1)
	{
		RecDeal();
	}
}