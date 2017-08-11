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


/* 打开 ISP,IAP 功能 */
void ISP_IAP_enable(void)
{
	EA	=	0;	/* 关中断 */
	ISP_CONTR	=	ISP_CONTR & 0x18;       /* 0001,1000 */
	ISP_CONTR	=	ISP_CONTR | WAIT_TIME;
	ISP_CONTR	=	ISP_CONTR | 0x80;       /* 1000,0000 */
}

/* 关闭 ISP,IAP 功能 */
void ISP_IAP_disable(void)
{
	ISP_CONTR	=	ISP_CONTR & 0x7f;	/* 0111,1111 */
	ISP_TRIG	=	0x00;

	ISP_CMD		=	0x00;
	ISP_ADDRH	=	0xff;
	ISP_ADDRL	=	0xff;

	EA			=   1;                	/* 开中断 */
}


/* 字节读 */
//含验证校验码
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

	//增加校验码  (改写)
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


//同时计算校验码并写入
INT8U flash_write(INT16U begin_addr, INT16U counter, INT8U array[])
{
	INT16U	i	=	0;
	INT16U	in_sector_begin_addr	=	0;
	INT16U	sector_addr	=	0;
	INT16U	byte_addr	=	0;
	unsigned char temp;

	/* 判是否是有效范围,此函数不允许跨扇区操作 */
	if((counter+1) > USED_BYTE_QTY_IN_ONE_SECTOR)		  //对于本程序有漏洞，多一个校验位及主密钥个数位或者无漏洞，严格限制array
		return ERROR;
	in_sector_begin_addr =        begin_addr & 0x01ff;         /* 0000,0001,1111,1111 */
	/* 假定从扇区的第0个字节开始，到USED_BYTE_QTY_IN_ONE_SECTOR-1个字节结束,后面部分不用,程序易编写	*/
	if( (in_sector_begin_addr + counter+1) > USED_BYTE_QTY_IN_ONE_SECTOR )
		return ERROR;

	/* 将该扇区数据 0 - (USED_BYTE_QTY_IN_ONE_SECTOR-1) 字节数据读入缓冲区保护 */
	sector_addr		=	(begin_addr & 0xfe00); 	/* 1111,1110,0000,0000; 取扇区地址		*/
	byte_addr		=   sector_addr;			/* 扇区地址为扇区首字节地址			 	*/

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

	

	/* 将要写入的数据写入保护缓冲区的相应区域,其余部分保留 */
	for(i = 0; i < counter; i++)
	{
		protect_buffer[in_sector_begin_addr] = array[i];
		in_sector_begin_addr++;
	}
	//增加校验码
	temp=array[0];
	for(i=0;i<(counter-1);i++)
	{
		temp^=array[i+1];
	}
	protect_buffer[in_sector_begin_addr] =temp;

	/* 擦除 要修改/写入 的扇区 */
	ISP_ADDRH	=	(INT8U)(sector_addr >> 8);
	ISP_ADDRL	=	0x00;
	ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
	ISP_CMD		=	ISP_CMD	|	SECTOR_ERASE_AP_and_Data_Memory_Command;	/* 0000,0011 */

	ISP_TRIG	=	0x46;        /* 触发ISP_IAP命令 */
	ISP_TRIG	=	0xb9;        /* 触发ISP_IAP命令 */
	_nop_();

	/* 将保护缓冲区的数据写入 Data Flash, EEPROM */
	byte_addr	=   sector_addr;			/* 扇区地址为扇区首字节地址	*/
	for(i = 0; i< USED_BYTE_QTY_IN_ONE_SECTOR; i++)
	{
		/* 写一个字节 */
		ISP_ADDRH	=	(INT8U)(byte_addr >> 8);
		ISP_ADDRL	=	(INT8U)(byte_addr & 0x00ff);
		ISP_DATA	=	protect_buffer[i];
		ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
		ISP_CMD		=	ISP_CMD	|	PROGRAM_AP_and_Data_Memory_Command;		/* 0000,0010 */

		ISP_TRIG	=	0x46;        /* 触发ISP_IAP命令 */
		ISP_TRIG	=	0xb9;        /* 触发ISP_IAP命令 */
		_nop_();

		/* 读回来 */
		ISP_DATA	=	0x00;

		ISP_CMD		=	ISP_CMD	&	0xf8;        /* 1111,1000 */
		ISP_CMD		=	ISP_CMD	|	READ_AP_and_Data_Memory_Command;	/* 0000,0001 */

		ISP_TRIG	=	0x46;        /* 触发ISP_IAP命令 */
		ISP_TRIG	=	0xb9;        /* 触发ISP_IAP命令 */
		_nop_();

		/*  比较对错 */
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

	ISP_TRIG	=	0x46;        /* 触发ISP_IAP命令 */
	ISP_TRIG	=	0xb9;        /* 触发ISP_IAP命令 */
	_nop_();
	ISP_IAP_disable();
}