/**************��ʱ����ʼ������****************
���ã�	  2ms��ʱ����ʼ�� ��ʱ��0
��ڲ�������
���ز�������
**********************************/
void InitTime(unsigned long us)

{
    TMOD |= 0x01;
	TH0 = (65536-3686)/256;
    TL0 = (65536-3686)%256;
	//TH0 = (65536-120000*us/221184)/256;		 //2ms
	//TL0 = (65536-120000*us/221184)%256;		 //�Ƿ�Ϊ3686��������
	ET0 = 1;
	TF0=0;
	TR0 = 1; //��ʱ����ʼ��ʱ
}

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
		beep = ~beep;
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
	long int num = 0; 						//ѭ��������¼�������Ľ������ֵΪ304������unsigned int����
	unsigned int random1,random2,temp;				//�����1�������2��ͨ����λȷ���������ÿһλ
	unsigned char high1,high2,low1,low2;			//ȡ��ʱ����ֵ�������������������ȶ�

	random1 = random2 = 0x0000;
	temp = 0x0001;
	high1 = high2 = TH2;
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
	
	if(!OutOfOrder() || !seqflag)
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
