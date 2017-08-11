#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <STC89C51RC_RD_PLUS.H>
#include <intrins.h>        /* use _nop_() function */ 
#include <stdio.h>


//头文件的包含一定要在EXTERN之前
#ifdef _EEPROM_C_
#define EXTERN 
#else
#define EXTERN extern
#endif


typedef     unsigned char	INT8U;		/* 8 bit 无符号整型  */
typedef     unsigned int    INT16U;     /* 16 bit 无符号整型 */

/* 定义命令 */
//#define STANDBY_Command								0x00        /*  待机模式   */
#define READ_AP_and_Data_Memory_Command				0x01        /*  字节读数据存储区   */
#define PROGRAM_AP_and_Data_Memory_Command          0x02        /*  字节编程数据存储区 */
#define SECTOR_ERASE_AP_and_Data_Memory_Command     0x03        /*  扇区擦除数据存储区 */

/* 定义常量 */
#define ERROR   -1
#define OK      0

/* 定义Flash 操作等待时间 */
//30M以下0，24M以下1，20M以下2，12M以下3，6M以下4，3M以下5，2M以下6，1M以下7

#define WAIT_TIME        0x01

#define DEBUG_STC89C_LE58RD	//后面"+"编译器不认

#ifdef DEBUG_STC89C_LE58RD                        //STC89C58RD+,  89LE58RD+
        #define DEBUG_AP_Memory_Begin_Sector_addr		0x0000
        #define DEBUG_AP_Memory_End_Sector_addr         0x7e00
        #define DEBUG_AP_Memory_End_Byte_addr           0x7fff

        #define DEBUG_Data_Memory_Begin_Sector_addr     0x8000
#endif
#ifdef DEBUG_STC89C_LE52RC                        //STC89C52RC,        89LE52RC
        #define DEBUG_AP_Memory_Begin_Sector_addr		0x0000
        #define DEBUG_AP_Memory_End_Sector_addr			0x1e00
        #define DEBUG_AP_Memory_End_Byte_addr			0x1fff

        #define DEBUG_Data_Memory_Begin_Sector_addr		0x2000
#endif

#define USED_BYTE_QTY_IN_ONE_SECTOR		32	 //总共59*1=59条记录
#define SIZE_SECTOR                     32	//每条记录的大小,长度+30内容+校验



EXTERN volatile INT8U xdata protect_buffer[USED_BYTE_QTY_IN_ONE_SECTOR];


INT8U flash_read(INT16U begin_addr, INT16U counter, INT8U array[]);
INT8U flash_write(INT16U begin_addr, INT16U counter, INT8U array[]);
void ReadEEPROM(INT16U begin_addr, INT16U counter, INT8U array[]);
void EraseEEPROM(INT16U begin_addr);
void InitEEPROM(void);

#define Read_Flash(x,y) flash_read(DEBUG_Data_Memory_Begin_Sector_addr+\
                        ((unsigned int)(x)*SIZE_SECTOR/USED_BYTE_QTY_IN_ONE_SECTOR)*0x0200+\
						((unsigned int)(x)%(USED_BYTE_QTY_IN_ONE_SECTOR/SIZE_SECTOR))*SIZE_SECTOR,\
						SIZE_SECTOR,y)
#define Write_Flash(x,y) flash_write(DEBUG_Data_Memory_Begin_Sector_addr+\
                        ((unsigned int)(x)*SIZE_SECTOR/USED_BYTE_QTY_IN_ONE_SECTOR)*0x0200+\
						((unsigned int)(x)%(USED_BYTE_QTY_IN_ONE_SECTOR/SIZE_SECTOR))*SIZE_SECTOR,\
						SIZE_SECTOR-1,y)


#undef EXTERN
#endif