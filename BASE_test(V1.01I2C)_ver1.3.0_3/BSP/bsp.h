#ifndef __BSP_H
#define __BSP_H
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "BaseBus_Protocol.h"
#include "CanBus.h"
#include "netconf.h"
#include "helloworld.h"
#include "Pad_client.h"
//#include "lwip/tcp.h"
#include "stm32_eth.h"
#include "stm32f10x_flash.h"
#include "Realtimedb.h"
#include "RTC_Time.h"
#include "com_manager.h"
#include "stdio.h"
#include "BaseBus_Protocol.h"
#include "soe.h"
#include "lwp103client.h"
#include "tcp_modbus.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define QRY_TMR_INTERVAL 2560

#define DiRetThreshold 10
#define DiActThreshold 16

#define Ethernet_HWRST_FLAG 0X10
#define Ethernet_SWRST_FLAG 0X01

#define COMRX_LED    GPIOA, GPIO_Pin_8
#define COMTX_LED    GPIOC, GPIO_Pin_9
#define CANRX_LED    GPIOC, GPIO_Pin_7
#define CANTX_LED    GPIOC, GPIO_Pin_6
#define ALARM_LED    GPIOD, GPIO_Pin_9
#define RUNSTAT_LED  GPIOD, GPIO_Pin_8
#define PWR_LED      GPIOB, GPIO_Pin_15

#define ETH_RESET    GPIOC, GPIO_Pin_10

#define DO0  GPIOD, GPIO_Pin_10
#define DO1  GPIOD, GPIO_Pin_11
#define DO2  GPIOD, GPIO_Pin_12
#define DO3  GPIOD, GPIO_Pin_13
#define DO4  GPIOD, GPIO_Pin_14
#define DO5  GPIOD, GPIO_Pin_15
#define DO6  GPIOD, GPIO_Pin_7
#define DO7  GPIOD, GPIO_Pin_4


#define DI0	 GPIOE,	GPIO_Pin_8
#define DI1	 GPIOE,	GPIO_Pin_9
#define DI2	GPIOE, GPIO_Pin_10
#define DI3	GPIOE, GPIO_Pin_11
#define DI4	GPIOE, GPIO_Pin_12
#define DI5	GPIOE, GPIO_Pin_13
#define DI6	GPIOE, GPIO_Pin_14
#define DI7	GPIOE, GPIO_Pin_15
#define DI8	 GPIOE,	GPIO_Pin_0
#define DI9	 GPIOE,	GPIO_Pin_1
#define DI10	 GPIOE,	GPIO_Pin_2
#define DI11	 GPIOE,	GPIO_Pin_3
#define DI12	 GPIOE,	GPIO_Pin_4
#define DI13	 GPIOE,	GPIO_Pin_5
#define DI14	 GPIOE,	GPIO_Pin_6
#define DI15	 GPIOE,	GPIO_Pin_7

									
typedef struct
{
  uint8_t Actcnt;
  uint8_t Retcnt;
  uint8_t Value; 
  //uint8_t SOE_Queue[16];	  
  
}DiStatus_Type;


//TYH:20130407 计数器
typedef struct
{
  uint32_t count;
  struct tm t_begin;
  struct tm t_now;
  uint32_t SWRST_count; //软复位计数器
}Count_Type;


void RCC_Configuration(void);
void RTC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void DMA_Configuration(void); 
void ADC_Configuration(void);
void COM1_Configuration(void);
void COM2_Configuration(void);
void I2C_Configuration(void);
void CAN_Configuration(void);
void SysTick_Configuration(void);
void TEMPMEA_Confitguration(void);
void Delay(volatile uint32_t nCount);
//void TimingDelay_Decrement(void);
void GPIO_DealInputData(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,uint8_t pos);
void Di_PostWork(void);
void DiScanning(void);
void Time_Update(void);
void WDGFeeding(void);
void I2CHW_Reset(void);
void Get_ChipSerialNum(void);
u32 Filter_Soft(u16 *pTab,u8 samnbr);
float GetTemperature(void);
void simpledelay(u32 timeout);
void _USART1_IRQHandler(void);

uint8_t EthStateCheck(void);
uint8_t EthLinkCheck(void);
uint8_t EthRecvCheck(void);
void Ethernet_Configuration(void);
void Ethernet_parameter_init(void);
void CloseEth(void);
void CommunicationInit(void);
void Udp_timing_test();
void Ethernet_SWRST();
void Ethernet_HWRST();

uint8_t Set_eth_reset_flag(const uint8_t flag);
uint8_t Get_eth_reset_flag();
uint8_t Reset_eth_recv_count();

//tyh:20130731
//void I2C_Configuration_cpal(void);

#endif

