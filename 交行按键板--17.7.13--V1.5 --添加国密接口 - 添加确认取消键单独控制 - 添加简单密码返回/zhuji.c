#include "UART.h"
#include "8820.h"
#include "EEPROM.h"
#include "config.h"
/**************�������ʾ����****************
���ã�	  �������ʾ
��ڲ�������
���ز�������
**********************************/
void Display(unsigned char num)
{
    chose0_7=0x00;
	chose9 = chose8 = 0;

	if(flag & 1) LED = ledcode[num];	   //��ʾ��Ӧλ�ŵ������
	else LED=0xbf;

    switch(num)			   //ѡͨλ�ţ�����ʾ��ѡͨ��Ϊ����Ӱ
	{
    case 0:
		    chose0 = 1;	   //1��λ�����ѡͨ
			break;
		case 1:
		    chose1 = 1;		//2��λ�����ѡͨ
			break;
		case 2:
		    chose2 = 1;	   //3��λ�����ѡͨ
			break;
		case 3:
		    chose3 = 1;	   //4��λ�����ѡͨ
			break;
		case 4:
		    chose4 = 1;	   //5��λ�����ѡͨ
			break;
		case 5:
		    chose5 = 1;	   //6��λ�����ѡͨ
			break;
		case 6:
		    chose6 = 1;	   //7��λ�����ѡͨ
			break;
		case 7:
		    chose7 = 1;	   //8��λ�����ѡͨ
			break;
		case 8:
		    chose8 = 1;	   //9��λ�����ѡͨ
			break;	 
		case 9:
		    chose9 = 1;	   //10��λ�����ѡͨ
			break;
		default:		   //���󣬲�ѡͨ�κ������
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

/**************����������****************
���ã�	  ��������
��ڲ�������
���ز�������
****************************************/
void Beep(void)
{
	unsigned int data i,j;
	for(i = 0; i < 255; i++)     //����
	{
		for(j = 0;j < 44;j++);   //��Ƶ��22MΪ17KHz
		{
			beep = ~beep;
			if (RI) return; //RIʱ�˳���������Ӧ����
		}
	}
}

/*************�жϺ���*******************		   //	 ���̵�������			   ˳��Ϊ
���ã�	  �ж��������ڵ��������Ƿ���ʾ����		   //		 chose0 				  1
��ڲ�����a:���жϵļ� b:���ڼ� c�����ڼ� 		   // chose9 chose8 chose7 	    	2 3	4
          ��ֻ��һ�����ڼ�������һ��������a		   // chose6 chose5 chose4 		    5 6	7
���ز���������ʾ�����򷵻�1						   // chose3 chose2 chose1 		    8 9	10
****************************************/		   //
bit Comp(unsigned char a,unsigned char b,unsigned char c)				   //  �����������ʮ��������˳��
												   //	��Ӳ�����������˳���෴
{												   //   ���������ط�Ӧ����Ӧ�޸�
	if(a == (b+1)%10) return 1;		//��1		   
	if(a == (c+1)%10) return 1;		
	if(a == (b+9)%10) return 1;		//С1
	if(a == (c+9)%10) return 1;
	return 0;
}

/*************������**************
���ã�	  ����0~9��������������˳����Ӳ��
          �����˳���෴
��ڲ�������
���ز��������ɹ������������򷵻�1
****************************************/
bit OutOfOrder(void)

{
	unsigned char i[10] = {0,0,0,0,0,0,0,0,0,0},k;	//9��forѭ���ĳ�ʼֵ��һ��ѭ������
	unsigned int res_recv1[10];

	long int num = 0; 						//ѭ��������¼�������Ľ������ֵΪ304������unsigned int����
	unsigned int random1, random2;				//�����1�������2��ͨ����λȷ���������ÿһλ
	//unsigned int temp;
	//unsigned char high1,high2,low1,low2;			//ȡ��ʱ����ֵ�������������������ȶ�

	random1 = random2 = 0x0000;
	/*
	temp = 0x0001;
	high1 = high2 = TH2; //wy 17.8.7�޸� ԭ��ΪTH2
	low1 = low2 = TL1;

	for(k = 0;k < 8;k++)					//8��ѭ��ȷ��random1��16λ��1����0
	{
		if(high1&0x80)						//ȡTH2�����λ
			random1 |= temp;				//����random1�����λ

		high1 = high1<<1;					//ȡ�θ�λ
		temp = temp<<1;

		if(low1&0x01)						//ȡTL1�����λ
			random1 |= temp;				//����random1�ĵڶ�λ

		low1 = low1>>1;						//ȡTL1�Ĵε�λ
		temp = temp<<1;
	}										//ѭ��8�Σ���TH2��TL1�������random1

	temp = 0x0001;

	for(k = 0;k < 8;k++)					//random2��ȡ�÷���ͬ�ϣ�����TH2��TL1�Ĳ���λ�ò�ͬʹrandom2!=random1
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
	IC_Apdu_T0("\x00\x84\x00\x00\x08", 5, res_recv1);//����8�ֽڵ������

	random1 = (res_recv1[0] << 8) | res_recv1[1];
	random2 = (res_recv1[2] << 8) | res_recv1[3];
	
	random1 %= 304;	  //��0~65536�������תΪ0~304�������
	random2 %= 304;	  //��0~65536�������תΪ0~304�������

	for(k = 0;k < 5;k++)	   //ȡ����������ַ��Ϊ9��ѭ���ĳ�ʼֵ
	{
	    i[2*k] = maval[random1][k]/16;
		i[2*k+1] = maval[random1][k]%16;
	}

	for(led_key[0] = i[0];led_key[0] < 10;led_key[0]++)			//���chose9�������			//�����߼������ڵ�������92640�ֿ���
	{																							//9��ѭ����������п��ܣ���̫��ʱ��
		for(led_key[1] = i[1];led_key[1] < 10;led_key[1]++)		//���chose8�������			//���Ȼ���Ϊ304��ÿ��304��������󼸸�
		{																						//ÿ��ĵ�һ������maval[][]��
			if(led_key[1] == led_key[0])continue;			//����ǰ��������������ͬ������	//ͨ��ѭ���ҵ���random1���random2����ʵ������
			for(led_key[2] = i[2];led_key[2] < 10;led_key[2]++)	//���chose7�������			//Ϊ���㷨����ٷ���˳����Ӳ��˳���෴����ͼ
			{																					//	 0			 chose9
				if((led_key[2] == led_key[0])||(led_key[2] == led_key[1]))continue;				// 1 2 3  chose8 chose7 chose6  ��ѭ���е�led_key[0]
				if(Comp(led_key[2],led_key[1],led_key[0]))continue;//������Χ��ʾ����������		// 4 5 6  chose5 chose4 chose3  ���Ƕ�Ӧchose0�������
				for(led_key[3] = i[3];led_key[3] < 10;led_key[3]++)	//���chose6�������		// 7 8 9  chose2 chose1 chose0 	���Ƕ�Ӧchose9�������
				{
					if((led_key[3]==led_key[0])||(led_key[3]==led_key[1])||(led_key[3]==led_key[2]))continue; //����ǰ��������������ͬ������
					if(Comp(led_key[3],led_key[2],led_key[3]))continue;//������Χ��ʾ����������
					for(led_key[4] = i[4];led_key[4] < 10;led_key[4]++)	//���chose5�������
					{
						if((led_key[4]==led_key[0])||(led_key[4]==led_key[1])||(led_key[4]==led_key[2])||(led_key[4]==led_key[3]))continue;//����ǰ��������������ͬ������
						if(Comp(led_key[4],led_key[1],led_key[4]))continue;//������Χ��ʾ����������
						for(led_key[5] = i[5];led_key[5] < 10;led_key[5]++)	//���chose4�������
						{
							if((led_key[5]==led_key[0])||(led_key[5]==led_key[1])||(led_key[5]==led_key[2])||(led_key[5]==led_key[3])||(led_key[5]==led_key[4]))continue;//����ǰ��������������ͬ������
							if(Comp(led_key[5],led_key[2],led_key[4]))continue;//������Χ��ʾ����������
							for(led_key[6] = i[6];led_key[6] < 10;led_key[6]++) //���chose3�������
							{
								if((led_key[6]==led_key[0])||(led_key[6]==led_key[1])||(led_key[6]==led_key[2])||(led_key[6]==led_key[3])||(led_key[6]==led_key[4])||(led_key[6]==led_key[5]))continue;//����ǰ��������������ͬ������
								if(Comp(led_key[6],led_key[3],led_key[5]))continue;	//������Χ��ʾ����������
								for(led_key[7] = i[7];led_key[7] < 10;led_key[7]++)	//���chose2�������
								{
									if((led_key[7]==led_key[0])||(led_key[7]==led_key[1])||(led_key[7]==led_key[2])||(led_key[7]==led_key[3])||(led_key[7]==led_key[4])||(led_key[7]==led_key[5])||(led_key[7]==led_key[6]))continue;//����ǰ��������������ͬ������
									if(Comp(led_key[7],led_key[4],led_key[7]))continue;//������Χ��ʾ����������
									for(led_key[8] = i[8];led_key[8] < 10;led_key[8]++)	//���chose1�������
									{
										if((led_key[8]==led_key[0])||(led_key[8]==led_key[1])||(led_key[8]==led_key[2])||(led_key[8]==led_key[3])||(led_key[8]==led_key[4])||(led_key[8]==led_key[5])||(led_key[8]==led_key[6])||(led_key[8]==led_key[7]))continue;//����ǰ��������������ͬ������
										if(Comp(led_key[8],led_key[5],led_key[7]))continue;//������Χ��ʾ����������
										led_key[9]=45-led_key[0]-led_key[1]-led_key[2]-led_key[3]-led_key[4]-led_key[5]-led_key[6]-led_key[7]-led_key[8];
										if(Comp(led_key[9],led_key[8],led_key[6]))continue;//������Χ��ʾ����������	
										num++;
										if(1+random2 == num) return 1; //�ҵ���random1���random2������ֵ
									}i[8] = 0;	//��ٷ�Ӧ��0~9ѭ�����ڵ�һ�δ�i[]��ʼ��9�������ڶ����Ժ��Ҫ��0��ʼ9����
								}i[7] = 0;
							}i[6] = 0;
						}i[5] = 0;
					}i[4] = 0;
				}i[3] = 0;
			}i[2] = 0;
		}i[1] = 0;if(led_key[0] == 9)i[0] = 0; //�����ϵ�304��304��֮��92640������������´ӵ�0�鿪ʼ�������������
	}	
	return 0;//δ���������������������ܣ���������һ	
}

/***********��ʾ��ֵƥ�亯��************
���ã�	  �ı��������ʾ˳�򣬽���ʾ��ֵ
          ����ȫ�ֱ���
��ڲ�������
���ز�������
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
 	
	for(i = 0;i < 10;i++)				//���������ʾ����ֵ����ڶ�Ӧλ��
	{
	    switch(led_key[9-i])			//Ϊ�˽�������Ӳ��˳��һ�£�ԭ��led_key[i]����
		{
		    case 0:
			    ledcode[i] = 0xc0;		//��ʾ0
				break;
			case 1:
			    ledcode[i] = 0xf9;		//��ʾ1
				break;
			case 2:
			    ledcode[i] = 0xa4;		//��ʾ2
				break;
			case 3:
			    ledcode[i] = 0xb0;		//��ʾ3
				break;
			case 4:
			    ledcode[i] = 0x99;		//��ʾ4
				break;	  
			case 5:
			    ledcode[i] = 0x92;		//��ʾ5
				break;
			case 6:
			    ledcode[i] = 0x82;		//��ʾ6
				break;
			case 7:
			    ledcode[i] = 0xf8;		//��ʾ7
				break;
			case 8:
			    ledcode[i] = 0x80;		//��ʾ8
				break;
			case 9:
			    ledcode[i] = 0x90;		//��ʾ9
				break;
			default:
			    ledcode[i] = 0x86;		//��ʾ������Ϣ��E��
				break;
		}
	}
}

void RecDeal(void)
{
	#define OVERTIME1 10000
	unsigned int data  time;
						  
	do				 //������Ч���ݣ�֪���յ�����ͷ
	{
		while(!RI)	//δ���յ�����ʱ   
		{						  //������ӿ���ʱ�跴�����õĺ�������ɨ�����    
			Destory_Deal();
			GetCode();//����keyflagΪ1ʱ�Ž���
			if (flag & 8)//wy 17.7.13 �޸� ����
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
		//Putbytespi(0xff);//��ͨ����  //wy 17.7.13 �޸�
		flag=0xff;//wy 17.7.13 �޸�
		seqflag=0;//wy 17.7.13 �޸�	
		Sequence();//wy 17.7.13 �޸�
		beepflag=0xff;//wy 17.7.13 �޸�
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
			//Putbytespi(0xff); //wy 17.7.13 �޸�
			flag=0xff;//wy 17.7.13 �޸�
			seqflag=0;//wy 17.7.13 �޸�	
			Sequence();//wy 17.7.13 �޸�
			beepflag=0xff;//wy 17.7.13 �޸�
			keyflag=1;	
		}
		else if(cmd[3] == 0x33)
		{
			//Putbytespi(0x80); //wy 17.7.13 �޸�
            flag=0x80;//wy 17.7.13 �޸�
			seqflag=0;//wy 17.7.13 �޸�	
			keyflag = 0;
			Sequence();//wy 17.7.13 �޸�
			beepflag=0xff;//wy 17.7.13 �޸�			
		}
		else
		{}
		break;
		case 0x52://'R'
		//Putbytespi(0x80); //wy 17.7.13 �޸�
		flag=0x80;//wy 17.7.13 �޸�
		seqflag=0;//wy 17.7.13 �޸�	
		keyflag = 0;
		Sequence();//wy 17.7.13 �޸�
		beepflag=0xff;//wy 17.7.13 �޸�	
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
			//Putbytespi(0xff); //wy 17.7.13 �޸�
			flag=0xff;//wy 17.7.13�޸�
			seqflag=0;//wy 17.7.13�޸�	
			Sequence();//wy 17.7.13�޸�
			beepflag=0xff;//wy 17.7.13�޸�
			
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
		
			//Putbytespi(0xff); //wy 17.7.13 �޸�
			flag=0xff;//wy 17.7.13�޸�
			seqflag=0;//wy 17.7.13�޸�	
			Sequence();//wy 17.7.13�޸�
			beepflag=0xff;//wy 17.7.13�޸�
			
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
		//Putbytespi(cmd[2]);//case 0x13://��ȷ�ϡ�ȡ��������״̬���� //wy 17.7.13 �޸�
		flag|=2;//wy 17.7.13�޸�
		keyflag=3;	
		keylength=0;
		break;

		default:
		break;
	}
	#undef OVERTIME1	
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

	if(RI == 1 || keyflag == 0)return; //wy 17.8.10�޸� ԭ��Ϊ if(RI == 0 && keyflag == 0) �����Ӧ���⣬Ӧ��������Ӧ��λ��

 	keydata=Scankey();//wy 17.7.13�޸� ��flag����λΪ1�򷵻�Ϊ��ֵ��� ��flag����λΪ0�򷵻�Ϊ*
	P4 = 0xef;
	if (P4 != 0xef)//�а�������
	{
		while (!RI && P4 != 0xef);
	}

	if(RI == 1 || (keydata ==0 || keydata == '*'))return; //wy 17.8.10�޸� ԭ��Ϊ if(RI == 0 && (keydata ==0 || keydata == '*')) �����Ӧ���⣬Ӧ��������Ӧ��λ��

	if(keyflag == 1)
	{
		if(keydata == 0x3B)
		{	
			SendChar(keydata);
			//Putbytespi(0x80); //wy 17.7.13 �޸�
			flag=0x80;  //wy 17.7.13�޸�
			seqflag=0;	//wy 17.7.13�޸�
			Sequence(); //wy 17.7.13�޸�
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
			//Putbytespi(0x23);//����ȡ���������� //wy 17.7.13 �޸�
			beepflag|=0x40;//wy 17.7.13�޸�
			if(keylength == maxkeylen)
			//Putbytespi(0x20);//���������ּ������� //wy 17.7.13 �޸�
			beepflag&=~0x01;//wy 17.7.13�޸�
			return;
		}
		if(keydata == 0x3a && keylength)
		{
			SendChar('K');
			SendChar(' ');
			keylength--;
			if(keylength == 0)
				//Putbytespi(0x24);//������ȡ���������� //wy 17.7.13 �޸�
			    beepflag&=~0x40;//wy 17.7.13�޸�
			//Putbytespi(0x1f);//�������ּ������� //wy 17.7.13 �޸�
			    beepflag|=0x01;//wy 17.7.13�޸�
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

				//Putbytespi(0x80); //wy 17.7.13 �޸�
				flag=0x80;//wy 17.7.13�޸�
				seqflag=0;//wy 17.7.13�޸�	
				Sequence();//wy 17.7.13�޸�
				
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
	TR0 = 0; //��ʱ��0 �ر� wy 17.8.7 �޸�

	if(Write_Flash(0,"\x5a\x5a\x5a\x5a\x5a") < 0)return -2;	 
	
	destoryflag=0;	//δ���
	recoverflag=1;	//����ѻָ�

	if(Init8820(MODE_I) < 0)
	{
		Erase();
		if(Init8820(MODE_I) < 0)return -1;
	}
	Erase();

	TR0 = 1;//��ʱ��0 ���� wy 17.8.7 �޸�
	return 0;
}
/**************����ɨ�躯��****************
���ã�	  ɨ����̣����ض�Ӧ����ܼ�ֵ
��ڲ�������
���ز�����������Ӧ����ܵļ�ֵ�����ض���
*******************************************/
unsigned char Scankey(void)
{
  unsigned char data keycode = 0;
	unsigned int data i=10000;

	if(flag & 0x07)//0~9 ȷ�� ȡ��
	{
	    //KEYPORT = 0xe0;  //��������
		P1 |= 0xf0;//��3λΪ�� ����  //wy 17.7.14�޸� 
		P4 = 0x00;//����λΪ��,����//wy 17.7.14�޸�
		if((P1 & 0xf0) != 0xf0)  //�м�����
		{
			while(i--) if(RI) return 0;
			if((P1 & 0xf0) != 0xf0)
			{
			    keycode = KEYPORT;   //ȡ������ֵ
				//KEYPORT = 0x1f;	  //��������
	            P1 &= 0x1f;//�������� ����λ���� //wy 17.7.14�޸�
				P4 = 0xef;//wy ����λ����17.7.14�޸�
				 if(P4 != 0xef)  //��������
							keycode |= KEYPORT; //ȡ����������ֵ
			}
		}

		seq = 1;// P3^4 ��Ϊ����� ����
		if (seq == 0)//wy 17.7.14�޸�
		{
			i = 1000;
			while (i--);
			while (!RI && seq == 0); //̧�ּ��		
			seqint = 1;
		}

		 if(flag & 1)
		{
			switch(keycode)	  //��ֵƥ��
			{
				case 0xde:
					if(beepflag & 1)Beep();	
					return led_key[9]+0x30;	 //���ص�ǰ������Ӧ�������ֵ��ΪASC��
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
					if (RI) return 0; //RIʱ�˳���������Ӧ����
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
			    	;	//δ����������ֵ
			}								
		}
		if(flag & 2)
		{
			if(keycode == 0xd7)
			{
				if(beepflag & 0x40)Beep();	
				return 0x3a;//ȡ��
			}
			if(keycode == 0x77)
			{
				if(beepflag & 0x02)Beep();	
				return 0x3b;//ȷ��
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
	return '*';		 //����δ����ʱ���Կ�ֵ����
}



/**************��ʱ����ʼ������****************
���ã�	  2ms��ʱ����ʼ�� ��ʱ��0
��ڲ�������
���ز�������
**********************************/
void InitTime(unsigned long us)
{
    TMOD |= 0x01; //���ö�ʱ��0 16λ
	//TH0 = (65536-3686)/256;
    //TL0 = (65536-3686)%256;
	TH0 = (65536-120000*us/221184)/256;		 //2ms
	TL0 = (65536-120000*us/221184)%256;		 //�Ƿ�Ϊ3686��������
	ET0 = 1; //��ʱ��0�ж�����
	TR0 = 1; //��ʱ����ʼ��ʱ
}

/**************2ms��ʱ���жϺ���****************
���ã�	  ����2ms��ʱ�жϣ�������ʾʮ�������
          һ���ж���ʾһ������ܣ�Ƶ��50Hz
��ڲ�������
���ز�������
**********************************/
void Timer0() interrupt 1 using 1	 //2ms�жϺ���			 //////��keyflag����ʱ��number���л��Ƿ��������ʾ������������
{
    static unsigned char num=0;		 //�����λ��
	  static unsigned char number=0;
	  unsigned int i = 10000;
    TH0 = (65536-3686)/256;
    TL0 = (65536-3686)%256;
	  number++;
	  
	  if(flag & 1)						
	  {
	    if(number > 9) number = 0;		//0~9ѭ��
		  num=number;
	  }
	  else
	  {
	    if(number>254)			//��ʱ�ã���ʾ����ʱ��������ܵļ��ʱ��
		  {
		    number=0;
			  num++;
			  if(num > 9)num = 0;
		  }
	  }
	  Display(num);			//������Ӧλ�ŵ�����ܣ�keyflag=1ʱ20msһѭ��
}

//void SeqInterrupt(void) interrupt 2 //wy 17.7.14�޸�
//{
	//seqint=1;
//}

void DotInterrupt(void) interrupt 0 //wy 17.7.14�޸� ԭ����6
{	
	dotint=1;
}


void LagInterrupt(void) interrupt 2 //wy 17.7.14�޸� ԭ����7
{	
	lagint=1;
}

/*EEPROM���䣺
				0������Ի�
				1���״������Զ����¼���оƬ
				2���������Ի�*/
void main(void)
{
	InitDelay();    //��ʼ����ʱ���� ��ʱ��2
	InitTime(2);    //��ʱ����ʼ�� ��ʱ��0 //wy 17.7.14�޸�
	VCC_IC=0;		//������оƬ�ϵ磬��ֹ����оƬ��һ�θ�λ����
	Delay_ms(50);   //��ʱ50���룬Ϊ��ϵͳ���ȶ�
	InitSC(115200);	//����ͨѶ������Ϊ115200
	InitEEPROM();
	
	EX0=1;IT0=1;//�ⲿ�ж�0 ������Ϊ0 dotint//wy 17.7.14�޸�
	EX1=1;IT1=1;//�ⲿ�ж�1 ������Ϊ2 lagint//wy 17.7.14�޸�
	
	Sequence();//wy 17.7.14�޸�
	//Beep();				  //�����Լ���

	

	//��ʼ������
	cmdlength=0;
	keylength=0;
	keyflag=0;
	maxkeylen=6;  

    seqint=0;//wy 17.7.14�޸�
	dotint=0;//wy 17.7.14�޸�
	lagint=0;//wy 17.7.14�޸�
	flag=0;//wy 17.7.14�޸�
	
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
			Beep();//wy 17.7.13�޸�
			Delay_ms(500);
			Beep();//wy 17.7.13�޸�
			Delay_ms(500);
			Beep();//wy 17.7.13�޸�
			Delay_ms(500);
			Beep();//wy 17.7.13�޸�
			Delay_ms(500);
			Beep();//wy 17.7.13�޸�
			Delay_ms(500);
		}
		else
		{	
			if(Init8820(MODE_I)<0)//8820��֤		//����������������
			{
				//������2����ʾ
				Beep();//wy 17.7.13�޸�
				Delay_ms(500);
				Beep();//wy 17.7.13�޸�
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
			//Putbytespi(0x80); //������̴���״̬ wy 17.7.13�޸�
			flag=0x80;//wy 17.7.13�޸�
			seqflag=0;//wy 17.7.13�޸�	
			Sequence();//wy 17.7.13�޸�
			Beep();//wy 17.7.13�޸�
		}
	}
	while(1)
	{
		RecDeal();
		
	}
}