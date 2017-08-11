#include "IC.h"


static unsigned char idata PCB;
static unsigned char ICtype;



void IC_SafeCard(void);
char IC_Response(unsigned char *res,unsigned short len);
char IC_CommandSend(unsigned char *command,unsigned char len);


#if 1
char IC_ReceiveChar(unsigned char *receive_data,unsigned short wait_time)
{
	FLAG temp={0};
	unsigned char idata ver=0;
	
	*receive_data=0;
	//TR0 = 0; //定时器0 关闭 wy 17.8.7 修改

	IO_IC=1;
	OpenDelay(wait_time);
	RST_IC=1;		
	while(IO_IC)
	{
		if(!GetDelayFlag())
		{
			CloseDelay();
			//TR0 = 1;//定时器0 开启 wy 17.8.7 修改
			return -2;
		} 
	}
	if(IO_IC)
	{
		TR2=0;
		//TR0 = 1;//定时器0 开启 wy 17.8.7 修改
		return -1;
	} 

	TR2=0;	
	TF2=0;
	RCAP2H=0xff;
	RCAP2L=0x42;
	TH2=0xff;
	TL2=0xdd;
	TR2=1;
	while(!TF2);
	TF2=0;
	if(IO_IC)
	{
		TR2=0;
		//TR0 = 1;//定时器0 开启 wy 17.8.7 修改
		return -1;
	} 
	

	while(!TF2);
	TF2=0;

	
	temp.data_bit.bit_1=IO_IC;	
	ver+=temp.data_bit.bit_1;
	
	while(!TF2);
	TF2=0;
	
	temp.data_bit.bit_2=IO_IC; 
	ver+=temp.data_bit.bit_2;

	while(!TF2);
	TF2=0;
	
	temp.data_bit.bit_3=IO_IC;	
	ver+=temp.data_bit.bit_3;

	while(!TF2);
	TF2=0;
	
	temp.data_bit.bit_4=IO_IC;	 
	ver+=temp.data_bit.bit_4;

	while(!TF2);
	TF2=0;
	
	temp.data_bit.bit_5=IO_IC;	 
	ver+=temp.data_bit.bit_5;  

	while(!TF2);
	TF2=0;
	
	temp.data_bit.bit_6=IO_IC;	
	ver+=temp.data_bit.bit_6;

	while(!TF2);
	TF2=0;
	
	temp.data_bit.bit_7=IO_IC;	 
	ver+=temp.data_bit.bit_7;

	while(!TF2);
	TF2=0;
	
	temp.data_bit.bit_8=IO_IC;	 
	ver+=temp.data_bit.bit_8;

  while(!TF2);
	TF2=0;
	if(IO_IC != (ver&1))
	{
		TR2=0;
		//TR0 = 1;//定时器0 开启 wy 17.8.7 修改
		return -3;
	} 
	TR2=0;
	
	*receive_data=temp.data_byte;
	IO_IC=1; 

	//TR0 = 1;//定时器0 开启 wy 17.8.7 修改
	return 0;
} 

#endif

#if 0
char IC_ReceiveChar(unsigned char *receive_data,unsigned short wait_time)
{
	FLAG temp={0},begin={0};
	unsigned char idata ver=0;
	
	*receive_data=0;

	IO_IC=1;
	OpenDelay(wait_time);		
	while(IO_IC)
	{
		if(!GetDelayFlag())
		{
			CloseDelay();
			return -2;
		} 
	}
	TR2=0;	
	TF2=0;
	RCAP2H=0xff;
	RCAP2L=0xa1;
	TH2=0xff;
	TL2=0xd9;
	TR2=1;
	while(!TF2);
	TF2=0;
	begin.data_bit.bit_2=IO_IC;


	while(!TF2);
	TF2=0;
	while(!TF2);
	TF2=0;

	temp.data_bit.bit_1=IO_IC;
	
	while(!TF2);
	TF2=0;
	while(!TF2);
	TF2=0;
	temp.data_bit.bit_2=IO_IC;

	while(!TF2);
	TF2=0;
	while(!TF2);
	TF2=0;
	temp.data_bit.bit_3=IO_IC;

	while(!TF2);
	TF2=0;
	while(!TF2);
	TF2=0;
	temp.data_bit.bit_4=IO_IC;

	while(!TF2);
	TF2=0;
	while(!TF2);
	TF2=0;
	temp.data_bit.bit_5=IO_IC;

	while(!TF2);
	TF2=0;
	while(!TF2);
	TF2=0;
	temp.data_bit.bit_6=IO_IC;

	while(!TF2);
	TF2=0;
	while(!TF2);
	TF2=0;
	temp.data_bit.bit_7=IO_IC;

	while(!TF2);
	TF2=0;
	while(!TF2);
	TF2=0;
	temp.data_bit.bit_8=IO_IC;

   	while(!TF2);
	TF2=0;
	while(!TF2);
	TF2=0;
	if(IO_IC == temp.data_bit.bit_1+temp.data_bit.bit_2+temp.data_bit.bit_3+temp.data_bit.bit_4+temp.data_bit.bit_5+temp.data_bit.bit_6+temp.data_bit.bit_7+temp.data_bit.bit_8+1)
	{} 
	TR2=0;

	*receive_data=temp.data_byte;
	IO_IC=1; 

	return 0;
}
#endif


char IC_SendChar(unsigned char send_data,unsigned short wait_time)
{
	FLAG temp;
	unsigned char idata ver=0;

	temp.data_byte=send_data;
	//TR0 = 0;//定时器0 关闭 wy 17.8.7 修改

	IO_IC=1;
	OpenDelay(wait_time);	
	while(!IO_IC)
	{
		if(!GetDelayFlag())
		{
			CloseDelay();
			//TR0 = 1;//定时器0 关闭 wy 17.8.7 修改
			return -1;
		}
	}	

	TR2=0;	
	TF2=0;
	TH2=RCAP2H=0xff;
	TL2=RCAP2L=0x42;//定时104us,原来是3f,加上误差

	IO_IC=0;
	TR2=1;
	while(!TF2);
	TF2=0;

	IO_IC=temp.data_bit.bit_1;
	ver+=temp.data_bit.bit_1;
	while(!TF2);
	TF2=0;

	IO_IC=temp.data_bit.bit_2;
	ver+=temp.data_bit.bit_2;
	while(!TF2);
	TF2=0;

	IO_IC=temp.data_bit.bit_3;
	ver+=temp.data_bit.bit_3;
	while(!TF2);
	TF2=0;

	IO_IC=temp.data_bit.bit_4;
	ver+=temp.data_bit.bit_4;
	while(!TF2);
	TF2=0;

	IO_IC=temp.data_bit.bit_5;
	ver+=temp.data_bit.bit_5;
	while(!TF2);
	TF2=0;

	IO_IC=temp.data_bit.bit_6;
	ver+=temp.data_bit.bit_6;
	while(!TF2);
	TF2=0;

	IO_IC=temp.data_bit.bit_7;
	ver+=temp.data_bit.bit_7;
	while(!TF2);
	TF2=0;

	IO_IC=temp.data_bit.bit_8;
	ver+=temp.data_bit.bit_8;
	while(!TF2);
	TF2=0;

	IO_IC=ver&1;
	while(!TF2);
	TF2=0;

	IO_IC=1;
	TR2=0;

	//TR0 = 1;//定时器0 开启 wy 17.8.7 修改
	return 0;
} 

void IC_SafeCard(void)
{
	VCC_IC=1;
	RST_IC=0;
	CLK0_IC=0;
	CLK1_IC=0;
	IO_IC=0;
}

char IC_Reset(unsigned char *res)
{									
	unsigned char idata temp,i,j;
	unsigned char idata res_len;
	unsigned char idata tck;

	res[0]=0;
	TR0 = 0;//定时器0 关闭 wy 17.8.7 修改
	IC_SafeCard();
	Delay_ms(1);
	
	VCC_IC=0;
	IO_IC=1;
	CLK0_IC=1;
	CLK1_IC=1;	
	
	PCB=0x40;																	
	
	Delay_ms(13);//42948个时钟周期延时（40000~45000）	45000=12.57ms
	//RST_IC=1;	

	res_len=0;
	//TS
	if(IC_ReceiveChar(res+res_len++,1000)<0)//(42000+50ms) 42000=11.735ms
	{	
		TR0 = 1;//定时器0 开启 wy 17.8.7 修改
		return -1;
	}
	
	if(res[0]!=0x3b && res[0]!=0x3f)//Pag:8.3.1(26)
	{ 
		TR0 = 1;//定时器0 开启 wy 17.8.7 修改
		return -1;/////////////////////////////
	}
	//T0
	if(IC_ReceiveChar(res+res_len++,2000)<0)//9600ETU
	{
		TR0 = 1;//定时器0 开启 wy 17.8.7 修改
		return -1;
	}
	tck=res[1];
	
		
	if((res[1]&0xf0)==0x60)ICtype=0;
	else if((res[1]&0xf0)==0xe0)ICtype=1;
	else ICtype=2;
	
	for(i=0;i<3;i++)//i=0:TA1~TD1    i=1:TA2~TD2    i=2:TA3~TD3    i>2:ì?3?
	{
		temp=res[res_len-1];//i=0:T0	i=1:TD1		i=2:TD2

		for(j=0;j<4;j++)
		{
			const unsigned char bit_pos[4]={0x10,0x20,0x40,0x80};
			
			if(temp&bit_pos[j])
			{
				if(IC_ReceiveChar(res+res_len++,2000)<0)//Pag:8.4(31)
				{
					TR0 = 1;//定时器0 开启 wy 17.8.7 修改
					return -1;
				}
				
				tck^=res[res_len-1];
				
				if(j==3)continue;
			}
			
			if(j==3)
			{
				i=3;
				break;
			}
		}

		if(i==0 && res[res_len-1]!=0x81)ICtype=2;
		if(i==1 && res[res_len-1]!=0x31)ICtype=2;
	}

		
	for(i=0;i<(res[1]& 0x0F);i++)
	{
		if(IC_ReceiveChar(res+res_len++,2000)< 0) //Pag:8.4(31)	
		{
			TR0 = 1;//定时器0 开启 wy 17.8.7 修改
			return -1;
		}
		tck^=res[res_len-1];
	}
	if(ICtype==1)
	{
		if(IC_ReceiveChar(res+res_len++,2000)<0)//TCK		//Pag:8.4(31)
		{
			TR0 = 1;//定时器0 开启 wy 17.8.7 修改
			return -1;
		}
		if(tck!=res[res_len-1])
		{
			TR0 = 1;//定时器0 开启 wy 17.8.7 修改
			return -13;
		}
			
	}
	
	//ICPower=1;
	TR0 = 1;//定时器0 开启 wy 17.8.7 修改
	return res_len;
} 

char IC_CommandSend(unsigned char *command,unsigned char len)
{
	unsigned char i,j;
	char ret;
	TR0 = 0;// wy 17.8.7 修改
	for(i=0;i<len;i++)
	{
		ETU();
		ETU();
		ETU();
		
		ret=IC_SendChar(command[i],1048);
		if(ret<0)
		{
			for(j=0;j<3;j++)
			{
				ret=IC_SendChar(command[i],1048);
				if(!ret)break;
			}
			if(!ret)break;
		}
	}
	TR0 = 1;// wy 17.8.7 修改
	return ret;
}

char IC_Response(unsigned char *res,unsigned short len)
{
	unsigned short i;
	unsigned char j,error_time=255;
	char ret;
	TR0 = 0;// wy 17.8.7 修改
	for(i=0;i<len;i++)
	{
		for(j=0;j<4;j++)
		{
			ret=IC_ReceiveChar(res+i,1048);
			if(ret<0)
			{
				IO_IC=0;
				ETU();
				ETU_1_4();
				ETU_1_4();
				IO_IC=1;
			}
			else break;
		}
		if(ret<0)
		{
			TR0 = 1;// wy 17.8.7 修改
			return -1;
		}
	}
	TR0 = 1;// wy 17.8.7 修改
	return 0;
}

int IC_Apdu_T0(unsigned char *cmd_send,unsigned short cmd_len,unsigned char *res)
{
	unsigned char tempdata[5]={0,0xc0,0,0,0};
	unsigned char templen=0;
	unsigned char INS=0;
	unsigned int count=5;
	int res_len;

	res[0]=res[1]=0xff;

	templen=cmd_send[4];
	INS=cmd_send[1];
	TR0 = 0;// wy 17.8.7 修改
	if(IC_CommandSend(cmd_send,5))
	{
		TR0 = 1;// wy 17.8.7 修改
		return -1;
	}
	
	while(1)
	{
		res_len=0;
		if(IC_Response(res,1))
		{
			TR0 = 1;// wy 17.8.7 修改
			return -2;
		}

		if(res[0]==INS)
		{
			if(cmd_len!=5)
			{
				if (IC_CommandSend(cmd_send + count, cmd_len - 5))
				{
					TR0 = 1;// wy 17.8.7 修改
					return -1;
				}
				cmd_len=5;
				continue;
			}
			else
			{
				res_len=0;
				if (IC_Response(res, 2 + templen))
				{
					TR0 = 1;// wy 17.8.7 修改
					return -2;
				}
				res_len=templen+2;
			}
		}
		else if(res[0]==~INS+1)
		{
			if(cmd_len!=5)
			{
				if (IC_CommandSend(cmd_send + count, 1))
				{
					TR0 = 1;// wy 17.8.7 修改
					return -1;
				}
				cmd_len--;
				count++;
			}
			continue;
		}
		else if(res[0]==0x60)
		{}
		else if(res[0]==0x61)
		{
			if (IC_Response(res + res_len + 1, 1))
			{
				TR0 = 1;// wy 17.8.7 修改
				return -2;
			}
			res_len++;
			tempdata[4]=res[res_len];
			if (IC_CommandSend(tempdata, 5))
			{
				TR0 = 1;// wy 17.8.7 修改
				return -1;
			}
			templen=tempdata[4];
			INS = tempdata[1];
			continue;
		}
		else if(res[0]==0x6c)
		{
			if (IC_Response(res + res_len + 1, 1))
			{
				TR0 = 1;// wy 17.8.7 修改
				return -2;
			}
			res_len++;
			cmd_send[4] = res[res_len];
			templen = res[res_len];
			if (IC_CommandSend(cmd_send, 5))
			{
				TR0 = 1;// wy 17.8.7 修改
				return -1;
			}
			continue;
		}
		else
		{
			if (IC_Response(res + res_len + 1, 1))
			{
				TR0 = 1;// wy 17.8.7 修改
				return -2;
			}
			res_len++;
			res_len++;
		}
		break;
	}
	TR0 = 1;// wy 17.8.7 修改
	return res_len;
}