#define _EEPROM_C_
#include "EEPROM.h"

void InitEEPROM(void)
{
	unsigned char data zero=0;

	if(zero != 0)
	{
		flash_read(0,0,NULL);
		flash_write(0,0,NULL);
		ReadEEPROM(0,0,NULL);
		EraseEEPROM(0);
	}
}


/* �� ISP,IAP ���� */
void ISP_IAP_enable(void)
{
	EA	=	0;	/* ���ж� */
	ISP_CONTR	=	ISP_CONTR & 0x18;       /* 0001,1000 */
	ISP_CONTR	=	ISP_CONTR | WAIT_TIME;
	ISP_CONTR	=	ISP_CONTR | 0x80;       /* 1000,0000 */
}

/* �ر� ISP,IAP ���� */
void ISP_IAP_disable(void)
{
	ISP_CONTR	=	ISP_CONTR & 0x7f;	/* 0111,1111 */
	ISP_TRIG	=	0x00;

	ISP_CMD		=	0x00;
	ISP_ADDRH	=	0xff;
	ISP_ADDRL	=	0xff;

	EA			=   1;                	/* ���ж� */
}


/* �ֽڶ� */
//����֤У����
INT8U flash_read(INT16U begin_addr, INT16U counter, INT8U array[])
{
	unsigned char idata i,temp;
	INT16U	byte_addr;

	byte_addr=begin_addr;
	ISP_IAP_enable();
	for(i = 0; i < counter; i++)
	{
		ISP_ADDRH	=	(INT8U)(byte_addr >> 8);
		ISP_ADDRL	=	(INT8U)(byte_addr & 0x00ff);

		ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
		ISP_CMD		=	ISP_CMD	|	READ_AP_and_Data_Memory_Command;	/* 0000,0001 */

		ISP_TRIG	=	0x46;
		ISP_TRIG	=	0xb9;
		_nop_();

		array[i]	=	ISP_DATA;
		byte_addr++;
	}
	ISP_IAP_disable();

	//����У����  (��д)
	temp=array[0];
	for(i=0;i<(counter-2);i++)
	{
		temp^=array[i+1];
	}
	if(temp==array[counter-1])
		return	OK;
	else
		return	ERROR;
	/*temp=0x00;
	for(i=0;i<array[0]+1;i++)
	{
		temp^=array[i];
	}
	if(temp==array[array[0]+1])
		return	OK;
	else
		return	ERROR;*/


}


//ͬʱ����У���벢д��
INT8U flash_write(INT16U begin_addr, INT16U counter, INT8U array[])
{
	INT16U	i	=	0;
	INT16U	in_sector_begin_addr	=	0;
	INT16U	sector_addr	=	0;
	INT16U	byte_addr	=	0;
	unsigned char temp;

	/* ���Ƿ�����Ч��Χ,�˺������������������ */
	if((counter+1) > USED_BYTE_QTY_IN_ONE_SECTOR)		  //���ڱ�������©������һ��У��λ������Կ����λ������©�����ϸ�����array
		return ERROR;
	in_sector_begin_addr =        begin_addr & 0x01ff;         /* 0000,0001,1111,1111 */
	/* �ٶ��������ĵ�0���ֽڿ�ʼ����USED_BYTE_QTY_IN_ONE_SECTOR-1���ֽڽ���,���沿�ֲ���,�����ױ�д	*/
	if( (in_sector_begin_addr + counter+1) > USED_BYTE_QTY_IN_ONE_SECTOR )
		return ERROR;

	/* ������������ 0 - (USED_BYTE_QTY_IN_ONE_SECTOR-1) �ֽ����ݶ��뻺�������� */
	sector_addr		=	(begin_addr & 0xfe00); 	/* 1111,1110,0000,0000; ȡ������ַ		*/
	byte_addr		=   sector_addr;			/* ������ַΪ�������ֽڵ�ַ			 	*/

	ISP_IAP_enable();
	for(i = 0; i < USED_BYTE_QTY_IN_ONE_SECTOR; i++)
	{
		ISP_ADDRH	=	(INT8U)(byte_addr >> 8);
		ISP_ADDRL	=	(INT8U)(byte_addr & 0x00ff);

		ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
		ISP_CMD		=	ISP_CMD	|	READ_AP_and_Data_Memory_Command;	/* 0000,0001 */

		ISP_TRIG	=	0x46;
		ISP_TRIG	=	0xb9;
		_nop_();

		protect_buffer[i]	=	ISP_DATA;
		byte_addr++;
	}

	

	/* ��Ҫд�������д�뱣������������Ӧ����,���ಿ�ֱ��� */
	for(i = 0; i < counter; i++)
	{
		protect_buffer[in_sector_begin_addr] = array[i];
		in_sector_begin_addr++;
	}
	//����У����
	temp=array[0];
	for(i=0;i<(counter-1);i++)
	{
		temp^=array[i+1];
	}
	protect_buffer[in_sector_begin_addr] =temp;

	/* ���� Ҫ�޸�/д�� ������ */
	ISP_ADDRH	=	(INT8U)(sector_addr >> 8);
	ISP_ADDRL	=	0x00;
	ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
	ISP_CMD		=	ISP_CMD	|	SECTOR_ERASE_AP_and_Data_Memory_Command;	/* 0000,0011 */

	ISP_TRIG	=	0x46;        /* ����ISP_IAP���� */
	ISP_TRIG	=	0xb9;        /* ����ISP_IAP���� */
	_nop_();

	/* ������������������д�� Data Flash, EEPROM */
	byte_addr	=   sector_addr;			/* ������ַΪ�������ֽڵ�ַ	*/
	for(i = 0; i< USED_BYTE_QTY_IN_ONE_SECTOR; i++)
	{
		/* дһ���ֽ� */
		ISP_ADDRH	=	(INT8U)(byte_addr >> 8);
		ISP_ADDRL	=	(INT8U)(byte_addr & 0x00ff);
		ISP_DATA	=	protect_buffer[i];
		ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
		ISP_CMD		=	ISP_CMD	|	PROGRAM_AP_and_Data_Memory_Command;		/* 0000,0010 */

		ISP_TRIG	=	0x46;        /* ����ISP_IAP���� */
		ISP_TRIG	=	0xb9;        /* ����ISP_IAP���� */
		_nop_();

		/* ������ */
		ISP_DATA	=	0x00;

		ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
		ISP_CMD		=	ISP_CMD	|	READ_AP_and_Data_Memory_Command;	/* 0000,0001 */

		ISP_TRIG	=	0x46;        /* ����ISP_IAP���� */
		ISP_TRIG	=	0xb9;        /* ����ISP_IAP���� */
		_nop_();

		/*  �Ƚ϶Դ� */
		if(ISP_DATA != protect_buffer[i])
		{
			ISP_IAP_disable();
			return ERROR;
        }
        byte_addr++;
	}
	ISP_IAP_disable();
	return OK;
}


void ReadEEPROM(INT16U begin_addr, INT16U counter, INT8U array[])
{
	unsigned char i;
	INT16U	byte_addr;
	
	byte_addr=begin_addr*0x200+DEBUG_Data_Memory_Begin_Sector_addr;
	ISP_IAP_enable();
	for(i = 0; i < counter; i++)
	{
		ISP_ADDRH	=	(INT8U)(byte_addr >> 8);
		ISP_ADDRL	=	(INT8U)(byte_addr & 0x00ff);

		ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
		ISP_CMD		=	ISP_CMD	|	READ_AP_and_Data_Memory_Command;	/* 0000,0001 */

		ISP_TRIG	=	0x46;
		ISP_TRIG	=	0xb9;
		_nop_();

		array[i]	=	ISP_DATA;
		byte_addr++;
	} 
	ISP_IAP_disable();
}

void EraseEEPROM(INT16U begin_addr)
{	   
	begin_addr=DEBUG_Data_Memory_Begin_Sector_addr+begin_addr*0x200;
	ISP_IAP_enable();
	ISP_ADDRH	=	(INT8U)(begin_addr >> 8);
	ISP_ADDRL	=	0x00;
	ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
	ISP_CMD		=	ISP_CMD	|	SECTOR_ERASE_AP_and_Data_Memory_Command;	/* 0000,0011 */

	ISP_TRIG	=	0x46;        /* ����ISP_IAP���� */
	ISP_TRIG	=	0xb9;        /* ����ISP_IAP���� */
	_nop_();
	ISP_IAP_disable();
}