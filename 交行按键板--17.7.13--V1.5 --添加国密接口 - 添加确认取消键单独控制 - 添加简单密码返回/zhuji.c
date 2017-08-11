#include "UART.h"
#include "8820.h"
#include "EEPROM.h"
#include "config.h"
/**************数码管显示函数****************
作用：	  数码管显示
入口参数：无
返回参数：无
**********************************/
void Display(unsigned char num)
{
    chose0_7=0x00;
	chose9 = chose8 = 0;

	if(flag & 1) LED = ledcode[num];	   //显示对应位号的数码管
	else LED=0xbf;

    switch(num)			   //选通位号，先显示再选通是为防重影
	{
    case 0:
		    chose0 = 1;	   //1号位数码管选通
			break;
		case 1:
		    chose1 = 1;		//2号位数码管选通
			break;
		case 2:
		    chose2 = 1;	   //3号位数码管选通
			break;
		case 3:
		    chose3 = 1;	   //4号位数码管选通
			break;
		case 4:
		    chose4 = 1;	   //5号位数码管选通
			break;
		case 5:
		    chose5 = 1;	   //6号位数码管选通
			break;
		case 6:
		    chose6 = 1;	   //7号位数码管选通
			break;
		case 7:
		    chose7 = 1;	   //8号位数码管选通
			break;
		case 8:
		    chose8 = 1;	   //9号位数码管选通
			break;	 
		case 9:
		    chose9 = 1;	   //10号位数码管选通
			break;
		default:		   //错误，不选通任何数码管
		    break;
	}
}

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

/**************蜂鸣器函数****************
作用：	  发出嘀声
入口参数：无
返回参数：无
****************************************/
void Beep(void)
{
	unsigned int data i,j;
	for(i = 0; i < 255; i++)     //音长
	{
		for(j = 0;j < 44;j++);   //音频，22M为17KHz
		{
			beep = ~beep;
			if (RI) return; //RI时退出，优先响应命令
		}
	}
}

/*************判断函数*******************		   //	 键盘倒过来看			   顺序为
作用：	  判断物理相邻的两个键是否显示相邻		   //		 chose0 				  1
入口参数：a:待判断的键 b:相邻键 c：相邻键 		   // chose9 chose8 chose7 	    	2 3	4
          若只有一个相邻键，则另一个参数填a		   // chose6 chose5 chose4 		    5 6	7
返回参数：若显示相邻则返回1						   // chose3 chose2 chose1 		    8 9	10
****************************************/		   //
bit Comp(unsigned char a,unsigned char b,unsigned char c)				   //  故乱序产生的十个数排列顺序
												   //	与硬件数码管排列顺序相反
{												   //   程序其他地方应做相应修改
	if(a == (b+1)%10) return 1;		//大1		   
	if(a == (c+1)%10) return 1;		
	if(a == (b+9)%10) return 1;		//小1
	if(a == (c+9)%10) return 1;
	return 0;
}

/*************乱序函数**************
作用：	  产生0~9的乱序数，排列顺序与硬件
          数码管顺序相反
入口参数：无
返回参数：若成功产生乱序数则返回1
****************************************/
bit OutOfOrder(void)

{
	unsigned char i[10] = {0,0,0,0,0,0,0,0,0,0},k;	//9重for循环的初始值，一般循环变量
	unsigned int res_recv1[10];

	long int num = 0; 						//循环次数记录变量，改进后最大值为304，可用unsigned int类型
	unsigned int random1, random2;				//随机数1，随机数2，通过移位确定随机数的每一位
	//unsigned int temp;
	//unsigned char high1,high2,low1,low2;			//取定时器的值做最初的随机数，不够稳定

	random1 = random2 = 0x0000;
	/*
	temp = 0x0001;
	high1 = high2 = TH2; //wy 17.8.7修改 原先为TH2
	low1 = low2 = TL1;

	for(k = 0;k < 8;k++)					//8次循环确定random1的16位是1还是0
	{
		if(high1&0x80)						//取TH2的最高位
			random1 |= temp;				//放入random1的最低位

		high1 = high1<<1;					//取次高位
		temp = temp<<1;

		if(low1&0x01)						//取TL1的最低位
			random1 |= temp;				//放入random1的第二位

		low1 = low1>>1;						//取TL1的次低位
		temp = temp<<1;
	}										//循环8次，将TH2、TL1交错放入random1

	temp = 0x0001;

	for(k = 0;k < 8;k++)					//random2的取得方法同上，由于TH2、TL1的插入位置不同使random2!=random1
	{
		if(low2&0x01)
			random2 |= temp;

		low2 = low2>>1;
		temp = temp<<1;

		if(high2&0x80)
			random2 |= temp;

		high2 = high2<<1;
		temp = temp<<1;
	}
	*/

	memset(res_recv1, 0xaa, 10);
	IC_Apdu_T0("\x00\x84\x00\x00\x08", 5, res_recv1);//产生8字节的随机数

	random1 = (res_recv1[0] << 8) | res_recv1[1];
	random2 = (res_recv1[2] << 8) | res_recv1[3];
	
	random1 %= 304;	  //将0~65536的随机数转为0~304的随机数
	random2 %= 304;	  //将0~65536的随机数转为0~304的随机数

	for(k = 0;k < 5;k++)	   //取数码管随机基址作为9重循环的初始值
	{
	    i[2*k] = maval[random1][k]/16;
		i[2*k+1] = maval[random1][k]%16;
	}

	for(led_key[0] = i[0];led_key[0] < 10;led_key[0]++)			//穷举chose9的数码管			//物理逻辑不相邻的乱序有92640种可能
	{																							//9重循环穷举了所有可能，但太费时间
		for(led_key[1] = i[1];led_key[1] < 10;led_key[1]++)		//穷举chose8的数码管			//事先划分为304组每组304个舍弃最后几个
		{																						//每组的第一个存入maval[][]中
			if(led_key[1] == led_key[0])continue;			//若与前面的数码管数字相同则跳出	//通过循环找到第random1组第random2个，实现乱序
			for(led_key[2] = i[2];led_key[2] < 10;led_key[2]++)	//穷举chose7的数码管			//为简化算法，穷举法的顺序与硬件顺序相反如下图
			{																					//	 0			 chose9
				if((led_key[2] == led_key[0])||(led_key[2] == led_key[1]))continue;				// 1 2 3  chose8 chose7 chose6  即循环中的led_key[0]
				if(Comp(led_key[2],led_key[1],led_key[0]))continue;//若与周围显示相邻则跳出		// 4 5 6  chose5 chose4 chose3  并非对应chose0的数码管
				for(led_key[3] = i[3];led_key[3] < 10;led_key[3]++)	//穷举chose6的数码管		// 7 8 9  chose2 chose1 chose0 	而是对应chose9的数码管
				{
					if((led_key[3]==led_key[0])||(led_key[3]==led_key[1])||(led_key[3]==led_key[2]))continue; //若与前面的数码管数字相同则跳出
					if(Comp(led_key[3],led_key[2],led_key[3]))continue;//若与周围显示相邻则跳出
					for(led_key[4] = i[4];led_key[4] < 10;led_key[4]++)	//穷举chose5的数码管
					{
						if((led_key[4]==led_key[0])||(led_key[4]==led_key[1])||(led_key[4]==led_key[2])||(led_key[4]==led_key[3]))continue;//若与前面的数码管数字相同则跳出
						if(Comp(led_key[4],led_key[1],led_key[4]))continue;//若与周围显示相邻则跳出
						for(led_key[5] = i[5];led_key[5] < 10;led_key[5]++)	//穷举chose4的数码管
						{
							if((led_key[5]==led_key[0])||(led_key[5]==led_key[1])||(led_key[5]==led_key[2])||(led_key[5]==led_key[3])||(led_key[5]==led_key[4]))continue;//若与前面的数码管数字相同则跳出
							if(Comp(led_key[5],led_key[2],led_key[4]))continue;//若与周围显示相邻则跳出
							for(led_key[6] = i[6];led_key[6] < 10;led_key[6]++) //穷举chose3的数码管
							{
								if((led_key[6]==led_key[0])||(led_key[6]==led_key[1])||(led_key[6]==led_key[2])||(led_key[6]==led_key[3])||(led_key[6]==led_key[4])||(led_key[6]==led_key[5]))continue;//若与前面的数码管数字相同则跳出
								if(Comp(led_key[6],led_key[3],led_key[5]))continue;	//若与周围显示相邻则跳出
								for(led_key[7] = i[7];led_key[7] < 10;led_key[7]++)	//穷举chose2的数码管
								{
									if((led_key[7]==led_key[0])||(led_key[7]==led_key[1])||(led_key[7]==led_key[2])||(led_key[7]==led_key[3])||(led_key[7]==led_key[4])||(led_key[7]==led_key[5])||(led_key[7]==led_key[6]))continue;//若与前面的数码管数字相同则跳出
									if(Comp(led_key[7],led_key[4],led_key[7]))continue;//若与周围显示相邻则跳出
									for(led_key[8] = i[8];led_key[8] < 10;led_key[8]++)	//穷举chose1的数码管
									{
										if((led_key[8]==led_key[0])||(led_key[8]==led_key[1])||(led_key[8]==led_key[2])||(led_key[8]==led_key[3])||(led_key[8]==led_key[4])||(led_key[8]==led_key[5])||(led_key[8]==led_key[6])||(led_key[8]==led_key[7]))continue;//若与前面的数码管数字相同则跳出
										if(Comp(led_key[8],led_key[5],led_key[7]))continue;//若与周围显示相邻则跳出
										led_key[9]=45-led_key[0]-led_key[1]-led_key[2]-led_key[3]-led_key[4]-led_key[5]-led_key[6]-led_key[7]-led_key[8];
										if(Comp(led_key[9],led_key[8],led_key[6]))continue;//若与周围显示相邻则跳出	
										num++;
										if(1+random2 == num) return 1; //找到第random1组第random2的乱序值
									}i[8] = 0;	//穷举法应从0~9循环，在第一次从i[]开始到9结束，第二次以后就要从0开始9结束
								}i[7] = 0;
							}i[6] = 0;
						}i[5] = 0;
					}i[4] = 0;
				}i[3] = 0;
			}i[2] = 0;
		}i[1] = 0;if(led_key[0] == 9)i[0] = 0; //理论上第304组304个之后，92640次溢出，会重新从第0组开始，但这儿有问题
	}	
	return 0;//未生成乱序数，基本不可能，仅仅防万一	
}

/***********显示码值匹配函数************
作用：	  改变数码管显示顺序，将显示码值
          存入全局变量
入口参数：无
返回参数：无
**************************************/
void Sequence(void)

{
    unsigned char data i;
	
	if(!seqflag || !OutOfOrder())
	{
	    for(i = 1;i < 10;i++)
		    led_key[i] = 10-i;
		  led_key[0]=0;
	}
 	
	for(i = 0;i < 10;i++)				//将数码管显示的码值存放在对应位置
	{
	    switch(led_key[9-i])			//为了将乱序与硬件顺序一致，原本led_key[i]即可
		{
		    case 0:
			    ledcode[i] = 0xc0;		//显示0
				break;
			case 1:
			    ledcode[i] = 0xf9;		//显示1
				break;
			case 2:
			    ledcode[i] = 0xa4;		//显示2
				break;
			case 3:
			    ledcode[i] = 0xb0;		//显示3
				break;
			case 4:
			    ledcode[i] = 0x99;		//显示4
				break;	  
			case 5:
			    ledcode[i] = 0x92;		//显示5
				break;
			case 6:
			    ledcode[i] = 0x82;		//显示6
				break;
			case 7:
			    ledcode[i] = 0xf8;		//显示7
				break;
			case 8:
			    ledcode[i] = 0x80;		//显示8
				break;
			case 9:
			    ledcode[i] = 0x90;		//显示9
				break;
			default:
			    ledcode[i] = 0x86;		//显示错误信息‘E’
				break;
		}
	}
}

void RecDeal(void)
{
	#define OVERTIME1 10000
	unsigned int data  time;
						  
	do				 //接收无效数据，知道收到命令头
	{
		while(!RI)	//未接收到数据时   
		{						  //以下添加空闲时需反复调用的函数，如扫描键盘    
			Destory_Deal();
			GetCode();//键盘keyflag为1时才进入
			if (flag & 8)//wy 17.7.13 修改 以下
			{
				if (seqint == 1)
				{
					unsigned long data i = 10000;
					while (i--);

					if (seqint == 1)
					{
						seqint = 0;
						if (beepflag & 0x08)Beep();
						seqflag = ~seqflag;
						chose0_7 = 0x00;
						chose9 = chose8 = 0;
						Sequence();
						SendChar(0x40 + (seqflag ? 1 : 0));
					}
				}
			}
			if (flag & 16)
			{
				if (dotint == 1)
				{
					unsigned long data i = 10000;
					while (i--);

					if (dotint == 1)
					{
						dotint = 0;
						if (beepflag & 0x10)Beep();
						SendChar(0x42);
					}
				}
			}
			if (flag & 32)
			{
				if (lagint == 1)
				{
					unsigned long data i = 10000;
					while (i--);

					if (lagint == 1)
					{
						lagint = 0;
						if (beepflag & 0x20)Beep();
						SendChar(0x44);
					}
				}
			}
		}
		RI=0;
		cmd[0]=SBUF;
	}while(cmd[0] != 0x1b);

	cmdlength=0;

	while(1)
	{
		time=OVERTIME1;
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
		SendChar('k');
		SendChar(0x99);	 
		//Putbytespi(0xff);//连通测试  //wy 17.7.13 修改
		flag=0xff;//wy 17.7.13 修改
		seqflag=0;//wy 17.7.13 修改	
		Sequence();//wy 17.7.13 修改
		beepflag=0xff;//wy 17.7.13 修改
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
			//Putbytespi(0xff); //wy 17.7.13 修改
			flag=0xff;//wy 17.7.13 修改
			seqflag=0;//wy 17.7.13 修改	
			Sequence();//wy 17.7.13 修改
			beepflag=0xff;//wy 17.7.13 修改
			keyflag=1;	
		}
		else if(cmd[3] == 0x33)
		{
			//Putbytespi(0x80); //wy 17.7.13 修改
            flag=0x80;//wy 17.7.13 修改
			seqflag=0;//wy 17.7.13 修改	
			keyflag = 0;
			Sequence();//wy 17.7.13 修改
			beepflag=0xff;//wy 17.7.13 修改			
		}
		else
		{}
		break;
		case 0x52://'R'
		//Putbytespi(0x80); //wy 17.7.13 修改
		flag=0x80;//wy 17.7.13 修改
		seqflag=0;//wy 17.7.13 修改	
		keyflag = 0;
		Sequence();//wy 17.7.13 修改
		beepflag=0xff;//wy 17.7.13 修改	
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
			//Putbytespi(0xff); //wy 17.7.13 修改
			flag=0xff;//wy 17.7.13修改
			seqflag=0;//wy 17.7.13修改	
			Sequence();//wy 17.7.13修改
			beepflag=0xff;//wy 17.7.13修改
			
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
		
			//Putbytespi(0xff); //wy 17.7.13 修改
			flag=0xff;//wy 17.7.13修改
			seqflag=0;//wy 17.7.13修改	
			Sequence();//wy 17.7.13修改
			beepflag=0xff;//wy 17.7.13修改
			
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
		//Putbytespi(cmd[2]);//case 0x13://打开确认、取消，其他状态不变 //wy 17.7.13 修改
		flag|=2;//wy 17.7.13修改
		keyflag=3;	
		keylength=0;
		break;

		default:
		break;
	}
	#undef OVERTIME1	
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

		Beep();
		Delay_ms(500);
		Beep();
		Delay_ms(500);
		Beep();
	}
}

void GetCode(void)
{
	unsigned char data keydata;

	if(RI == 1 || keyflag == 0)return; //wy 17.8.10修改 原先为 if(RI == 0 && keyflag == 0) 解决响应问题，应该优先响应上位机

 	keydata=Scankey();//wy 17.7.13修改 若flag低三位为1则返回为键值或空 若flag低三位为0则返回为*
	P4 = 0xef;
	if (P4 != 0xef)//有按键按下
	{
		while (!RI && P4 != 0xef);
	}

	if(RI == 1 || (keydata ==0 || keydata == '*'))return; //wy 17.8.10修改 原先为 if(RI == 0 && (keydata ==0 || keydata == '*')) 解决响应问题，应该优先响应上位机

	if(keyflag == 1)
	{
		if(keydata == 0x3B)
		{	
			SendChar(keydata);
			//Putbytespi(0x80); //wy 17.7.13 修改
			flag=0x80;  //wy 17.7.13修改
			seqflag=0;	//wy 17.7.13修改
			Sequence(); //wy 17.7.13修改
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
			//Putbytespi(0x23);//允许取消键按键音 //wy 17.7.13 修改
			beepflag|=0x40;//wy 17.7.13修改
			if(keylength == maxkeylen)
			//Putbytespi(0x20);//不允许数字键按键音 //wy 17.7.13 修改
			beepflag&=~0x01;//wy 17.7.13修改
			return;
		}
		if(keydata == 0x3a && keylength)
		{
			SendChar('K');
			SendChar(' ');
			keylength--;
			if(keylength == 0)
				//Putbytespi(0x24);//不允许取消键按键音 //wy 17.7.13 修改
			    beepflag&=~0x40;//wy 17.7.13修改
			//Putbytespi(0x1f);//允许数字键按键音 //wy 17.7.13 修改
			    beepflag|=0x01;//wy 17.7.13修改
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

				//Putbytespi(0x80); //wy 17.7.13 修改
				flag=0x80;//wy 17.7.13修改
				seqflag=0;//wy 17.7.13修改	
				Sequence();//wy 17.7.13修改
				
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
	TR0 = 0; //定时器0 关闭 wy 17.8.7 修改

	if(Write_Flash(0,"\x5a\x5a\x5a\x5a\x5a") < 0)return -2;	 
	
	destoryflag=0;	//未拆机
	recoverflag=1;	//拆机已恢复

	if(Init8820(MODE_I) < 0)
	{
		Erase();
		if(Init8820(MODE_I) < 0)return -1;
	}
	Erase();

	TR0 = 1;//定时器0 开启 wy 17.8.7 修改
	return 0;
}
/**************键盘扫描函数****************
作用：	  扫描键盘，返回对应数码管键值
入口参数：无
返回参数：按键对应数码管的键值，可重定义
*******************************************/
unsigned char Scankey(void)
{
  unsigned char data keycode = 0;
	unsigned int data i=10000;

	if(flag & 0x07)//0~9 确认 取消
	{
	    //KEYPORT = 0xe0;  //列线拉高
		P1 |= 0xf0;//高3位为列 拉高  //wy 17.7.14修改 
		P4 = 0x00;//低四位为行,拉低//wy 17.7.14修改
		if((P1 & 0xf0) != 0xf0)  //有键按下
		{
			while(i--) if(RI) return 0;
			if((P1 & 0xf0) != 0xf0)
			{
			    keycode = KEYPORT;   //取得列线值
				//KEYPORT = 0x1f;	  //行线拉高
	            P1 &= 0x1f;//行线拉高 高三位拉低 //wy 17.7.14修改
				P4 = 0xef;//wy 低四位拉高17.7.14修改
				 if(P4 != 0xef)  //键还按着
							keycode |= KEYPORT; //取得行线列线值
			}
		}

		seq = 1;// P3^4 设为输入口 拉高
		if (seq == 0)//wy 17.7.14修改
		{
			i = 1000;
			while (i--);
			while (!RI && seq == 0); //抬手检测		
			seqint = 1;
		}

		 if(flag & 1)
		{
			switch(keycode)	  //键值匹配
			{
				case 0xde:
					if(beepflag & 1)Beep();	
					return led_key[9]+0x30;	 //返回当前按键对应的数码管值，为ASC码
				case 0xbe:
					if(beepflag & 1)Beep();	
					return led_key[8]+0x30;
				case 0x7e:
					if(beepflag & 1)Beep();	
					return led_key[7]+0x30;
				case 0xdd:
					if(beepflag & 1)Beep();	
					return led_key[6]+0x30;
				case 0xbd:
					if(beepflag & 1)Beep();	
				    return led_key[5]+0x30;
				case 0x7d:
					if (RI) return 0; //RI时退出，优先响应命令
					if(beepflag & 1)Beep();	
				    return led_key[4]+0x30;
				case 0xdb:
					if(beepflag & 1)Beep();	
				    return led_key[3]+0x30;
				case 0xbb:
					if(beepflag & 1)Beep();	
				    return led_key[2]+0x30;
				case 0x7b:
					if(beepflag & 1)Beep();	
				    return led_key[1]+0x30;	
				case 0xb7:
					if(beepflag & 1)Beep();	
			    	return led_key[0]+0x30;	  
				default:
			    	;	//未按键或错误键值
			}								
		}
		if(flag & 2)
		{
			if(keycode == 0xd7)
			{
				if(beepflag & 0x40)Beep();	
				return 0x3a;//取消
			}
			if(keycode == 0x77)
			{
				if(beepflag & 0x02)Beep();	
				return 0x3b;//确认
			}
		}
		if(flag & 4)
		{
			if(keycode == 0xcf)
			{
				if(beepflag & 0x04)Beep();	
				return 0x3c;
			}
			if(keycode == 0xaf)
			{
				if(beepflag & 0x04)Beep();	
				return 0x3d;
			}
			if(keycode == 0x6f)
			{
				if(beepflag & 0x04)Beep();	
				return 0x3e;
			}
		}
	}
	return '*';		 //键盘未开启时，以空值返回
}



/**************定时器初始化函数****************
作用：	  2ms定时器初始化 定时器0
入口参数：无
返回参数：无
**********************************/
void InitTime(unsigned long us)
{
    TMOD |= 0x01; //设置定时器0 16位
	//TH0 = (65536-3686)/256;
    //TL0 = (65536-3686)%256;
	TH0 = (65536-120000*us/221184)/256;		 //2ms
	TL0 = (65536-120000*us/221184)%256;		 //是否为3686还待斟酌
	ET0 = 1; //定时器0中断运行
	TR0 = 1; //定时器开始计时
}

/**************2ms定时器中断函数****************
作用：	  产生2ms定时中断，依次显示十个数码管
          一次中断显示一个数码管，频率50Hz
入口参数：无
返回参数：无
**********************************/
void Timer0() interrupt 1 using 1	 //2ms中断函数			 //////当keyflag跳变时，number的切换是否会引起显示滞留？？？？
{
    static unsigned char num=0;		 //数码管位号
	  static unsigned char number=0;
	  unsigned int i = 10000;
    TH0 = (65536-3686)/256;
    TL0 = (65536-3686)%256;
	  number++;
	  
	  if(flag & 1)						
	  {
	    if(number > 9) number = 0;		//0~9循环
		  num=number;
	  }
	  else
	  {
	    if(number>254)			//延时用，显示待机时相邻数码管的间隔时间
		  {
		    number=0;
			  num++;
			  if(num > 9)num = 0;
		  }
	  }
	  Display(num);			//点亮对应位号的数码管，keyflag=1时20ms一循环
}

//void SeqInterrupt(void) interrupt 2 //wy 17.7.14修改
//{
	//seqint=1;
//}

void DotInterrupt(void) interrupt 0 //wy 17.7.14修改 原先是6
{	
	dotint=1;
}


void LagInterrupt(void) interrupt 2 //wy 17.7.14修改 原先是7
{	
	lagint=1;
}

/*EEPROM分配：
				0：拆机自毁
				1：首次下载自动更新加密芯片
				2：允许拆机自毁*/
void main(void)
{
	InitDelay();    //初始化延时函数 定时器2
	InitTime(2);    //定时器初始化 定时器0 //wy 17.7.14修改
	VCC_IC=0;		//给加密芯片上电，防止加密芯片第一次复位错误
	Delay_ms(50);   //延时50毫秒，为了系统的稳定
	InitSC(115200);	//设置通讯波特率为115200
	InitEEPROM();
	
	EX0=1;IT0=1;//外部中断0 向量号为0 dotint//wy 17.7.14修改
	EX1=1;IT1=1;//外部中断1 向量号为2 lagint//wy 17.7.14修改
	
	Sequence();//wy 17.7.14修改
	//Beep();				  //开机自检音

	

	//初始化变量
	cmdlength=0;
	keylength=0;
	keyflag=0;
	maxkeylen=6;  

    seqint=0;//wy 17.7.14修改
	dotint=0;//wy 17.7.14修改
	lagint=0;//wy 17.7.14修改
	flag=0;//wy 17.7.14修改
	
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
			Beep();//wy 17.7.13修改
			Delay_ms(500);
			Beep();//wy 17.7.13修改
			Delay_ms(500);
			Beep();//wy 17.7.13修改
			Delay_ms(500);
			Beep();//wy 17.7.13修改
			Delay_ms(500);
			Beep();//wy 17.7.13修改
			Delay_ms(500);
		}
		else
		{	
			if(Init8820(MODE_I)<0)//8820认证		//！！！！！！！！
			{
				//蜂鸣器2声提示
				Beep();//wy 17.7.13修改
				Delay_ms(500);
				Beep();//wy 17.7.13修改
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
			//Putbytespi(0x80); //密码键盘待机状态 wy 17.7.13修改
			flag=0x80;//wy 17.7.13修改
			seqflag=0;//wy 17.7.13修改	
			Sequence();//wy 17.7.13修改
			Beep();//wy 17.7.13修改
		}
	}
	while(1)
	{
		RecDeal();
		
	}
}