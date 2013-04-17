
/*��crivain : chinuxer 
SOE ��������      Ԭ��

��Ҫ���������۵�ʱ����ʱ����Ҫר�ģ����ðٰ�����ͣ������Ľ�ѵ����Ϊʱ��Ҫ�����˱��ᷳ
�����ĵ������䷢����������Լ����������ڶ����������ƫ������������ȴҪ���½�����
���ܿ��ѣ��������Ĺ��򣬾����ְ�֡������ڱ����죬��������ʱ���ˡ��Ǽ��������Ѿ���
���ˣ����ϵ�·���Ѿ��ܾ��ˣ����ŵĵ����Ѿ���ס�ˡ��Ӵ��Ժ��й���Ĺ���Ϊ�Ҵ���������
���Ź������е�����������Ҫ�͸��ҵģ������͸��ң�Ҳ�͸�����Ľ�����ֵ��ˡ�	
�����	 */

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
����Ϊerror���ƿ��ƺ���
����ʵʱȫϢ��ӳϵͳ�����������Ϣ
��soe��ʽ�洢��flash���Ա����ҷ���ԭ��
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
//    telprintf("I2C1����SR1=%08x SR2=%08x CR1=%08x CR2=%08x\r\n",\
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


