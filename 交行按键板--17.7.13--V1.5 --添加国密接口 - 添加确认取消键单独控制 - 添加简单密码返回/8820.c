#define _8820_C_

#include "8820.h"

#define Pos 128
#define cmd_send temp
#define res_recv (temp+Pos)

unsigned char xdata i;
/*
函数说明：加密芯片复位或认证函数
入口参数：mode: MODE_R 复位模式  
				 MODE_I 认证模式
返回：
			0：正确返回
			-1：APDU错误，具体错误看err
*/
char Init8820(unsigned char mode)//认证
{
	unsigned char data zero=0;
	if(zero != 0)
	{
		Erase();
		LoadKey_SM1(0,NULL,0);
		LoadKey_SM4(0,NULL);
		LoadKey_SM2(0,NULL,0);
		LoadKey_DES_Main(0,NULL);
		LoadKey_DES_Work(0,0,NULL);
		LoadKey_DES(0,NULL);
		Encrpty_SM1(0,NULL,0,0);
		Encrpty_SM4(0,NULL,0,0);
		Encrpty_SM2(0,NULL,NULL,0);
		Encrpty_DES(0,NULL,0);
		GetKey_SM2(NULL);
		Encrpty_SM3(NULL,0);
	}

	if(IC_Reset(res_recv)<0 || res_recv[0]!=0x3b) return -1;//复位失败
	Delay_ms(50);

	if(mode==MODE_R)return 0; //仅仅复位，以下为认证
	if (IC_Apdu_T0("\x00\x84\x00\x00\x08", 5, res_recv) < 0 || res_recv[8] != 0x90 || res_recv[9] != 0) return -1;//取随机数失败	

	//5757572e42524f4144535441522e434e
	des_2(res_recv,res_recv+8,"\x57\x57\x57\x2e\x42\x52\x4f\x41\x44\x53\x54\x41\x52\x2e\x43\x4e",1);

	memcpy(cmd_send,"\x00\x82\x00\x00\x08",5);//认证命令头

	memcpy(cmd_send+5,res_recv+8,8);

	Delay_ms(1);
	if (IC_Apdu_T0(cmd_send, 13, res_recv) < 0 || res_recv[0] != 0x90 || res_recv[1] != 0) return -1;

	Delay_ms(1);
	return 0;
}
/*
函数说明：加密芯片擦除函数
入口参数：无
返回：		0：正确执行
			-1：APDU错误			
*/
char Erase(void)
{
	res_recv[0]=IC_Apdu_T0("\x80\x0e\x00\x00\x00",5,res_recv+1);//擦除

	Delay_ms(1);
	//此文件大小可以存放27组SM1 SM2 SM4密钥
	res_recv[3]=IC_Apdu_T0("\x80\xe0\x00\x00\x07\x3f\x62\x00\xff\xf0\xff\xff",12,res_recv+4);//建立KEY文件

	Delay_ms(1);
	res_recv[6]=IC_Apdu_T0("\x80\xd4\x01\x00\x15\x39\xf0\xf0\xff\xff\x57\x57\x57\x2e\x42\x52\x4f\x41\x44\x53\x54\x41\x52\x2e\x43\x4e",26,res_recv+7);//下载认证密钥，使用默认

	Delay_ms(1);
	res_recv[9]=IC_Apdu_T0("\x80\xd4\x01\x01\x15\x39\xf0\xf0\xff\xff\x57\x57\x57\x2e\x42\x52\x4f\x41\x44\x53\x54\x41\x52\x2e\x43\x4e",26,res_recv+10);

	Delay_ms(1);

	if(res_recv[0]!=2 || res_recv[3]!=2 || res_recv[6]!=2 || res_recv[9]!=2)return -1;
	if(res_recv[1]!=0x90 || res_recv[4]!=0x90 || res_recv[7]!=0x90 || res_recv[10]!=0x90)return -1;
	if(res_recv[2]!=0 || res_recv[5]!=0 || res_recv[8]!=0 || res_recv[11]!=0)return -1;

	return 0;
}


/*
函数说明：下载SM1密钥
参数：		add:密钥下载地址 00~ff ,根据实际需要使用前几组，可能芯片空间不足以存放全部256组密钥
			*msg:密钥值，长度由密钥类型决定
			keytype:	SM1:EK AK SK
返回：	0：正确执行
		-1：APDU错误
		-2：SM1的密钥类型错误
		-3：加密芯片未返回9000

*/
char LoadKey_SM1(unsigned char add,unsigned char *msg,unsigned char keytype)
{
	if(keytype!=EK && keytype!=AK && keytype!=SK) return -2;	//密钥类型错误
	memcpy(cmd_send,"\x80\xd4\x20\x00\x10",5); //更改加密密钥命令头
	cmd_send[2]=keytype;					   //更改加密密钥类型
	cmd_send[3]=add;						   //更改加密密钥地址
	memcpy(cmd_send+5,msg,16);				   //加密密钥长度
	
	if(IC_Apdu_T0(cmd_send,21,res_recv)<0)return -1;//更改加密密钥错误
	cmd_send[2]+=3;
	
	if(IC_Apdu_T0(cmd_send,21,res_recv+2)<0)return -1;//更改解密密钥错误

	if(res_recv[2]==0x94 && res_recv[3]==0x03)	//解密密钥不存在，不能更改，需要直接下载
	{
		cmd_send[5]=cmd_send[2];				//修改命令头为直接下载解密密钥
		cmd_send[2]=0x01;
		cmd_send[4]=0x15;
		memcpy(cmd_send+6,"\xf0\xf0\x01\x01",4);
		memcpy(cmd_send+10,msg,16);	//密钥值	

		if(IC_Apdu_T0(cmd_send,26,res_recv+2)<0)return -1;//下载解密密钥
	}	 
	cmd_send[5]-=3;
	if(res_recv[0]==0x94 && res_recv[1]==0x03)
	{	
		if(IC_Apdu_T0(cmd_send,26,res_recv)<0)return -1;//下载加密密钥	
	}

	if(res_recv[0]==0x90 && res_recv[1]==0 && res_recv[2]==0x90 && res_recv[3]==0)
	{	
		return 0;
	}
	else
	{
		return -3;
	}	
}

/*
函数说明：下载SM4密钥
参数：		add:密钥下载地址 00~ff ,根据实际需要使用前几组，可能芯片空间不足以存放全部256组密钥
			*msg:密钥值，长度由密钥类型决定
返回：	0：正确执行
		-1：APDU错误
		-2：SM4的密钥类型错误
		-3：加密芯片未返回9000

*/
char LoadKey_SM4(unsigned char add,unsigned char *msg)
{
	memcpy(cmd_send,"\x80\xd4\x26\x00\x10",5); //更改加密密钥命令头
	cmd_send[3]=add;						   //更改加密密钥地址
	memcpy(cmd_send+5,msg,16);				   //加密密钥长度

	if(IC_Apdu_T0(cmd_send,21,res_recv)<0)return -1;//更改加密密钥错误
	cmd_send[2]++;
	
	if(IC_Apdu_T0(cmd_send,21,res_recv+2)<0)return -1;//更改解密密钥错误

	if(res_recv[2]==0x94 && res_recv[3]==0x03)	//解密密钥不存在，不能更改，需要直接下载
	{
		cmd_send[5]=cmd_send[2];				//修改命令头为直接下载解密密钥
		cmd_send[2]=0x01;
		cmd_send[4]=0x15;
		memcpy(cmd_send+6,"\xf0\xf0\x01\x01",4);
		memcpy(cmd_send+10,msg,16);	//密钥值	

		if(IC_Apdu_T0(cmd_send,26,res_recv+2)<0)return -1;//下载解密密钥
	}	 
	cmd_send[5]--;
	if(res_recv[0]==0x94 && res_recv[1]==0x03)
	{	
		if(IC_Apdu_T0(cmd_send,26,res_recv)<0)return -1;//下载加密密钥	
	}

	if(res_recv[0]==0x90 && res_recv[1]==0 && res_recv[2]==0x90 && res_recv[3]==0)
	{	
		return 0;
	}
	else
	{
		return -3;
	}	
}
/*
函数说明：SM2密钥下载，DK,QX，QY
参数：
		add:密钥下载地址 最多120组
		msg:密钥值
		keytype：密钥类型，DK，QX，QY
返回：
		0：正确执行
		-1：APDU错误
		-4：密钥类型错误
		-5：地址错误
*/
char LoadKey_SM2(unsigned char add,unsigned char *msg,unsigned char keytype)
{
	if(add>120)return -5;	   //地址错误
	if(keytype!=DK && keytype!=QX && keytype!=QY)return -4;	 //密钥类型错误

	memcpy(cmd_send,"\x00\xa4\x00\x00\x02\x00\x01",7);	//选择密钥文件
	cmd_send[6]=keytype==DK?add*2+1:add*2+2;

	if(IC_Apdu_T0(cmd_send,7,res_recv)<0)return -1;
	if(res_recv[0]==0x6a && res_recv[1]==0x82)		  //未找到密钥文件，需要重新建立
	{
		if(keytype==DK)
		{
			memcpy(cmd_send,"\x80\xe0\x00\x01\x07\x3b\x00\x20\xf0\xf0\xef\xff",12);//建立私钥文件
			cmd_send[3]=add*2+1;
		}
		else
		{
			memcpy(cmd_send,"\x80\xe0\x00\x02\x07\x3c\x00\x40\xf0\xf0\xf0\xff",12);//建立公钥文件	
			cmd_send[3]=add*2+2;
		}	

		if(IC_Apdu_T0(cmd_send,12,res_recv)<0 || res_recv[0]!=0x90 || res_recv[1]!=0)return -1;

		memcpy(cmd_send,"\x00\xa4\x00\x00\x02\x00\x01",7);//重新选择文件
		cmd_send[6]=keytype==DK?add*2+1:add*2+2;

		if(IC_Apdu_T0(cmd_send,7,res_recv)<0)return -1;	
	}
	
	if(res_recv[0]!=0x90 || res_recv[1]!=0)return -3;	 
	
	memcpy(cmd_send,"\x00\xd6\x00\x00\x20",5); //写密钥文件
	cmd_send[3]=keytype==QY?0x20:0;
	memcpy(cmd_send+5,msg,32);

	if(IC_Apdu_T0(cmd_send,37,res_recv)<0 || res_recv[0]!=0x90 || res_recv[1]!=0)return -1;	 

	return 0;
}

char LoadKey_DES_Main(unsigned char add,unsigned char *msg)
{
	if(msg[0] != 8)return -2;
	if(add == 0xff)return -3;
	
	memcpy(cmd_send,"\x80\xd5\x00\x00\x0d\x35\xf0\xf0\x01\x01",10);
	cmd_send[3]=add;	
	cmd_send[4]=5+msg[0];
	memcpy(cmd_send+10,msg+1,msg[0]);

	if(IC_Apdu_T0(cmd_send,10+msg[0],res_recv)<0 || res_recv[4]!=0x90 || res_recv[5]!=0)return -1;
	return 0;
}

char LoadKey_DES_Work(unsigned char main_add,unsigned char work_add,unsigned char *msg)
{
	if(main_add == 0xff)return -3;
	if(msg[0] != 8 && msg[0] != 16 && msg[0] != 24)return -2;

	memcpy(cmd_send,"\x84\xDA\x00\x00\x08",5);
	cmd_send[2]=main_add;
	cmd_send[3]=work_add;
	cmd_send[4]=msg[0];
	memcpy(cmd_send+5,msg+1,msg[0]);

	if(IC_Apdu_T0(cmd_send,5+msg[0],res_recv)<0 || res_recv[4]!=0x90 || res_recv[5]!=0)return -1;
	return 0;
}

char LoadKey_DES(unsigned char add,unsigned char *msg)
{
	unsigned char i=2;

	if(msg[0] != 8 && msg[0] != 16 && msg[0] != 24)return -2;

	memcpy(cmd_send,"\x84\xDA\xff\x00\x08",5);
	cmd_send[3]=add;
	cmd_send[4]=msg[0];
	for(i=0;i<msg[0]>>3;i++)
	{
		des(msg+1+i*8,cmd_send+5+i*8,"13241324",1);//13241324为传输密钥
	}

   	for(i=0;i<2;i++)
	{
		Delay_ms(1);
		res_recv[4]=res_recv[5]=0xff;

		if(IC_Apdu_T0(cmd_send,5+msg[0],res_recv)<0)return -1;
		Delay_ms(1);

		if(res_recv[0] == 0x6A && res_recv[1] == 0x83)
		{
			if(IC_Apdu_T0("\x00\xA4\x00\x00\x02\x3F\x00",7,res_recv) < 0)return -1;
			res_recv[4]=res_recv[5]=0xff;
			if(IC_Apdu_T0("\x80\xd5\x00\xff\x0d\x35\xf0\xf0\x01\x01\x31\x33\x32\x34\x31\x33\x32\x34",18,res_recv)<0)return -1;
			if(res_recv[0] == 0x4A && res_recv[1] == 0xFA && res_recv[2] == 0x28 && res_recv[3] == 0x20 && res_recv[4] == 0x90 && res_recv[5] == 0)	
				continue;
			else
				return -3;
		}
		if(res_recv[4] == 0x90 && res_recv[5] == 0)return 0;
	}
	return -4;
}

/*
函数说明：SM1算法加解密
参数：		add:使用目标地址密钥加解密
			msg:待加解密数据，长度严格限制
			length:16的倍数
			flag:加解密模式，加密：MODE_E，解密：MODE_D
返回：		0：正确执行
			-1：APDU错误
			-2：加解密类型错误
*/
char Encrpty_SM1(unsigned char add,unsigned char *msg,unsigned char length,unsigned char flag)
{

	if(length != length>>4<<4)
	{
		memset(msg,0,length);
		return -4; //长度必须为16的倍数
	}
	
	memcpy(cmd_send,"\x80\x40\x00\x01\x10",5); 
	cmd_send[3]=add;	  //密钥地址	
	if(flag==MODE_E)	  //加密
	{
		cmd_send[2]=0;
	}
	else if(flag==MODE_D) //解密
	{
		cmd_send[2]=0x80;
	}
	else
	{
		memset(msg,0,length);
		return -2;
	}
	for(i=0;i<length>>4;i++)	//分段加密数据
	{
		memcpy(cmd_send+5,msg+(i<<4),16);

		if(IC_Apdu_T0(cmd_send,21,res_recv)<0 || res_recv[16]!=0x90 || res_recv[17]!=0)
		{
			memset(msg,0,length);
			return -1;
		}
		else
		{
			memcpy(msg+(i<<4),res_recv,16);
		}
	}
	return 0;	
}




/*
函数说明：SM4算法加解密
参数：		add:使用目标地址密钥加解密
			msg:待加解密数据，长度严格限制
			length:16的倍数
			flag:加解密模式，加密：MODE_E，解密：MODE_D
返回：		0：正确执行
			-1：APDU错误
			-2：加解密类型错误
*/
char Encrpty_SM4(unsigned char add,unsigned char *msg,unsigned char length,unsigned char flag)
{

	if(length != length>>4<<4)
	{
		memset(msg,0,length);
		return -4; //长度必须为16的倍数
	}
	
	memcpy(cmd_send,"\x80\x41\x00\x01\x10",5); 
	cmd_send[3]=add;	  //密钥地址	
	if(flag==MODE_E)	  //加密
	{
		cmd_send[2]=0;
	}
	else if(flag==MODE_D) //解密
	{
		cmd_send[2]=0x80;
	}
	else
	{
		memset(msg,0,length);
		return -2;
	}
	for(i=0;i<length>>4;i++)	//分段加密数据
	{
		memcpy(cmd_send+5,msg+(i<<4),16);
		
		if(IC_Apdu_T0(cmd_send,21,res_recv)<0 || res_recv[16]!=0x90 || res_recv[17]!=0)
		{
			memset(msg,0,length);
			return -1;
		}
		else
		{
			memcpy(msg+(i<<4),res_recv,16);
		}
	}
	return 0;	
}


/*
函数说明： SM2 加解密函数
参数：
		add:密钥地址
		msg:加密数据，可以为空字段，得预留length+96的空间
		length：数据长度，加密时可以为0，但不能超过70，否则返回结果会溢出
						  解密时最小为96，最大不能超过200
		flag：加解密模式 MODE_E加密  MODE_D解密	MODE_G 签名  MODE_V 签名认证
返回：
		0：正确执行
		-1：	
		-2:length长度错误	
*/
char Encrpty_SM2(unsigned char add,unsigned char *msg,unsigned char *length,unsigned char flag)
{
	if(flag==MODE_E)//加密
	{	
		if(*length>32)return -2;

		memcpy(cmd_send,"\x80\x43\x00\x02\x10",5);
		cmd_send[3]=add*2+2;
		cmd_send[4]=*length;
		memcpy(cmd_send+5,msg,*length);

		if(IC_Apdu_T0(cmd_send,5+*length,msg)<0 || msg[cmd_send[4]+0x60]!=0x90 || msg[cmd_send[4]+0x61]!=0)
		{
			memset(msg,0,*length);
			*length=0;
			return -1;
		}
		*length=cmd_send[4]+0x60;
	}
	else if(flag==MODE_D)
	{
		if(*length<96 || *length>200)return -2;

		memcpy(cmd_send,"\x80\x44\x00\x01\x70",5);
		cmd_send[3]=add*2+1;
		cmd_send[4]=*length;
		memcpy(cmd_send+5,msg,*length);

		if(IC_Apdu_T0(cmd_send,5+*length,msg)<0 || msg[cmd_send[4]-0x60]!=0x90 || msg[cmd_send[4]-0x5f]!=0)
		{
			memset(msg,0,*length);
			*length=0;
			return -1;
		}
		*length=cmd_send[4]-0x60;
	}
	else if(flag==MODE_G)
	{
		if(*length>32)return -2;

		memcpy(cmd_send,"\x80\x45\x00\x01\x10",5);
		cmd_send[3]=add*2+1;
		cmd_send[4]=*length;
		memcpy(cmd_send+5,msg,*length);

		if(IC_Apdu_T0(cmd_send,5+*length,msg)<0 || msg[0x40]!=0x90 || msg[0x41]!=0)
		{
			memset(msg,0,*length);
			*length=0;
			return -1;
		}
		*length=0x40;
	}
	else if(flag==MODE_V)
	{
		if(*length>132)return -2;

		memcpy(cmd_send,"\x80\x46\x00\x02\x50",5);
		cmd_send[3]=add*2+2;
		cmd_send[4]=*length;
		memcpy(cmd_send+5,msg,*length);

		if(IC_Apdu_T0(cmd_send,5+*length,msg)<0 || msg[0]!=0x90 || msg[1]!=0)
		{
			memset(msg,0,*length);
			*length=0;
			return -1;
		}
		*length=0;	
	}
	else 
	{
		memset(msg,0,*length);
		*length=0;
		return -2;
	}
	return 0;

	#undef	Pos
	#define Pos	128
}

char Encrpty_DES(unsigned char add,unsigned char *msg,unsigned char flag)
{
	if (msg[0] != msg[0] & 0xf8)
	{
		return -2;
	}
	if (flag != MODE_E && flag != MODE_D)
	{
		return -3;
	}

	memcpy(cmd_send,"\x80\xD8\x10\x00\x08",5);
	cmd_send[2]=flag==MODE_E?0x10:0x11;
	cmd_send[3]=add;
	cmd_send[4]=msg[0];
	memcpy(cmd_send+5,msg+1,msg[0]);
	memset(msg + 1,0, msg[0]);
	if(IC_Apdu_T0(cmd_send,5+msg[0],msg+1)<0 || msg[msg[0]+1] != 0x90 || msg[msg[0]+2] != 0)
	{  
		memset(msg+1,0,msg[0]);
		msg[0]=0;
		return -1;
	}
	return 0;
}
													 
char GetKey_SM2(unsigned char *msg)
{
	if(IC_Apdu_T0("\x80\x42\x00\x00\x00",5,msg)<0 || msg[0x60]!=0x90 || msg[0x61]!=0)
	{
		memset(msg,0,0x60);
		return -3;
	}
	return 0;
}													 

char Encrpty_SM3(unsigned char *msg,unsigned char length)
{
	if(length == 0)return -2;

	memcpy(cmd_send,"\x80\xcc\x00\x04",4);
	cmd_send[4]=length;
	memcpy(cmd_send+5,msg,length);

	if(IC_Apdu_T0(cmd_send,length+5,msg)<0 || msg[32]!=0x90 || msg[33]!=0)
	{
		memset(msg,0,length);
		return -1;
	}
	return 0;
}

