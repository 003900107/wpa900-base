
/*écrivain : chinuxer 
SOE 触发引擎      袁博

务要传道，无论得时不得时，总要专心，并用百般的忍耐，各样的教训。因为时候要到，人必厌烦
纯正的道理，耳朵发痒，都随从自己的情欲。掩耳不听真道，偏向荒谬的言语。你却要凡事谨慎，
忍受苦难，作传道的功夫，尽你的职分。我现在被浇奠，我离世的时候到了。那艰苦的仗我已经打
过了，当赶的路我已经跑尽了，当信的道我已经守住了。从此以后，有公义的冠冕为我存留，就是
按着公义审判的主到了那日要赐给我的；不但赐给我，也赐给凡爱慕他显现的人。	
提多书	 */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "stm32f10x.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "Pad_client.h"
#include "BaseBus_Protocol.h"
#include "shellcmd.h"
#include "soe.h"
#include "lwp103client.h"
#include "Proj_Config.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern NetState comstate[4];
extern __IO uint8_t Msec_Value;
extern Setting SetCurrent;
SOE_Struct SOE_Queue[32];



void soe_engine( u8 i, u8 state)
{
  SOE_Struct temp_soe;
  
  temp_soe.state=state;
  temp_soe.diseq=i;
  temp_soe.timetamp=(time_t)RTC_GetCounter();
  temp_soe.mscd=Get_Msec();
  Soe_enqueue(&temp_soe);	
  Soe_tirg(&temp_soe);		
}

void Soe_enqueue(SOE_Struct *insert)
{
  static u8 inst_seq;
  
  memcpy(SOE_Queue+(inst_seq++&0x01f),insert,sizeof(SOE_Struct));
  if((insert->diseq>5)&&(insert->diseq<10))
  {
//    if (SetCurrent.TrigSoeQueue.total<=SOE_FLASH_MAXNUM) 
//      SetCurrent.TrigSoeQueue.total++;
//    else
//      SetCurrent.TrigSoeQueue.total=SOE_FLASH_MAXNUM;
    
    if (SetCurrent.TrigSoeQueue.total<SOE_FLASH_MAXNUM) 
      SetCurrent.TrigSoeQueue.total++;
    else
      SetCurrent.TrigSoeQueue.total=SOE_FLASH_MAXNUM;
    
    memcpy(SetCurrent.TrigSoeQueue.SOE_Queue_Flash+SetCurrent.TrigSoeQueue.insrt_pos,insert,sizeof(SOE_Struct));
    SetCurrent.TrigSoeQueue.insrt_pos=(SetCurrent.TrigSoeQueue.insrt_pos+1)%SOE_FLASH_MAXNUM;
    DataBase_Write(STORAGE_ROMADDR,(u32 *)(&SetCurrent),sizeof(Setting));
  }
}

void Soe_tirg(SOE_Struct *trig)
{
  if (1==comstate[TERM].state)
  {
    Term_soe_trig(trig);	
  }
  if (1==comstate[PAD].state)
  {
    Pad_soe_trig(trig);	
  }
#ifdef PT103EN
  if (comstate[P103].state>0)
  {
    P103_soe_trig(trig);	
  }
#endif
}
/********************************************
以下为error机制控制函数
用来实时全息反映系统或外设出错信息
以soe格式存储至flash，以备查找分析原因
********************************************/
void error_engine(uint32_t PERH_ZONE  )
{
  ERROR_Struct temp_soe;
  
  temp_soe.timetamp=(time_t)RTC_GetCounter();
  temp_soe.mscd=Get_Msec();
  
  switch(PERH_ZONE)
  {
  case I2C1_BASE:
    temp_soe.Perirh_baseaddr=I2C1_BASE;
    temp_soe.Register_1=I2C1->SR1;
    temp_soe.Register_2=I2C1->SR2;
    temp_soe.Register_3=I2C1->CR1;
    temp_soe.Register_4=I2C1->CR2;
//    telprintf("I2C1出错，SR1=%08x SR2=%08x CR1=%08x CR2=%08x\r\n",\
//      temp_soe.Register_1,temp_soe.Register_2,temp_soe.Register_3,temp_soe.Register_4);
    break;
  default:
    break;
  }
  error_enqueue(&temp_soe);	
  
}

void error_enqueue(ERROR_Struct *insert)
{	
  static u8 inst_seq;
  
  if (SetCurrent.ErrorQueue.total<ERR_FLASH_MAXNUM) 
    SetCurrent.ErrorQueue.total++;
  else
    SetCurrent.ErrorQueue.total=ERR_FLASH_MAXNUM;
  
  memcpy(SetCurrent.ErrorQueue.ERROR_Queue_Flash+SetCurrent.ErrorQueue.insrt_pos,insert,sizeof(ERROR_Struct));
  SetCurrent.ErrorQueue.insrt_pos=(SetCurrent.ErrorQueue.insrt_pos+1)%ERR_FLASH_MAXNUM;
  DataBase_Write(STORAGE_ROMADDR,(u32 *)(&SetCurrent),sizeof(Setting));
  
}


