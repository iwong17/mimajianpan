#include "UART.h"
#include "MCU.h"
#include "8820.h"
#include "EEPROM.h"

#define KEY_CLOSE	0x80
#define KEY_BEEP_ON	0x01

sbit OPEN=P3^5;		   //����Ի�����

unsigned char xdata nowkey=0;


unsigned char xdata cmd[256];
unsigned char xdata cmdlength;
unsigned char xdata keyflag;
unsigned char xdata keycode[16];
unsigned char xdata keylength;
unsigned char xdata maxkeylen;

unsigned char xdata destoryflag=0;	   //����Իٱ�־
unsigned char xdata recoverflag=1;	   //����Իٻָ���־
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
						  
	do				 //������Ч���ݣ�֪���յ�����ͷ
	{
		while(!RI)	//δ���յ�����ʱ   
		{						  //������ӿ���ʱ�跴�����õĺ�������ɨ�����    
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

		case 0x08://дEEPROM����
		TwoToOne(cmd+2,cmdlength-2);
		if(cmd[2] >= 30)//�жϵ�ַ�Ƿ���ȷ
		{
			SendChar('f');
			break;
		}
		cmd[1] = cmd[2]+20;//cmd[1]�洢ʵ�ʼ�¼��ַ
		cmd[2] = (cmdlength - 4)/2;
		if(cmd[2] > 30 || cmd[2] == 0)//�жϼ�¼�����Ƿ���ȷ
		{
			SendChar('f');
			break;
		}
		Write_Flash(cmd[1],cmd+2);
		SendChar('k');
		break;

		case 0x09://��EEPROM����
		TwoToOne(cmd+2,2);
		if(cmd[2] >= 30)//�жϵ�ַ�Ƿ���ȷ
		{
			SendChar('f');
			break;
		}
		Read_Flash(cmd[2]+20,cmd+100);
		if(cmd[100] > 30 || cmd[100] == 0)//�жϼ�¼�����Ƿ���ȷ
		{
			SendChar('f');
			break;
		}
		cmd[99] = cmd[100];//�ݴ��¼���ȣ��򳤶�ҲҪ�����
		cmd[100] <<= 1;//��¼���ȸ�Ϊ��ֺ󳤶�
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

		case 0x4e://'N' ��������SM4��Կ
		TwoToOne(cmd+2,34);

		if(LoadKey_SM4(cmd[2],cmd+3) < 0)//����SM4��Կ
		{
			SendChar(0x66);
			break;
		}
		nowkey = cmd[2];
		SendChar('k');	
		break;
		case 0x47://'G'���ĸ���SM4��Կ
		TwoToOne(cmd+2,36);

		if(Encrpty_SM4(cmd[2],cmd+4,16,MODE_D) < 0)//���ܵõ�SM4��Կ����
		{
			SendChar(0x66);
			break;
		}
		if(LoadKey_SM4(cmd[3],cmd+4) < 0)//����SM4��Կ
		{
			SendChar(0x66);
			break;
		}
		nowkey = cmd[3];
		SendChar('k');
		break;
		case 0x48://'H' ����SM4��Կ
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

		case 0x59://	���������루SM4��
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

		case 0x5a:// ��������
		TwoToOne(cmd+2,cmdlength-2);
		cmd[100] = (cmdlength-2)>>1;//����ʵ�ʳ���
		memset(cmd+2+cmd[100],0,(16-cmd[100])&15);
		cmd[100] = (cmd[100]+15)&0xf0;//���������ݳ���
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

		case 0x80://������ȷ��ȡ����
		Putbytespi(cmd[2]);
		keyflag=3;	
		keylength=0;
		break;


		default:
		break;
	}
	#undef OVERTIME	
}

/*���������*/
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

		destoryflag=1;	//�Ѳ��
		recoverflag=0;	//���δ�ָ�
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
			SendChar(keydata+' ');//ע�ⲻҪ������ֵ��ͻ��
			Putbytespi(0x23);//����ȡ����������
			if(keylength == maxkeylen)Putbytespi(0x20);//���������ּ�������
			return;
		}
		if(keydata == 0x3a && keylength)
		{
			SendChar('K');
			SendChar(' ');
			keylength--;
			if(keylength == 0)Putbytespi(0x24);//������ȡ����������
			Putbytespi(0x1f);//�������ּ�������
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

				if(cmd[1] == 0x46)//DES ����
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
				else//SM4����
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
	
	destoryflag=0;	//δ���
	recoverflag=1;	//����ѻָ�

	if(Init8820(MODE_I) < 0)
	{
		Erase();
		if(Init8820(MODE_I) < 0)return -1;
	}
	Erase();
}


/*EEPROM���䣺
				0������Ի�
				1���״������Զ����¼���оƬ
				2���������Ի�*/
void main(void)
{
	InitDelay();  //��ʼ����ʱ����
	VCC_IC=0;//������оƬ�ϵ磬��ֹ����оƬ��һ�θ�λ����
	Delay_ms(50); //��ʱ50���룬Ϊ��ϵͳ���ȶ�
	InitSC(115200);	//����ͨѶ������Ϊ115200
	InitEEPROM();

	//��ʼ������
	cmdlength=0;
	keylength=0;
	keyflag=0;
	maxkeylen=6;  

	
	memset(cmd,0,5);//Ϊ�˽������ȷ������֮ǰ������
	Read_Flash(2,cmd);//������Իٱ�־
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
				destoryflag=1;	//�Ѳ��
				recoverflag=0;	//���δ�ָ�
				Write_Flash(0,"\xA5\xA5\xA5\xA5\xA5");	
			}
		}
	}
	memset(cmd,0,5);//Ϊ�˽������ȷ������֮ǰ������
	Read_Flash(0,cmd);//������Իٱ�־
	if(enabledestory == 1 && cmd[0] == 0xA5 && cmd[1] == 0xA5 && cmd[2] == 0xA5 && cmd[3] == 0xA5 && cmd[4] == 0xA5) //�ϸ��жϲ����������Ϊδ���
	{
		destoryflag=1;//�ò��
		recoverflag=0;//�����ָ�
	}
	else
	{
		destoryflag=0;//����
		recoverflag=1;//�ûָ�
	
		if(Init8820(MODE_R)<0)//8820��λ����Ϊ��ͨ�Բ��ԣ����ⶼ��������·������
		{
			//������5����ʾ
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
			if(Init8820(MODE_I)<0)//8820��֤		//����������������
			{
				//������2����ʾ
				Putbytespi(KEY_BEEP_ON);
				Delay_ms(500);
				Putbytespi(KEY_BEEP_ON);
				Delay_ms(500);
			}
	
			memset(cmd,0,5);
			Read_Flash(1,cmd);//��ȡ�Ƿ��һ�ν�����򣬵�һ�ν�����Ҫ��8820����������8820�ĸ�ʽ
			if(cmd[0] == 0xff)//δ�������EEPROMӦΪ0xFF 
			{
				Erase();
				memset(cmd,0,5);
				Write_Flash(1,cmd);//��ɲ�����Ӧ����־���㣬�´ξͲ���Ҫ��������	
			}
			Putbytespi(0x80); //������̴���״̬
			Putbytespi(KEY_BEEP_ON);//������̷�������һ��
		}
	}

	

	while(1)
	{
		RecDeal();
	}
}