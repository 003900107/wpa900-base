
/*écrivain : chinuxer 
通讯管理      袁博*/
/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "stm32f10x.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "Pad_client.h"
#include "BaseBus_Protocol.h"
#include "shellcmd.h"
#include "com_manager.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t DevIPAddressTab[4] ;

NetState comstate[4]=
{
  {"Termin",0,0,0},
  {"Pt103",0,0,0},
  {"PAD",0,0,0},
  {"Modbus",0,0,0},
};


u8 netstat(char *p)
{
  u8 i,IpTab[4],strcnt=0;
  
  sprintf(p,"协议	  本地地址	   外地地址     状态\r\n");
  strcnt+=strlen("协议	  本地地址	   外地地址     状态\r\n");
  
  p+=strcnt;
  
  for(i=0;i<4;i++)
  {
    if(comstate[i].state>0) 
    {
      IpTab[0] = (uint8_t)((uint32_t)(comstate[i].foreignIP) >> 24);  
      IpTab[1] = (uint8_t)((uint32_t)(comstate[i].foreignIP) >> 16);
      IpTab[2] = (uint8_t)((uint32_t)(comstate[i].foreignIP) >> 8);
      IpTab[3] = (uint8_t)((uint32_t)(comstate[i].foreignIP));
      sprintf(p, "%6s %3d.%3d.%3d.%3d %3d.%3d.%3d.%3d %8s\r\n",comstate[i].name,DevIPAddressTab[0],DevIPAddressTab[1],\
        DevIPAddressTab[2],DevIPAddressTab[3], IpTab[3],IpTab[2],IpTab[1],IpTab[0],(comstate[i].state==1?"链接建立":"报文活动"));
      strcnt+=49; 
      p+=49;
    }
  }
  
  
  return strcnt;
}