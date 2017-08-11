#include "UART.h"
#include "8820.h"
#include "EEPROM.h"
#include "ErrorCode.h"

#define KEYPORT  P2

#define tempkey  33

sbit OPEN=P3^5;		   //����Ի�����
sbit GreenLED=P1^6;		//�̵�
sbit RedLED=P1^7;		//���


unsigned char xdata cmd[256];//���յ��������
unsigned char xdata cmdlength;//���յ��������
unsigned char xdata command;

unsigned char xdata keyflag;  //���̿��ű�־
unsigned char xdata keycode[16]; //��������뻺��
unsigned char xdata keylength;	//�������볤��
unsigned char xdata maxkeylen;	//����������볤��

unsigned char xdata nowkey;
unsigned char xdata EncryptMode;
unsigned char xdata account[8];

unsigned char xdata destoryflag=0;	   //����Իٱ�־
unsigned char xdata recoverflag=1;	   //����Իٻָ���־
unsigned char xdata enabledestory=0;   //0�����������Ի٣�1���������Ի�

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

/*����ֺ�������ֽںϲ���һ���ֽ�*/
char TwoToOne(unsigned char *msg,unsigned char length)
{
	unsigned char data i;

	if(length & 1)return -1;//���ȱ���Ϊż��

	for(i=0;i<length>>1;i++)
		msg[i]=(msg[i*2]<<4)&0xf0|msg[i*2+1]&0x0f;
	return 0;
}

/*�Ƚ��������ݴ��Ƿ���ȣ�����Ϊ0ʱ���*/
char Equal(unsigned char *msg1,unsigned char *msg2,unsigned char length)
{
	while(length--)
		if(msg1[length] ^ msg2[length])
			return -1; 

	return 0;
}

/*��λ���򣬽����ݲ�λ��8�ı������ȣ�ĩβ��0x00������λ���������output����*/
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

//��EEPROM add�ڵ���Կ�ӽ��ܳ���Ϊlengthh����Ϊmsg�����ݣ������Ľ���Ծɴ��msg��
char Encrypt(unsigned char add,unsigned char *msg,unsigned char length,unsigned char flag)
{								
	unsigned char data key[26];

	if(length ^ (length & 0xf8))return -1;//���ȱ���Ϊ8�ı���

	Read_Flash(add,key);//��ȡ��Կ

	if(key[0] == 8)//��ԿΪ��DES
	{
		while(length)
		{	 
			length -= 8;
			des_1(msg+length,msg+length,key+1,flag);
			
		}
	}
	else if(key[0] == 16)//��ԿΪ˫DES
	{
		while(length)
		{	 
			length -= 8;
			des_2(msg+length,msg+length,key+1,flag);
		}
	}
	else if(key[0] ==24)//��ԿΪ��DES
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

/*�ر�������̣�����ָʾ��*/
void CloseKey(void)
{
	keyflag=0;
	keylength=0;	
	GreenLED=1;
	RedLED=1;
}																			 


void RecDeal(void)
{
	#define OVERTIME 10000	//�������������ֽ�֮��Ľ��ճ�ʱʱ��
	unsigned short data  time;
	unsigned char data i;
	//signed char testdata=0;
						  
	do				 //������Ч���ݣ�ֱ���յ�����ͷ
	{
		while(!RI)	//δ���յ�����ʱ   
		{						  //������ӿ���ʱ�跴�����õĺ�������ɨ�����    
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
		keyflag = 6;//����������
		reflag=0;
		keylength = 0;
	}
	else if(cmd[0] == 0x81)
	{
		RedLED=0;
		Play_voice(2);
    keyflag = 6;//����������
		reflag=0;
		keylength = 0;
	}
	else if(cmd[0] == 0x80 || cmd[0]== 0x83)
	{
		CloseKey();
	}
	
	else if(cmd[0] == 0x1b)//1B��������ʴ�����
	{
		cmdlength=1;
		
		while(!duanflag)
		{
			time=OVERTIME;													//����һ���ֽ�
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
				while(!RI && time--);									//����һ������
				if(RI)																//���յ����ݣ�����1b 49 XXXX����
				{												
					RI=0;
					cmd[cmdlength++]=SBUF;
				}
				else																	//���ղ������� ����1b 49 ������
				{
					duanflag=1;
				}																			//�ɴ�����49�ֲ�ִͬ����ڣ����жϲ���1b 49��������
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
				if(cmd[2] == '/' && cmd[3]=='4')			//1b [ / 34 8�ֽ���Կ��� 1�ֽ�����Կ��ַ���  +  1b [ / 31 or 32
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

		CloseKey();//ÿ���л������ر��������
		
		//��ʼ����ƥ��
		switch(cmd[1])
		{
			case 0://��ͨ�Բ���
				Warning(1);
			SendChar(0x99);
			break;
			case 0x02://Զ������
			SendChar(0xbb);
			Delay_ms(6000);
			ISP_CONTR=0x60;	//��λ
			break;
			case 0x04://��������Իٹ���
			enabledestory=1;
			memset(cmd,0x5a,5);
			Write_Flash(212,cmd);
			SendChar(0xbb);
			break;
			case 0x06://�رղ���Իٹ���
			enabledestory=0;
			memset(cmd,0xa5,5);
			Write_Flash(212,cmd);
			SendChar(0xbb);
			break;
			case 0x08://�ָ�����
			if(InitDevice() < 0)
				SendChar(0x55);
			else
				SendChar(0xbb);
			break;
			case 0x10://��ȡ��¼
			Read_Flash(cmd[2]*256+cmd[3],cmd+10);
			SendMsg(cmd+10,25);
			break;
			case 0x12://�޸Ĳ�����
			switch(cmd[2])
			{
				case 1:
				InitSC(9600);	//����ͨѶ������Ϊ1200
				break;
				case 2:
				InitSC(2400);	//����ͨѶ������Ϊ2400
				break;
				case 3:
				InitSC(4800);	//����ͨѶ������Ϊ4800
				break;
				case 4:
				InitSC(1200);	//����ͨѶ������Ϊ9600
				break;
				case 5:
				InitSC(115200);	//����ͨѶ������Ϊ115200
				break;

				default:
				InitSC(9600);	//����ͨѶ������Ϊ9600
				break;
			}
			Write_Flash(214,cmd+2);
			SendChar(0xbb);
			break;
		}
	
		if(recoverflag == 0)return;
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////����///////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		switch(cmd[1])
		{
			case '[':
			duanflag=0;
			if(cmd[3] == '1')
			{
				GreenLED=0;
				Play_voice(0);
			  keyflag = 6;//����������
		    keylength = 0;
				break;
			}
			else if(cmd[3] == '2')
			{
				RedLED=0;
				Play_voice(2);
			  keyflag = 6;//����������
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
							keyflag = 6;//����������
							keylength = 0;
						}
						else if(cmd[25] == '2')     //ԭ����cmd[26],0318 TM�޸�
						{
							RedLED=0;
							Play_voice(2);
							keyflag = 6;//����������
							keylength = 0;
						}
					}
				}
							
				if(cmd[12] > 0x09)//����Կ��ַ������Կ��ַ����
				{
					SendChar(0x55);
					break;
				}
				Read_Flash(211,cmd+200);//��ȡ����Կ�Ƿ��ʼ����

				IsInit = 200+(cmd[12]&0xf8?1:0);
				if(cmd[IsInit] & (1 << (cmd[12] & 0x07)))//����Կ����ʼ���˾��������س�ʼ����ֵ
				{		
					memset(cmd+101,'8',8);
					cmd[100]=8;
					Write_Flash(cmd[12],cmd+100);
				}
				
				if(Encrypt(cmd[12],cmd+4,8,0) < 0)//����Կ���ܹ�����Կ		 
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
				//keyflag = 6;//����������
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
			
			case 0x49://'I'   ������������
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
					if(cmd[2] == 0x30)//�����������̵���
					{
						GreenLED=0;
						Play_voice(0);
					}
					else //��������һ�κ����
					{
						RedLED=0;
						Play_voice(2);
					}

					keyflag = 1;//���볤������
					keylength = 0;
				}
				break;
			}
			break;
			
			case 'C':
			if(cmd[2] == 0x30 || cmd[2] == 0x31)
			{
				if(cmd[2] == 0x30)//�����������̵���
				{
					GreenLED=0;
					Play_voice(0);
				}
				else //��������һ�κ����
				{
					RedLED=0;
					Play_voice(2);
				}

				keyflag = 5;//���볤������
				keylength = 0;		
			}
			break;
					
			case 0x46://'F'	  ����������
			if(cmd[2] == 0x30 || cmd[2] == 0x31)
			{
				if(cmd[2] == 0x30)//�����������̵���
				{
					GreenLED=0;
					Play_voice(0);
				}
				else //��������һ�κ����
				{
					RedLED=0;
					Play_voice(2);
				}

				keyflag = 1;//���볤������
				keylength = 0;
				
			}
			break;
	
	    case 0x59:
			case 0x4d://'M'	�޸�����Կ	   //////////////////////////////////////////////////////////////////////////////////////////
			#define IsInit	cmd[255]
			TwoToOne(cmd+2,cmdlength-3);
			if(cmd[2] > 0x0f)//����Կ��ַ
			{
				SendChar(0x55);
				break;
			} 
			Read_Flash(cmd[2],cmd+100);//ȡ���洢������Կ
			Read_Flash(211,cmd+200);//��ȡ����Կ�Ƿ��ʼ����

			IsInit = 200+(cmd[2]&0xf8?1:0);

			if(cmd[IsInit] & (1<< (cmd[2]&0x07)))//����Կ����ʼ����
			{		
				if(Equal("88888888",cmd+3,8) < 0)//������Կ��Ĭ�ϵ���Կ���бȽ�
				{
					SendChar(0x55);
					break;
				}
			}
			else //����Կδ����ʼ��
			{
				if(Equal(cmd+3,cmd+101,cmd[100]) < 0)//������Կ��ԭ����Կ���бȽ�
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
	
	
			case 0x53://'S'	 �޸Ĺ�����Կ	//////////////////////////////////////////////////////////////////////////////////////////
			#define IsInit	cmd[255]
			TwoToOne(cmd+2,cmdlength-3);
			if(cmd[2] > 0x0f || cmd[3] > 7)//����Կ��ַ������Կ��ַ����
			{
				SendChar(0x55);
				break;
			}
			Read_Flash(211,cmd+200);//��ȡ����Կ�Ƿ��ʼ����

			IsInit = 200+(cmd[2]&0xf8?1:0);
			if(cmd[IsInit] & (1 << (cmd[2] & 0x07)))//����Կ����ʼ���˾��������س�ʼ����ֵ
			{		
				memset(cmd+101,'8',8);
				cmd[100]=8;
				Write_Flash(cmd[2],cmd+100);
			}
			cmdlength=(cmdlength-7)/2;//���㹤����Կ����

			Padding(cmd+4,&cmdlength,NULL,2);//������Կ��λ����
			if(Encrypt(cmd[2],cmd+4,cmdlength,0) < 0)//����Կ���ܹ�����Կ		 
			{
				SendChar(0x55);
			}
			else
			{	
				cmd[202+cmd[2]] &= ~(1<<cmd[3]);
				Write_Flash(211,cmd+200);
				cmd[2]=cmd[2]*8+cmd[3]+16;
				cmd[3]=8; 

				Write_Flash(cmd[2],cmd+3);//��¼������Կ
				SendChar(0xbb);
			} 
			#undef IsInit
			break;
	
	
			case 0x41://'A'	   �������Կ	 //////////////////////////////////////////////////////////////////////////////////////////
			#define IsInit	cmd[255]
			TwoToOne(cmd+2,4);
			if(cmd[2] > 0x0f || cmd[3] > 7)//����Կ������Կ����
			{
				SendChar(0x55);
				break;
			}
			nowkey = cmd[2]*8+cmd[3]+16;
			Read_Flash(211,cmd+200);
			if(cmd[202+cmd[2]] & (1<<cmd[3]))//���������Կ�Ѿ�����ʼ���������س�ʼ�����ֵ
			{
				memset(cmd+101,0,8);
				cmd[100]=8;
				Write_Flash(nowkey,cmd+100);
			}
			SendChar(0xbb);
			#undef IsInit
			break;
	
	
			case 0x4e://'N'	   ���ó���		//////////////////////////////////////////////////////////////////////////////////////////
			TwoToOne(cmd+2,4);
			if(cmd[2] > 0x0e || cmd[2] == 0x00)//���õ����볤�ȳ������ֵ
			{
				SendChar(0x55);
				break;
			}
			maxkeylen = cmd[2];
			SendChar(0xbb);
			break;
	
	
			case 0x52://'R'		  �ָ�����	   //////////////////////////////////////////////////////////////////////////////////////////
			memset(cmd,0xff,18);
			Write_Flash(211,cmd);
			memset(cmd+1,0,8);
			cmd[0]=8;
			Write_Flash(16,cmd);//Ĭ��ѡ�е�0�鹤����Կ����0�鹤����Կ�Ȼָ��ɳ�ʼ����ֵ
	
			nowkey=16;
			maxkeylen=6;
			SendChar(0xbb);
			break;

			case 0x56://�汾 CJ908
			#define Ver "Ver5.517Y"
			SendMsg(Ver,sizeof(Ver)-1);
			#undef Ver
			break;
	
			case 0x60:		//���԰���
			RedLED=0;
			GreenLED=0;
			keyflag = 2;
			break;
	
			case 0x62:			//�������к����
			memcpy(cmd,"\x38\x36\x39\x37\x30\x30\x30\x30",8);						//���к�����	cmd
			memcpy(cmd+10,"\x0F\x1E\x2D\x3C\x4B\x5A\x69\x78",8);				//DES������Կ	cmd+10
			if(GetRandomNum(cmd+20,8)<0)																//DES�����Կ	cmd+20
			{
				SendChar(0x55);
				break;
			}
			//SendMsg(cmd+20,8);
			des_1(cmd,cmd+50,cmd+20,1);																	//���к����� cmd+50
			des_1(cmd+20,cmd+60,cmd+10,1);															//��������� cmd+60
			SendMsg(cmd+50,8);
			SendMsg(cmd+60,8);
			break;
			
			default:
			break;
		}
	 }

	
	else //�رռ���	          //////////////////////////////////////////////////////////////////////////////////////////
	{
		CloseKey();	
	}
	
	#undef OVERTIME	
}

/*���������*/
void Destory_Deal(void)
{
	if(enabledestory == 0)return; //���������Ի٣�ֱ������

	OPEN = 1;
	if(RI == 0 && OPEN == 1 && destoryflag == 0 && recoverflag == 1)
	{ 
		Delay_ms(5);
		if(OPEN == 0)return;
		Delay_ms(5);
		if(OPEN == 0)return;

		destoryflag=1;	//�Ѳ��
		recoverflag=0;	//���δ�ָ�
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
	keyval = KEYPORT;							  //ȫ�����ͣ�����Ƿ񰴼�
    if(!RI && (keyval ^ 0x70))	//�а���
	{
	    for(i=0;i<5000;i++)
			if(RI)return 0;	//ȥ����  250us

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
	    return 0;	 //δ����,�Կո������	 

	switch(keyval)
	{	
		case 0xb7:
		    keyval=0x3B;//ȷ��
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
		    keyval=0x3a;//ȡ�� 
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
			return 0;//����
	} 
	return keyval;	
} 

/*�ȴ�����̧�ֶ���*/
void WaitKeyUp(void)
{
	while(KEYPORT^0x70 && !RI);//���ּ��	
}

//������������12λ�˺Ŵ����ANSI98��Ҫ��8�ֽ�����,��cardnum����
void ANSI98(unsigned char *codenum,unsigned char codelen,unsigned char *cardnum)
{
	unsigned char data i;
	unsigned char data temp[16];
	unsigned char data flag = cardnum[0];//glb 2016.3.8 ��� ���ڼ�¼cardnum��0��

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
		/*glb 2016.3.8 �޸ģ�cardnum[0]�ں���ᱻ��д��������һ����ʱ����������*/
		if(flag ^ 0)//�˺���12λ����Ҫ�����8λ���Ѿ���8λ�ľͲ���Ҫ������
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

	if(!RI && keyflag == 1)//���ģ����볤������
	{
		if(keydata > 0x2f && keydata < 0x3a)//����Ϊ���ּ�
		{
			//if(keylength < ((cmd[3] == 0x30 || cmd[3] == 0x32)?maxkeylen:14))//���볤�����ޣ���������Ϊ���ֵ�����볤�Ȳ����ޣ��������󳤶�Ҳ���ܳ���14��
			if(keylength < maxkeylen)
			{
				Warning(1);
				WaitKeyUp();
				keycode[keylength++]= keydata;	
				if(keylength == maxkeylen)//��������ʱ�������Զ��ύ
				{
					if(cmd[1] == 0x49) //ANSI98����
					{
						ANSI98(keycode,keylength,cmd+4);
						keylength = 8;
					}
					else //��ͨ��λ�����
						Padding(keycode,&keylength,cmd+4,1);
					if(Encrypt(nowkey,cmd+4,keylength,1) < 0)//��ͨ����
					{
						CloseKey();
						SendChar(0x55);
						return;
					}
					SendChar(2);
					//SendChar(keylength*2>>4|0x30);   //glb 2016.3.8ע��
					//SendChar(keylength*2&0x0f|0x30); //glb 2016.3.8ע��
					/*glb 2016.3.8 ��� DES���ܷ��س���Ϊ10 98���ܷ��س���Ϊ20*/
					if(cmd[1] == 0x49)
					{
						SendChar('2');
					}
					else SendChar('1');
					SendChar('0');
					/*glb 2016.3.8 �޸Ľ���*/
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
		else if(keydata == 0x3a)//����Ϊȡ����
		{
			Warning(1);
			WaitKeyUp();
			keylength=0;
		}
		else if(keydata == 0x3b)//����Ϊȷ�ϼ�
		{
			if(keylength == 0)return;
			Warning(1);	
			WaitKeyUp();

			if(cmd[1] == 0x49)//ANSI98����
			{
				ANSI98(keycode,keylength,cmd+4);
				keylength = 8;
			}
			else   //��ͨ��λ�����
				Padding(keycode,&keylength,cmd+4,1);
			if(Encrypt(nowkey,cmd+4,keylength,1) < 0)//��ͨ����
			{
				CloseKey();
				SendChar(0x55);
				return;
			}
			SendChar(2);
			//SendChar(keylength*2>>4|0x30);	//glb 2016.3.8ע��
			//SendChar(keylength*2&0x0f|0x30);  //glb 2016.3.8ע��
			/*glb 2016.3.8 ��� DES���ܷ��س���Ϊ10 98���ܷ��س���Ϊ20*/
			if(cmd[1] == 0x49)
			{
				SendChar('2');
			}
			else SendChar('1');
			SendChar('0');
			/*glb 2016.3.8 �޸Ľ���*/
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
		if(keydata == 0x3a)//ȡ����
		{
			Warning(1);
			WaitKeyUp();
			SendChar(0x08);
		}
		if(keydata == 0x3b)//ȷ�ϼ�
		{
			Warning(1);
			WaitKeyUp();
			CloseKey();
			SendChar(0x0d);
		}	
	}
	else if(!RI && keyflag == 3)
	{	  
		if(keydata > 0x2F && keydata < 0x3A)//���ּ�
		{
			Warning(1);
			WaitKeyUp();
			memcpy(cmd+100,"\x02\x03\x69\x00\x00\x03",6);
			cmd[104]=keydata;
			SendMsg(cmd+100,6);
			CloseKey();
			return;
		}
		else if(keydata == 0x3b)//ȡ����
		{
			Warning(1);
			WaitKeyUp();
			memcpy(cmd+100,"\x02\x03\x69\x00\x00\x03",6);
			cmd[104]=0x0d;
			SendMsg(cmd+100,6);
			CloseKey();
			return;	
		}
		else if(keydata == 0x3a)//ȷ�ϼ�
		{
			Warning(1);
			WaitKeyUp();
			memcpy(cmd+100,"\x02\x03\x69\x00\x00\x03",6);
			cmd[104]=0x08;
			SendMsg(cmd+100,6);
			CloseKey();
			return;	
		}
		else if(keydata == 0x3e)//С����
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
		if(keydata == 0x3a)//ȡ����
		{
			if(keylength != 0)
			{	
				Warning(1);
				WaitKeyUp();	
				keylength--;
				//SendChar(0x08);
			}
		}
		if(keydata == 0x3b)//ȷ�ϼ�
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
		if(keydata == 0x3a)//ȡ����
		{
			if(keylength != 0)
			{	
				Warning(1);
				WaitKeyUp();	
				keylength--;
				//SendChar(0x08);
			}
		}
		if(keydata == 0x3b)//ȷ�ϼ�
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
				if(Encrypt_DES(nowkey,cmd+4,8,MODE_E) < 0)//��ͨ����
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
	recoverflag=1;//�ָ����
	enabledestory=0;//���������Ի�
	memset(cmd,0xa5,5);
	Write_Flash(212,cmd);
	Write_Flash(213,cmd);

	return 0;
}


/*EEPROM���䣺
				0~15�������ʴ�����Կ����һ�ֽ�����Կ���ȣ���ʼ����Ϊ8��0x80
				16~143�������ʴ�������Կ����һ�ֽ�����Կ����,��ʼ����Ϊ8��0x00
				144~159����������Կ	����һ�ֽ�����Կ����
				160~207�����й�����Կ����һ�ֽ�����Կ����
				208~210���������кţ�������¼���ʹ�ã���һ�ֽ�Ϊ���ȣ�������кų��Ȳ�����60
				211�������ʴ���ʼ����־����һ���ֽڵ�ÿһλ�ø߱�ʾ��Ӧ������Կ�ǳ�ʼ��״̬
				212:�������Իٱ�־
				213������Ի�״̬
				214��������
				*/
void main(void)
{
	InitDelay();  //��ʼ����ʱ����
	VCC_IC=0;//������оƬ�ϵ磬��ֹ����оƬ��һ�θ�λ����
	Delay_ms(50); //��ʱ50���룬Ϊ��ϵͳ���ȶ�
	InitSC(9600);	//����ͨѶ������Ϊ9600
	InitEEPROM();

	CloseKey();
	maxkeylen=6;
	nowkey=16;
	EncryptMode=0;

	WT_CLK = 1;    //CLK����
	WT_DI = 0;
	Delay_ms(50);	//��ʱ��ȷ������оƬ���ȶ�
	WTH_2L(0xa0140); //оƬ��ʼ��

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
	if(Equal("\x5a\x5a\x5a\x5a\x5a",cmd,5) == 0)//�������Ի�
	{
		enabledestory=1;

		memset(cmd,0,5);
		Read_Flash(213,cmd);
		if(Equal("\x5a\x5a\x5a\x5a\x5a",cmd,5) == 0)//�Ѳ��
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

	//��ȡ������
	memset(cmd,0,5);
	Read_Flash(214,cmd);
	switch(cmd[0])
	{
		case 1:
		InitSC(9600);	//����ͨѶ������Ϊ9600
		break;
		case 2:
		InitSC(2400);	//����ͨѶ������Ϊ2400
		break;
		case 3:
		InitSC(4800);	//����ͨѶ������Ϊ4800
		break;
		case 4:
		InitSC(1200);	//����ͨѶ������Ϊ1200
		break;
		case 5:
		InitSC(115200);	//����ͨѶ������Ϊ115200
		break;

		default:
		InitSC(9600);	//����ͨѶ������Ϊ9600
		break;
	}

	while(1)
	{
		RecDeal();
	}
}
