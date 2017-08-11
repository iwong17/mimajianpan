#ifndef _8820_H_
#define _8820_H_

	
	#include "IC.h"
	#include "DES.h"
	#include "UART.h"
	
	
	//头文件的包含一定要在EXTERN之前
	#ifdef _8820_C_
	#define EXTERN 
	#else
	#define EXTERN extern
	#endif
	
		#define EN_ERR
		#define SM1  	1
		#define SM2  	2
		#define SM3  	3
		#define SM4		4
		
		
		#define EK		0x20
		#define AK		0x21
		#define SK		0x22

		#define DK		1
		#define QX		2
		#define QY		3

		#define MODE_E	1
		#define MODE_D	2
		#define MODE_G	3
		#define MODE_V	4
		#define MODE_R	5
		#define MODE_I 	6
	

		
		EXTERN unsigned char xdata temp[256];


		char Init8820(unsigned char mode);
		char Erase(void);
		char LoadKey_SM1(unsigned char add,unsigned char *msg,unsigned char keytype);
		char LoadKey_SM4(unsigned char add,unsigned char *msg);
		char LoadKey_SM2(unsigned char add,unsigned char *msg,unsigned char keytype);
		char LoadKey_DES_Main(unsigned char add,unsigned char *msg);
		char LoadKey_DES_Work(unsigned char main_add,unsigned char work_add,unsigned char *msg);
		char LoadKey_DES(unsigned char add,unsigned char *msg);
		char Encrpty_SM1(unsigned char add,unsigned char *msg,unsigned char length,unsigned char flag);
		char Encrpty_SM4(unsigned char add,unsigned char *msg,unsigned char length,unsigned char flag);
		char Encrpty_SM2(unsigned char add,unsigned char *msg,unsigned char *length,unsigned char flag);
		char Encrpty_DES(unsigned char add,unsigned char *msg,unsigned char flag);
		char GetKey_SM2(unsigned char *msg);
		char Encrpty_SM3(unsigned char *msg,unsigned char length);
	
	#undef EXTERN

#endif