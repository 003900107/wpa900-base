#ifndef _PROJ_CONFIG_H_
#define _PROJ_CONFIG_H_
/*__________________________________________________________________

						 ���������ļ�
	 �˴�ע���������ƺ���ţ������޸��ߺ�ʵ����
	 ���и��Ǳ깤���йص�macro������ø��ļ��£�ע��ʹ�÷����뺬��
	                                                           --- Ԭ��
__________________________________________________________________*/

#define PSX600 0x01
#define EYEWIN 0x02
//#define MONITOR EYEWIN
#define ASDU6EN //103��ԼASDU6��ʱʹ��
//#define PT103EN	//103��Լʹ��
#define TCPMODBUSEN	 //TCP-MODBUS��Լʹ��
#define TCPMODBUS_ADDR 0x01	   //TCPMODBUS��ַ
#define NonElectro_Nbr 4 //the total number of Non-Electrical Trip DI
#define TRIPDIMAP 7,8,9,10
#define TRIPDOMAP 2,2,2,2	   //trip nbr sequence
//#define FRONTDITRIP	//ǰ���������ǵ������� 
//#define REARDITRIP		//�����������ǵ�������
#define SPECIALDITRIP

#define VERSION 12

//#define TEST
#define TEST_PARTS
//#define TEST_RTC
#define PRINTF_TEST
#define CAN_APP

#endif