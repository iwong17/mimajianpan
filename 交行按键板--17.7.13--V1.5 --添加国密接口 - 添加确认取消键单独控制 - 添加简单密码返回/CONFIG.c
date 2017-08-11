/**************定时器初始化函数****************
作用：	  2ms定时器初始化 定时器0
入口参数：无
返回参数：无
**********************************/
void InitTime(unsigned long us)

{
    TMOD |= 0x01;
	TH0 = (65536-3686)/256;
    TL0 = (65536-3686)%256;
	//TH0 = (65536-120000*us/221184)/256;		 //2ms
	//TL0 = (65536-120000*us/221184)%256;		 //是否为3686还待斟酌
	ET0 = 1;
	TF0=0;
	TR0 = 1; //定时器开始计时
}

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
		beep = ~beep;
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
	long int num = 0; 						//循环次数记录变量，改进后最大值为304，可用unsigned int类型
	unsigned int random1,random2,temp;				//随机数1，随机数2，通过移位确定随机数的每一位
	unsigned char high1,high2,low1,low2;			//取定时器的值做最初的随机数，不够稳定

	random1 = random2 = 0x0000;
	temp = 0x0001;
	high1 = high2 = TH2;
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
	
	if(!OutOfOrder() || !seqflag)
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
