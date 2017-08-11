#ifndef _ERRORCODE_H_
#define _ERRORCODE_H_


	  #define 	CMDOK	0x00	//正确返回
	  #define 	Err_1	0x01	//写EEPROM错误
	  #define 	Err_2	0x02	//读EEPROM错误
	  #define 	Err_3	0x03	//命令参数错误
	  #define 	Err_4	0x04	//已拆机，请恢复
	  #define 	Err_5	0x05	//此条命令的命令体不能为空
	  #define 	Err_6	0x06	//主密钥编号超出范围
	  #define 	Err_7	0x07	//下载DES密钥错误
	  #define 	Err_8	0x08	//DES加密错误
	  #define 	Err_9	0x09	//密钥类型错误

	  #define 	Err_10	0x0a	//密钥类型跟密钥长度不一致
	  #define 	Err_11	0x0b	//不存在主密钥，请先下载
	  #define 	Err_12	0x0c	//工作密钥编号超出范围
	  #define 	Err_13	0x0d	//不存在工作密钥，请先下载
	  #define 	Err_14	0x0e	//设置的超时时间太长
	  #define 	Err_15	0x0f	//密码键盘已超时，自动关闭
	  #define 	Err_16	0x10	//随机数长度错误
	  #define	Err_17	0x11	//取随机数失败
	  #define	Err_18	0x12  	//SM2公钥错误（输入的公钥长度不正确）
	  #define	Err_19	0x13  	//SM2密钥下载错误
	  #define	Err_20	0x14  	//SM2加密错误
	  #define	Err_21	0x15  	//SM4密钥下载错误
	  #define	Err_22	0x16  	//SM4加密错误
	  #define	Err_23	0x17  	//临时密钥不存在或没有对应的临时密钥
	  #define	Err_24	0x18  	//ANSI账号位数不足
	  #define	Err_25	0x19  	//加密模式错误
	  #define	Err_26	0x1a  	//最小密码长度错误
	  #define	Err_27	0x1b  	//最大密码长度错误（包括最大密码长度小于最小密码长度）
	  #define	Err_28	0x1c  	//输入的密码长度小于最小输入长度
	  #define	Err_29	0x1d  	//加密模式错误
	  #define	Err_30	0x1e  	//初始化失败
	  #define	Err_31	0x1f  	//密码键盘已拆机（针对所有命令）
	  #define	Err_32	0x20  	//RSA公钥错误(输入的公钥长度不正确)
	  #define	Err_33	0x21  	//RSA密钥下载错误
		#define Err_34  0x22		//RSA加密错误
		#define Err_35  0x23		//取PS的随机数失败
		#define Err_36  0x24		//RSApadding错误
		#define Err_37  0x25
#endif