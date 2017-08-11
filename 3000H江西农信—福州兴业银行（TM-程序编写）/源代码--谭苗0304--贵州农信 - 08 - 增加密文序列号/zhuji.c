#include "UART.h"
#include "8820.h"
#include "EEPROM.h"
#include "ErrorCode.h"

#define KEYPORT  P2

#define tempkey  33

sbit OPEN=P3^5;		   //拆机自毁引脚
sbit GreenLED=P1^6;		//绿灯
sbit RedLED=P1^7;		//红灯


unsigned char xdata cmd[256];//接收到的命令缓存
unsigned char xdata cmdlength;//接收到的命令长度
unsigned char xdata command;

unsigned char xdata keyflag;  //键盘开放标志
unsigned char xdata keycode[16]; //输入的密码缓存
unsigned char xdata keylength;	//输入密码长度
unsigned char xdata maxkeylen;	//最大输入密码长度

unsigned char xdata nowkey;
unsigned char xdata EncryptMode;
unsigned char xdata account[8];

unsigned char xdata destoryflag=0;	   //拆机自毁标志
unsigned char xdata recoverflag=1;	   //拆机自毁恢复标志
unsigned char xdata enabledestory=0;   //0：不允许拆机自毁，1：允许拆机自毁

unsigned char xdata duanflag=0;
unsigned char xdata reflag=0;
signed char testdata;


void GetCode(void);
void RecDeal(void);
char InitDevice(void);
void Destory_Deal(void);
char TwoToOne(unsigned char *msg,unsigned char length);
char Equal(unsigned char *msg1,unsigned char *msg2,unsigned char length);
void Padding(unsigned char *msg,unsigned char *length,unsigned char *output,unsigned char flag);
char Encrypt(unsigned char keyadd,unsigned char *msg,unsigned char length,unsigned char flag);

/*将拆分后的两个字节合并成一个字节*/
char TwoToOne(unsigned char *msg,unsigned char length)
{
	unsigned char data i;

	if(length & 1)return -1;//长度必须为偶数

	for(i=0;i<length>>1;i++)
		msg[i]=(msg[i*2]<<4)&0xf0|msg[i*2+1]&0x0f;
	return 0;
}

/*比较两个数据串是否相等，长度为0时相等*/
char Equal(unsigned char *msg1,unsigned char *msg2,unsigned char length)
{
	while(length--)
		if(msg1[length] ^ msg2[length])
			return -1; 

	return 0;
}

/*补位规则，将数据补位成8的倍数长度（末尾补0x00），补位后的数据由output传出*/
void Padding(unsigned char *msg,unsigned char *length,unsigned char *output,unsigned char flag)
{
	if(output == NULL)
		output = msg;
	if(output != msg)
		memcpy(output,msg,*length);
	if(flag == 1)
	{
		memset(output+*length,0,16-(*length&15));				//&7
		*length = (*length+15)&0xf0;
	}
	else if(flag == 2)
	{
		memset(output+*length,0,8-(*length&7));
		*length = (*length+7)&0xf8;
	}
}

//用EEPROM add内的密钥加解密长度为lengthh内容为msg的数据，处理后的结果仍旧存放msg内
char Encrypt(unsigned char add,unsigned char *msg,unsigned char length,unsigned char flag)
{								
	unsigned char data key[26];

	if(length ^ (length & 0xf8))return -1;//长度必须为8的倍数

	Read_Flash(add,key);//获取密钥

	if(key[0] == 8)//密钥为单DES
	{
		while(length)
		{	 
			length -= 8;
			des_1(msg+length,msg+length,key+1,flag);
			
		}
	}
	else if(key[0] == 16)//密钥为双DES
	{
		while(length)
		{	 
			length -= 8;
			des_2(msg+length,msg+length,key+1,flag);
		}
	}
	else if(key[0] ==24)//密钥为三DES
	{
		while(length)
		{	  
			length -= 8;
			des_3(msg+length,msg+length,key+1,flag);
		}
	}
	else
		return -1;
}

/*关闭密码键盘，包括指示灯*/
void CloseKey(void)
{
	keyflag=0;
	keylength=0;	
	GreenLED=1;
	RedLED=1;
}																			 


void RecDeal(void)
{
	#define OVERTIME 10000	//定义连续两个字节之间的接收超时时间
	unsigned short data  time;
	unsigned char data i;
	//signed char testdata=0;
						  
	do				 //接收无效数据，直到收到命令头
	{
		while(!RI)	//未接收到数据时   
		{						  //以下添加空闲时需反复调用的函数，如扫描键盘    
			Destory_Deal();
			GetCode();
		}
		RI=0;
		cmd[0]=SBUF;
	}while(cmd[0] != 0x1b && cmd[0] != 0x02 && cmd[0] != 0x80 && cmd[0] != 0x81 && cmd[0] != 0x82 && cmd[0] != 0x83 && cmd[0] != 0x86);

	if(cmd[0] == 0x82 || cmd[0]== 0x86)
	{
		GreenLED=0;
		Play_voice(0);
		keyflag = 6;//明文输密码
		reflag=0;
		keylength = 0;
	}
	else if(cmd[0] == 0x81)
	{
		RedLED=0;
		Play_voice(2);
    keyflag = 6;//明文输密码
		reflag=0;
		keylength = 0;
	}
	else if(cmd[0] == 0x80 || cmd[0]== 0x83)
	{
		CloseKey();
	}
	
	else if(cmd[0] == 0x1b)//1B命令，北京邮储命令
	{
		cmdlength=1;
		
		while(!duanflag)
		{
			time=OVERTIME;													//接收一个字节
			while(!RI && time--);
			if(RI)
			{												
				RI=0;
				cmd[cmdlength++]=SBUF;
			}
			else return;
		
			if(cmd[1] == 'I')			//1b 49
			{
				time=OVERTIME;
				while(!RI && time--);									//再收一个数据
				if(RI)																//若收到数据，则是1b 49 XXXX命令
				{												
					RI=0;
					cmd[cmdlength++]=SBUF;
				}
				else																	//若收不到数据 则是1b 49 短命令
				{
					duanflag=1;
				}																			//由此区分49分不同执行入口，故判断不出1b 49加乱数据
			}
			
		  if(cmd[1] == 'E')												// 1b 45
			{
				 duanflag=1;
			}
			
			if(cmd[1] == '[')		  									//1b [ / 31 or 32 or 33
			{
				time=OVERTIME;
				while(!RI && time--);
				if(RI)
				{												
					RI=0;
					cmd[cmdlength++]=SBUF;
				}
				else return;
				time=OVERTIME;
				while(!RI && time--);
				if(RI)
				{												
					RI=0;
					cmd[cmdlength++]=SBUF;
				}
				else return;
				if(cmd[2] == '/' && (cmd[3]=='1' || cmd[3]=='2' ||cmd[3]=='3'))	
				{
					duanflag=1;
				}
				if(cmd[2] == '/' && cmd[3]=='4')			//1b [ / 34 8字节密钥拆分 1字节主密钥地址拆分  +  1b [ / 31 or 32
				{
					i=22;
					while(i)
					{
						i--;
						time=OVERTIME;
						while(!RI && time--);
						if(RI)
						{												
							RI=0;
							cmd[cmdlength++]=SBUF;
						}
						else break;
					}
					duanflag=1;
				}
			}
			
			if(cmd[cmdlength-1] == 0x1b)
			{
				cmdlength=1;
				duanflag=0;
				continue;
			}
			if(cmd[cmdlength-1] == 0x0d)
			{
				break;
			}
		}

		CloseKey();//每次切换命令都会关闭密码键盘
		
		//开始命令匹配
		switch(cmd[1])
		{
			case 0://连通性测试
				Warning(1);
			SendChar(0x99);
			break;
			case 0x02://远程升级
			SendChar(0xbb);
			Delay_ms(6000);
			ISP_CONTR=0x60;	//软复位
			break;
			case 0x04://开启拆机自毁功能
			enabledestory=1;
			memset(cmd,0x5a,5);
			Write_Flash(212,cmd);
			SendChar(0xbb);
			break;
			case 0x06://关闭拆机自毁功能
			enabledestory=0;
			memset(cmd,0xa5,5);
			Write_Flash(212,cmd);
			SendChar(0xbb);
			break;
			case 0x08://恢复出厂
			if(InitDevice() < 0)
				SendChar(0x55);
			else
				SendChar(0xbb);
			break;
			case 0x10://读取记录
			Read_Flash(cmd[2]*256+cmd[3],cmd+10);
			SendMsg(cmd+10,25);
			break;
			case 0x12://修改波特率
			switch(cmd[2])
			{
				case 1:
				InitSC(9600);	//设置通讯波特率为1200
				break;
				case 2:
				InitSC(2400);	//设置通讯波特率为2400
				break;
				case 3:
				InitSC(4800);	//设置通讯波特率为4800
				break;
				case 4:
				InitSC(1200);	//设置通讯波特率为9600
				break;
				case 5:
				InitSC(115200);	//设置通讯波特率为115200
				break;

				default:
				InitSC(9600);	//设置通讯波特率为9600
				break;
			}
			Write_Flash(214,cmd+2);
			SendChar(0xbb);
			break;
		}
	
		if(recoverflag == 0)return;
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////命令///////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		switch(cmd[1])
		{
			case '[':
			duanflag=0;
			if(cmd[3] == '1')
			{
				GreenLED=0;
				Play_voice(0);
			  keyflag = 6;//明文输密码
		    keylength = 0;
				break;
			}
			else if(cmd[3] == '2')
			{
				RedLED=0;
				Play_voice(2);
			  keyflag = 6;//明文输密码
				//reflag=0;
		    keylength = 0;
				break;
			}
			else if(cmd[3] == '3')
			{
				CloseKey();
				break;
			}				
			else if(cmd[3] == '4'&& (cmdlength == 22	|| cmdlength == 26))
			{		
				#define IsInit	cmd[255]
				TwoToOne(cmd+4,18);	
				
				if(cmdlength == 26)
				{				
					if(cmd[22] == 0x1b && cmd[23]=='[' && cmd[24]=='/')
					{
						if(cmd[25] == '1')
						{
							GreenLED=0;
							Play_voice(0);
							keyflag = 6;//明文输密码
							keylength = 0;
						}
						else if(cmd[25] == '2')     //原先是cmd[26],0318 TM修改
						{
							RedLED=0;
							Play_voice(2);
							keyflag = 6;//明文输密码
							keylength = 0;
						}
					}
				}
							
				if(cmd[12] > 0x09)//主密钥地址或工作密钥地址错误
				{
					SendChar(0x55);
					break;
				}
				Read_Flash(211,cmd+200);//读取主密钥是否初始化了

				IsInit = 200+(cmd[12]&0xf8?1:0);
				if(cmd[IsInit] & (1 << (cmd[12] & 0x07)))//主密钥被初始化了就重新下载初始化的值
				{		
					memset(cmd+101,'8',8);
					cmd[100]=8;
					Write_Flash(cmd[12],cmd+100);
				}
				
				if(Encrypt(cmd[12],cmd+4,8,0) < 0)//主密钥解密工作密钥		 
				{
					SendChar(0x55);
					break;
				}
			
				cmd[3]=8;
				if(LoadKey_DES(tempkey,cmd+3) < 0)
				{
					SendChar(0x55);				//55
					break;
				}
				nowkey=tempkey;
				//keyflag = 6;//明文输密码
				reflag=1;
				break;
			}
			break;
			
			case 'E':
			RedLED=0;
			Play_voice(2);
    	keyflag = 6;
			reflag =2;
			keylength = 0;
			duanflag = 0;
			break;
			
			case 0x49://'I'   带卡号输密码
			if(duanflag == 1)
			{
				GreenLED=0;
				Play_voice(0);
				keyflag = 6;
				reflag =2;
				keylength = 0;
				duanflag = 0;
				break;
			}
			else
			{
				if(cmd[2] == 0x30 || cmd[2] == 0x31)
				{
					if(cmd[2] == 0x30)//请输入密码绿灯亮
					{
						GreenLED=0;
						Play_voice(0);
					}
					else //请在输入一次红灯亮
					{
						RedLED=0;
						Play_voice(2);
					}

					keyflag = 1;//密码长度受限
					keylength = 0;
				}
				break;
			}
			break;
			
			case 'C':
			if(cmd[2] == 0x30 || cmd[2] == 0x31)
			{
				if(cmd[2] == 0x30)//请输入密码绿灯亮
				{
					GreenLED=0;
					Play_voice(0);
				}
				else //请在输入一次红灯亮
				{
					RedLED=0;
					Play_voice(2);
				}

				keyflag = 5;//密码长度受限
				keylength = 0;		
			}
			break;
					
			case 0x46://'F'	  请输入密码
			if(cmd[2] == 0x30 || cmd[2] == 0x31)
			{
				if(cmd[2] == 0x30)//请输入密码绿灯亮
				{
					GreenLED=0;
					Play_voice(0);
				}
				else //请在输入一次红灯亮
				{
					RedLED=0;
					Play_voice(2);
				}

				keyflag = 1;//密码长度受限
				keylength = 0;
				
			}
			break;
	
	    case 0x59:
			case 0x4d://'M'	修改主密钥	   //////////////////////////////////////////////////////////////////////////////////////////
			#define IsInit	cmd[255]
			TwoToOne(cmd+2,cmdlength-3);
			if(cmd[2] > 0x0f)//主密钥地址
			{
				SendChar(0x55);
				break;
			} 
			Read_Flash(cmd[2],cmd+100);//取出存储的主密钥
			Read_Flash(211,cmd+200);//读取主密钥是否初始化了

			IsInit = 200+(cmd[2]&0xf8?1:0);

			if(cmd[IsInit] & (1<< (cmd[2]&0x07)))//主密钥被初始化了
			{		
				if(Equal("88888888",cmd+3,8) < 0)//新主密钥跟默认的密钥进行比较
				{
					SendChar(0x55);
					break;
				}
			}
			else //主密钥未被初始化
			{
				if(Equal(cmd+3,cmd+101,cmd[100]) < 0)//新主密钥跟原主密钥进行比较
				{
					SendChar(0x55);
					break;
				}	
			}
			
			cmdlength=(cmdlength-5)/4;
			cmd[2+cmdlength]=cmdlength;
			Write_Flash(cmd[2],cmd+2+cmdlength);
			cmd[IsInit] &= ~(1<<(cmd[2]&0x07));
			Write_Flash(211,cmd+200);
			SendChar(0xbb);
			#undef IsInit
			break;
	
	
			case 0x53://'S'	 修改工作密钥	//////////////////////////////////////////////////////////////////////////////////////////
			#define IsInit	cmd[255]
			TwoToOne(cmd+2,cmdlength-3);
			if(cmd[2] > 0x0f || cmd[3] > 7)//主密钥地址或工作密钥地址错误
			{
				SendChar(0x55);
				break;
			}
			Read_Flash(211,cmd+200);//读取主密钥是否初始化了

			IsInit = 200+(cmd[2]&0xf8?1:0);
			if(cmd[IsInit] & (1 << (cmd[2] & 0x07)))//主密钥被初始化了就重新下载初始化的值
			{		
				memset(cmd+101,'8',8);
				cmd[100]=8;
				Write_Flash(cmd[2],cmd+100);
			}
			cmdlength=(cmdlength-7)/2;//计算工作密钥长度

			Padding(cmd+4,&cmdlength,NULL,2);//工作密钥补位操作
			if(Encrypt(cmd[2],cmd+4,cmdlength,0) < 0)//主密钥解密工作密钥		 
			{
				SendChar(0x55);
			}
			else
			{	
				cmd[202+cmd[2]] &= ~(1<<cmd[3]);
				Write_Flash(211,cmd+200);
				cmd[2]=cmd[2]*8+cmd[3]+16;
				cmd[3]=8; 

				Write_Flash(cmd[2],cmd+3);//记录工作密钥
				SendChar(0xbb);
			} 
			#undef IsInit
			break;
	
	
			case 0x41://'A'	   激活工作密钥	 //////////////////////////////////////////////////////////////////////////////////////////
			#define IsInit	cmd[255]
			TwoToOne(cmd+2,4);
			if(cmd[2] > 0x0f || cmd[3] > 7)//主密钥或工作密钥错误
			{
				SendChar(0x55);
				break;
			}
			nowkey = cmd[2]*8+cmd[3]+16;
			Read_Flash(211,cmd+200);
			if(cmd[202+cmd[2]] & (1<<cmd[3]))//如果工作密钥已经被初始化了则下载初始化后的值
			{
				memset(cmd+101,0,8);
				cmd[100]=8;
				Write_Flash(nowkey,cmd+100);
			}
			SendChar(0xbb);
			#undef IsInit
			break;
	
	
			case 0x4e://'N'	   设置长度		//////////////////////////////////////////////////////////////////////////////////////////
			TwoToOne(cmd+2,4);
			if(cmd[2] > 0x0e || cmd[2] == 0x00)//设置的密码长度超过最大值
			{
				SendChar(0x55);
				break;
			}
			maxkeylen = cmd[2];
			SendChar(0xbb);
			break;
	
	
			case 0x52://'R'		  恢复出厂	   //////////////////////////////////////////////////////////////////////////////////////////
			memset(cmd,0xff,18);
			Write_Flash(211,cmd);
			memset(cmd+1,0,8);
			cmd[0]=8;
			Write_Flash(16,cmd);//默认选中第0组工作密钥，第0组工作密钥先恢复成初始化的值
	
			nowkey=16;
			maxkeylen=6;
			SendChar(0xbb);
			break;

			case 0x56://版本 CJ908
			#define Ver "Ver5.517Y"
			SendMsg(Ver,sizeof(Ver)-1);
			#undef Ver
			break;
	
			case 0x60:		//测试按键
			RedLED=0;
			GreenLED=0;
			keyflag = 2;
			break;
	
			case 0x62:			//增加序列号输出
			memcpy(cmd,"\x38\x36\x39\x37\x30\x30\x30\x30",8);						//序列号明文	cmd
			memcpy(cmd+10,"\x0F\x1E\x2D\x3C\x4B\x5A\x69\x78",8);				//DES传输密钥	cmd+10
			if(GetRandomNum(cmd+20,8)<0)																//DES随机密钥	cmd+20
			{
				SendChar(0x55);
				break;
			}
			//SendMsg(cmd+20,8);
			des_1(cmd,cmd+50,cmd+20,1);																	//序列号密文 cmd+50
			des_1(cmd+20,cmd+60,cmd+10,1);															//随机数密文 cmd+60
			SendMsg(cmd+50,8);
			SendMsg(cmd+60,8);
			break;
			
			default:
			break;
		}
	 }

	
	else //关闭键盘	          //////////////////////////////////////////////////////////////////////////////////////////
	{
		CloseKey();	
	}
	
	#undef OVERTIME	
}

/*拆机处理函数*/
void Destory_Deal(void)
{
	if(enabledestory == 0)return; //不允许拆机自毁，直接跳出

	OPEN = 1;
	if(RI == 0 && OPEN == 1 && destoryflag == 0 && recoverflag == 1)
	{ 
		Delay_ms(5);
		if(OPEN == 0)return;
		Delay_ms(5);
		if(OPEN == 0)return;

		destoryflag=1;	//已拆机
		recoverflag=0;	//拆机未恢复
		memset(cmd,0x5a,5);
		Write_Flash(213,cmd);	 
	}
	
	if(RI == 0 && destoryflag == 1 && recoverflag == 0)
	{
		destoryflag=0;
		SendMsg("\x02\x02\x05\x1f\x03",5);

		Warning(10);
	}
}

unsigned char key_scan(void)
{
	unsigned data i;
	unsigned char data keyval;

	KEYPORT = 0x70;
	keyval = KEYPORT;							  //全部拉低，检测是否按键
    if(!RI && (keyval ^ 0x70))	//有按键
	{
	    for(i=0;i<5000;i++)
			if(RI)return 0;	//去抖动  250us

		if(keyval ^ 0x70)
		{
			KEYPORT = 0x8f;
			keyval |= KEYPORT;
			KEYPORT = 0x70;
		}
		else
			return 0;
	}
	else   
	    return 0;	 //未按键,以空格键返回	 

	switch(keyval)
	{	
		case 0xb7:
		    keyval=0x3B;//确认
			break;
		case 0xeb:
		    keyval=0x31; //1
			break;
		case 0xdb:
		    keyval=0x32; //2
			break;
		case 0xbb:
		    keyval=0x33; //3 
			break;
		case 0xed:
		    keyval=0x34; //4
			break;
		case 0xdd:
		    keyval=0x35; //5
			break;
		case 0xbd:
		    keyval=0x36; //6
			break;
		case 0xee:
			if(RI)return 0;
		    keyval=0x37; //7
			break;
		case 0xde:
		    keyval=0x38; //8
			break;
		case 0xbe:
		    keyval=0x39; //9
			break;
		case 0xd7:
		    keyval=0x30; //0  
			break;
		case 0xe7:
		    keyval=0x3a;//取消 
			break;
		case 0x6f:
		    keyval=0x3c;//
			break;
		case 0x5f:
		    keyval=0x3d;//
			break;

		case 0x3f:
		    keyval=0x3e;//
			break;
		default:
			return 0;//错误
	} 
	return keyval;	
} 

/*等待按键抬手动作*/
void WaitKeyUp(void)
{
	while(KEYPORT^0x70 && !RI);//松手检测	
}

//将输入的密码和12位账号处理成ANSI98需要的8字节数据,由cardnum返回
void ANSI98(unsigned char *codenum,unsigned char codelen,unsigned char *cardnum)
{
	unsigned char data i;
	unsigned char data temp[16];
	unsigned char data flag = cardnum[0];//glb 2016.3.8 添加 用于记录cardnum【0】

	memset(temp,0xff,16);
	codelen=codelen>14?14:codelen;
	memcpy(temp+2,codenum,codelen);
	temp[0]=0;
	temp[1]=codelen;	

	for(i=0;i<2;i++)
	{
		temp[i]=(temp[i*2]<<4)&0xf0|temp[1+i*2]&0x0f;
	}
	for(;i<8;i++)
	{
		temp[i]=(temp[i*2]<<4)&0xf0|temp[1+i*2]&0x0f;
		/*glb 2016.3.8 修改，cardnum[0]在后面会被改写，所以用一个临时变量存起来*/
		if(flag ^ 0)//账号是12位的需要处理成8位，已经是8位的就不需要处理了
			cardnum[i-2]=(cardnum[i*2-4]<<4)&0xf0|cardnum[i*2-3]&0x0f;
		temp[i]^=cardnum[i-2];	
	}
	memcpy(cardnum,temp,8);
}

void GetCode(void)
{
	unsigned char data keydata,i;

	if(RI == 0 && keyflag == 0)return;

 	keydata=key_scan();
	if(keydata == 0)return;

	if(!RI && keyflag == 1)//密文，密码长度受限
	{
		if(keydata > 0x2f && keydata < 0x3a)//按键为数字键
		{
			//if(keylength < ((cmd[3] == 0x30 || cmd[3] == 0x32)?maxkeylen:14))//密码长度受限，输入上限为最大值，密码长度不受限，输入的最大长度也不能超过14；
			if(keylength < maxkeylen)
			{
				Warning(1);
				WaitKeyUp();
				keycode[keylength++]= keydata;	
				if(keylength == maxkeylen)//长度受限时按满键自动提交
				{
					if(cmd[1] == 0x49) //ANSI98加密
					{
						ANSI98(keycode,keylength,cmd+4);
						keylength = 8;
					}
					else //普通补位后加密
						Padding(keycode,&keylength,cmd+4,1);
					if(Encrypt(nowkey,cmd+4,keylength,1) < 0)//普通加密
					{
						CloseKey();
						SendChar(0x55);
						return;
					}
					SendChar(2);
					//SendChar(keylength*2>>4|0x30);   //glb 2016.3.8注释
					//SendChar(keylength*2&0x0f|0x30); //glb 2016.3.8注释
					/*glb 2016.3.8 添加 DES加密返回长度为10 98加密返回长度为20*/
					if(cmd[1] == 0x49)
					{
						SendChar('2');
					}
					else SendChar('1');
					SendChar('0');
					/*glb 2016.3.8 修改结束*/
					for(i=0;i<keylength;i++)
					{
						SendChar(cmd[4+i]>>4|0x30);
						SendChar(cmd[4+i]&0x0f|0x30);
					}
					SendChar(3);
					CloseKey();
				}
			}
		}
		else if(keydata == 0x3a)//按键为取消键
		{
			Warning(1);
			WaitKeyUp();
			keylength=0;
		}
		else if(keydata == 0x3b)//按键为确认键
		{
			if(keylength == 0)return;
			Warning(1);	
			WaitKeyUp();

			if(cmd[1] == 0x49)//ANSI98加密
			{
				ANSI98(keycode,keylength,cmd+4);
				keylength = 8;
			}
			else   //普通补位后加密
				Padding(keycode,&keylength,cmd+4,1);
			if(Encrypt(nowkey,cmd+4,keylength,1) < 0)//普通加密
			{
				CloseKey();
				SendChar(0x55);
				return;
			}
			SendChar(2);
			//SendChar(keylength*2>>4|0x30);	//glb 2016.3.8注释
			//SendChar(keylength*2&0x0f|0x30);  //glb 2016.3.8注释
			/*glb 2016.3.8 添加 DES加密返回长度为10 98加密返回长度为20*/
			if(cmd[1] == 0x49)
			{
				SendChar('2');
			}
			else SendChar('1');
			SendChar('0');
			/*glb 2016.3.8 修改结束*/
			for(i=0;i<keylength;i++)
			{
				SendChar(cmd[4+i]>>4|0x30);
				SendChar(cmd[4+i]&0x0f|0x30);
			}
			SendChar(3);
			CloseKey();
		}
	}
	else if(!RI && keyflag == 2)
	{
		if(keydata > 0x2F && keydata < 0x3A)
		{
			Warning(1);
			WaitKeyUp();
			SendChar(keydata);
		}
		if(keydata == 0x3a)//取消键
		{
			Warning(1);
			WaitKeyUp();
			SendChar(0x08);
		}
		if(keydata == 0x3b)//确认键
		{
			Warning(1);
			WaitKeyUp();
			CloseKey();
			SendChar(0x0d);
		}	
	}
	else if(!RI && keyflag == 3)
	{	  
		if(keydata > 0x2F && keydata < 0x3A)//数字键
		{
			Warning(1);
			WaitKeyUp();
			memcpy(cmd+100,"\x02\x03\x69\x00\x00\x03",6);
			cmd[104]=keydata;
			SendMsg(cmd+100,6);
			CloseKey();
			return;
		}
		else if(keydata == 0x3b)//取消键
		{
			Warning(1);
			WaitKeyUp();
			memcpy(cmd+100,"\x02\x03\x69\x00\x00\x03",6);
			cmd[104]=0x0d;
			SendMsg(cmd+100,6);
			CloseKey();
			return;	
		}
		else if(keydata == 0x3a)//确认键
		{
			Warning(1);
			WaitKeyUp();
			memcpy(cmd+100,"\x02\x03\x69\x00\x00\x03",6);
			cmd[104]=0x08;
			SendMsg(cmd+100,6);
			CloseKey();
			return;	
		}
		else if(keydata == 0x3e)//小数点
		{
			Warning(1);
			WaitKeyUp();
			memcpy(cmd+100,"\x02\x03\x69\x00\x00\x03",6);
			cmd[104]=0x2e;
			SendMsg(cmd+100,6);
			CloseKey();
			return;	
		}
	}
	
	else if(!RI && keyflag == 5)
	{
		if(keydata > 0x2F && keydata < 0x3A && keylength < maxkeylen)
		{
			Warning(1);
			WaitKeyUp();
			if(keydata == 0x30) keydata=0x5F;
			else keydata=keydata+0x1f;
			keycode[keylength++] = keydata;	
			//SendChar('*');
		}
		if(keydata == 0x3a)//取消键
		{
			if(keylength != 0)
			{	
				Warning(1);
				WaitKeyUp();	
				keylength--;
				//SendChar(0x08);
			}
		}
		if(keydata == 0x3b)//确认键
		{
			Warning(1);							
			WaitKeyUp();
			keyflag=0;
			SendMsg(keycode,keylength);
			SendChar(0x0d);
			CloseKey();
			return;
		}
	}
	
	else if(!RI && keyflag == 6)
	{
		if(keydata > 0x2F && keydata < 0x3A && keylength < maxkeylen)
		{
			Warning(1);
			WaitKeyUp();
			keycode[keylength++] = keydata;	
			//SendChar('*');
		}
		if(keydata == 0x3a)//取消键
		{
			if(keylength != 0)
			{	
				Warning(1);
				WaitKeyUp();	
				keylength--;
				//SendChar(0x08);
			}
		}
		if(keydata == 0x3b)//确认键
		{
			Warning(1);							
			WaitKeyUp();
			//keyflag=0;
			
      if(reflag==0)
			{	
				SendMsg(keycode,keylength);
				SendChar(0x0d);
				CloseKey();
				keyflag=0;
			  return;
			}
			else if(reflag == 1)
			{
				keyflag=0;reflag=0;
				Padding(keycode,&keylength,cmd+4,2);
				//testdata=Encrypt_DES(nowkey,cmd+4,8,MODE_E);
				if(Encrypt_DES(nowkey,cmd+4,8,MODE_E) < 0)//普通加密
				{
					CloseKey();
					SendChar(0x55);
					return;
				}
				SendChar(0x02);
				SendChar(0x10);
				for(i=0;i<keylength;i++)
				{
					SendChar(cmd[4+i]>>4|0x30);
					SendChar(cmd[4+i]&0x0f|0x30);
				}
				CloseKey();
			  return;
			}
			else if(reflag == 2)
			{
				keyflag=0;reflag=0;
				SendChar(0x02);
				SendMsg(keycode,keylength);
				SendChar(0x03);
				CloseKey();
			  return;
			}
		}
	}
}

char InitDevice(void)
{

	if(Init8820(MODE_I) < 0)
	{
		Erase();
		if(Init8820(MODE_I) < 0)return -1;
	}
	Erase();

	maxkeylen = 6;
	nowkey = 16;
	EncryptMode = 0;
	CloseKey();
	destoryflag=0;
	recoverflag=1;//恢复拆机
	enabledestory=0;//不允许拆机自毁
	memset(cmd,0xa5,5);
	Write_Flash(212,cmd);
	Write_Flash(213,cmd);

	return 0;
}


/*EEPROM分配：
				0~15：北京邮储主密钥，第一字节是密钥长度，初始化后为8个0x80
				16~143：北京邮储工作密钥，第一字节是密钥长度,初始化后为8个0x00
				144~159：招行主密钥	，第一字节是密钥长度
				160~207：招行工作密钥，第一字节是密钥长度
				208~210：招行序列号，三条记录组合使用，第一字节为长度，最大序列号长度不超过60
				211：北京邮储初始化标志，第一二字节的每一位置高表示对应的主密钥是初始化状态
				212:允许拆机自毁标志
				213：拆机自毁状态
				214：波特率
				*/
void main(void)
{
	InitDelay();  //初始化延时函数
	VCC_IC=0;//给加密芯片上电，防止加密芯片第一次复位错误
	Delay_ms(50); //延时50毫秒，为了系统的稳定
	InitSC(9600);	//设置通讯波特率为9600
	InitEEPROM();

	CloseKey();
	maxkeylen=6;
	nowkey=16;
	EncryptMode=0;

	WT_CLK = 1;    //CLK反向
	WT_DI = 0;
	Delay_ms(50);	//延时，确保语音芯片的稳定
	WTH_2L(0xa0140); //芯片初始化

	if(Init8820(MODE_R) < 0)
	{
		Warning(5);
	}
	else
	{
		if(Init8820(MODE_I) < 0)
		Warning(3);
		else
		Warning(1);
	}

	memset(cmd,0,5);
	Read_Flash(212,cmd);
	if(Equal("\x5a\x5a\x5a\x5a\x5a",cmd,5) == 0)//允许拆机自毁
	{
		enabledestory=1;

		memset(cmd,0,5);
		Read_Flash(213,cmd);
		if(Equal("\x5a\x5a\x5a\x5a\x5a",cmd,5) == 0)//已拆机
		{
			destoryflag=1;
			recoverflag=0;
		}
		else
		{
			destoryflag=0;
			recoverflag=1;	
		}
	}
	else
	{
		enabledestory=0;
	}

	//获取波特率
	memset(cmd,0,5);
	Read_Flash(214,cmd);
	switch(cmd[0])
	{
		case 1:
		InitSC(9600);	//设置通讯波特率为9600
		break;
		case 2:
		InitSC(2400);	//设置通讯波特率为2400
		break;
		case 3:
		InitSC(4800);	//设置通讯波特率为4800
		break;
		case 4:
		InitSC(1200);	//设置通讯波特率为1200
		break;
		case 5:
		InitSC(115200);	//设置通讯波特率为115200
		break;

		default:
		InitSC(9600);	//设置通讯波特率为9600
		break;
	}

	while(1)
	{
		RecDeal();
	}
}
