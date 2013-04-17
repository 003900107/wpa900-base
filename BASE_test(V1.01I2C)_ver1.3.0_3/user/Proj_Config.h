#ifndef _PROJ_CONFIG_H_
#define _PROJ_CONFIG_H_
/*__________________________________________________________________

						 工程配置文件
	 此处注明工程名称和令号，程序修改者和实验者
	 所有跟非标工程有关的macro定义放置该文件下，注明使用方法与含义
	                                                           --- 袁博
__________________________________________________________________*/

#define PSX600 0x01
#define EYEWIN 0x02
//#define MONITOR EYEWIN
#define ASDU6EN //103规约ASDU6对时使能
//#define PT103EN	//103规约使能
#define TCPMODBUSEN	 //TCP-MODBUS规约使能
#define TCPMODBUS_ADDR 0x01	   //TCPMODBUS地址
#define NonElectro_Nbr 4 //the total number of Non-Electrical Trip DI
#define TRIPDIMAP 7,8,9,10
#define TRIPDOMAP 2,2,2,2	   //trip nbr sequence
//#define FRONTDITRIP	//前面个别点做非电量开入 
//#define REARDITRIP		//后面个别点做非电量开入
#define SPECIALDITRIP

#define VERSION 12

//#define TEST
#define TEST_PARTS
//#define TEST_RTC
#define PRINTF_TEST
#define CAN_APP

#endif