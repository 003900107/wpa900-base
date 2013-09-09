#ifndef _REALTIMEDB_
#define _REALTIMEDB_

#include <time.h>
#include "soe.h"
#define STORAGE_ROMADDR 0x803D000
#define BACKUPS_ROMADDR 0x803E000

#define BKP_METHOD 0x01
#define FLASH_METHOD 0x02
#define STORE_METHOD FLASH_METHOD

#define CDT 0x01
#define MODBUS 0x02
#define SIERIAL103 0x03

#define TCP_MODBUS 0x01
#define SAC_NET103 0x02
#define SOE_FLASH_MAXNUM 25
#define ERR_FLASH_MAXNUM 10

#define ETH_LINK_TIME 10 //以分钟为单位
#define ETH_RECV_TIME 10 //以分钟为单位

typedef struct tag_SETTING_ITEM
{
  uint8_t SETTYPE;
  union
  {
    uint32_t	u_val;
    float   f_val;
  } un_val;
}Setting_Item;

typedef struct Tag_SETTING_STRCT
{
  uint8_t SettingNo;
  char SettingName[16];
  Setting_Item Setting ;
}SettingDemo;

typedef struct Tag_MEASURE_STRCT
{
  uint8_t MeasureNo;
  char MeasureName[16];
  float Value ;
}MeasureDemo;

typedef struct Tag_ComPara
{
  u16 Baud;
  u8 Parity;
  u8 WordLength;
  u8 StopBits;
  u8 Protocol; 
}ComPara;

typedef struct Tag_TrigSoe
{
  u8 total;
  u8 insrt_pos;
  SOE_Struct SOE_Queue_Flash[SOE_FLASH_MAXNUM];
}TrigSoe;

typedef struct Tag_TrigError
{
  u8 total;
  u8 insrt_pos;
  ERROR_Struct ERROR_Queue_Flash[ERR_FLASH_MAXNUM];
}TrigError;

typedef struct tag_SETTING
{	
  u16 Mem_Used_Check;
  u8 IPaddress[4];
  u16 DevNameChar[16];
  u8 vers;
  time_t cpledtm;
  u16 capa;
  u16 ct;
  u16 pt;
  ComPara Com1Para;
  ComPara Com2Para;
  TrigSoe TrigSoeQueue;
  TrigError ErrorQueue;
  //tyh:20130530 添加以太网状态判断时间
  u16 eth_link_time;
  u16 eth_recv_time;
  u16 i2c_reset;
  //tyh:20130806 添加CPU复位的记录
  u16 eth_recv_reset;
  u16 eth_link_reset;
  u16 def_reset;
  u16 eth_recv_day_reset;
  u16 eth_link_day_reset;
}Setting;

#if STORE_METHOD == FLASH_METHOD
u8 DataBase_Write(u32 FLASH_Offset, u32 *Writebuff, u16 Transfer_Length) ;
#else
u8 DataBase_Write(u32 FLASH_Offset, u16 *Writebuff, u16 Transfer_Length) ;
#endif

void DataBase_Init(uint8_t *IPTab);

#endif
