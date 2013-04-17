/*
*     @arg I2C_EVENT_SLAVE_ADDRESS_MATCHED   : EV1
*     @arg I2C_EVENT_SLAVE_BYTE_RECEIVED     : EV2
*     @arg I2C_EVENT_SLAVE_BYTE_TRANSMITTED  : EV3
*     @arg I2C_EVENT_SLAVE_ACK_FAILURE       : EV3-2
*     @arg I2C_EVENT_MASTER_MODE_SELECT      : EV5
*     @arg I2C_EVENT_MASTER_MODE_SELECTED    : EV6
*     @arg I2C_EVENT_MASTER_BYTE_RECEIVED    : EV7
*     @arg I2C_EVENT_MASTER_BYTE_TRANSMITTED : EV8
*     @arg I2C_EVENT_MASTER_MODE_ADDRESS10   : EV9
*     @arg I2C_EVENT_SLAVE_STOP_DETECTED     : EV4


字节	报文	释义
1	A5	    同步字头
2	5A	    同步字头
3	TYPE	帧类型
4	SEQ	    序号
5	DATA1	数据1
6	DATA2	数据2
7	DATA3	数据3
8	DATA4	数据4

*/

#include "BaseBus_Protocol.h"
#include "shellcmd.h"
#include "modbus.h"

#define Transmitter             0x00
#define Receiver                0x01
#define AIN 0xC0
#define DO  0xB0


#define TOLERER 0x1fff
#define I2C_PollingWait 0xfff


uint8_t Compress_Factor=10;
unsigned char TxBuffer[8];
#if MEAUPDATE_METHOD == SINGLEBYTE
unsigned char RxBuffer[8];
#endif
#if MEAUPDATE_METHOD == MEMBLKCP
unsigned char RxBuffer[127];
#endif
extern const MeasureItem MeaPropTab[28]; 
uint8_t CommingCall=0;
uint8_t BusHolding=0;

__IO uint8_t MasterReceptionComplete = 0;

__IO uint8_t I2C_BUSYHOLDING_FLAG=0x00;//hold住总线信号量


uint8_t BusBusyCounter=0;

extern I2C_InitTypeDef I2C_InitStruct;
float I2C_MeasureTab[MEANUM];
uint8_t Burst_Measure[MEANUM];
uint8_t Burst_Di[16];

static float MeaRecording[MEANUM];

#define HEADCHECK(a,b)  ((a==0xA5&&b==0x5A)?1:0)
#define BufferSize            8

uint16_t checksum16(uint8_t *pByte , uint16_t Len )
{
  uint16_t sum=0,i=0;
  while(Len--)
    sum+=*(pByte+i++);
  return sum;	
}

void DoSelect(unsigned char DoSeq)
{
  TxBuffer[0]=0xA5;
  TxBuffer[1]=0x5A;
  TxBuffer[2]=DO_SEL;
  TxBuffer[3]=DoSeq;
  TxBuffer[4]=0x00;
  TxBuffer[5]=0x00;
  TxBuffer[6]=0x00;
  TxBuffer[7]=0x00;
  
  //	i2c_buffer_write();
}

void DoExecute(unsigned char DoSeq)
{
  if((DoSeq!=0)&&(DoSeq<9))
  {
    TxBuffer[0]=0xA5;
    TxBuffer[1]=0x5A;
    TxBuffer[2]=DO_EXE;
    TxBuffer[3]=DoSeq;
    TxBuffer[4]=0x00;
    TxBuffer[5]=0x00;
    TxBuffer[6]=0x00;
    TxBuffer[7]=0x00;
    
    if(BinSemPend(I2C_BUSYHOLDING_FLAG))
    {
      I2C_Master_BufferWrite(TxBuffer,8,DO);
      BinSemPost(I2C_BUSYHOLDING_FLAG);
    }
  }	
}

bool Calibration(void)
{
  u8 Wstatus=0;
  
  
  memset(RxBuffer,0,8);
  TxBuffer[0]=0xA5;
  TxBuffer[1]=0x5A;
  TxBuffer[2]=AI_CALB;
  TxBuffer[3]=AI_CALB;
  TxBuffer[4]=0xFF;
  TxBuffer[5]=0xFF;
  TxBuffer[6]=0x00;
  TxBuffer[7]=0x00;
  
  if(BinSemPend(I2C_BUSYHOLDING_FLAG))
  {
    Wstatus=I2C_Master_BufferWrite(TxBuffer,8,AIN);
    BinSemPost(I2C_BUSYHOLDING_FLAG);
  }
  if(Wstatus)
  {
    return 1;
  }
  return 0;
  
}

bool Time_Calib(void)
{
  u8 Wstatus=0;
  time_t tempcounter;
  
  memset(RxBuffer,0,8);
  TxBuffer[0]=0xA5;
  TxBuffer[1]=0x5A;
  TxBuffer[2]=AI_TIME;
  TxBuffer[3]=AI_TIME;
  tempcounter = (time_t)RTC_GetCounter();
  memcpy((u8*)(&tempcounter),&TxBuffer[4],4); 
  
  if(BinSemPend(I2C_BUSYHOLDING_FLAG))
  {
    Wstatus=I2C_Master_BufferWrite(TxBuffer,8,AIN);
    BinSemPost(I2C_BUSYHOLDING_FLAG);
  }
  if(Wstatus)
  {
    return 1;
  }
  return 0;
  
}

bool AiQuerry(void)
{
  bool result;
  
  u8 othello_cnt4i2cerr = 0;
  memset(RxBuffer,0,127);
  
  if(BinSemPend(I2C_BUSYHOLDING_FLAG))
  {
#if MEAUPDATE_METHOD==SINGLEBYTE
    result=I2C_Master_BufferRead(RxBuffer,8,AIN);
#endif
#if MEAUPDATE_METHOD==MEMBLKCP
    result=I2C_Master_BufferRead(RxBuffer,98,AIN);
#endif
    /*TYH: 添加出错处理 20120928*/
    if(result != Success)
    {
      if(0 == othello_cnt4i2cerr)
      {
        error_engine(I2C1_BASE);
        othello_cnt4i2cerr++;
      }
      
      I2CHW_Reset();
    } 
    else
    {
      othello_cnt4i2cerr=0;	
    }
    BinSemPost(I2C_BUSYHOLDING_FLAG);
    return result;
  }
  return 0;
}

bool AiCoefSet(uint8_t AiSeq,uint32_t coef)
{
  u8 Wstatus=0;
  
  memset(RxBuffer,0,8);
  TxBuffer[0]=0xA5;
  TxBuffer[1]=0x5A;
  TxBuffer[2]=AI_COF;
  TxBuffer[3]=AiSeq;
  TxBuffer[4]=coef&0xff;
  TxBuffer[5]=coef>>8;
  TxBuffer[6]=0x00;
  TxBuffer[7]=0x00;
  
  // status=i2c_buffer_write();
  if(BinSemPend(I2C_BUSYHOLDING_FLAG))
  {
    Wstatus=I2C_Master_BufferWrite(TxBuffer,8,AIN);
    BinSemPost(I2C_BUSYHOLDING_FLAG);
  }
  if(Wstatus)
  {
    /* while(timeout--);
    if(BinSemPend(I2C_BUSYHOLDING_FLAG))
    {
    Rstatus=I2C_Master_BufferRead(RxBuffer,8,AIN);
    BinSemPost(I2C_BUSYHOLDING_FLAG);
  }
    
    if(Rstatus&&(AI_COF==RxBuffer[2])&&(TxBuffer[3]==RxBuffer[3])&&(RxBuffer[4]))*/ 
    return 1;
    
  }
  return 0;
  
}
#if MEAUPDATE_METHOD==SINGLEBYTE
void AiProcess(unsigned char AiSeq)
{  	
  
  memcpy((u8*)(&(I2C_MeasureTab[AiSeq])),&RxBuffer[4],4); 
  if((I2C_MeasureTab[AiSeq]-MeaRecording[AiSeq])>Compress_Factor/100.0*MeaPropTab[AiSeq].scale)
    Burst_Measure[AiSeq]=0x01;
  MeaRecording[AiSeq]=	I2C_MeasureTab[AiSeq];
  // I2C_MeasureTab[AiSeq]= (float*)(&RxBuffer[4]);
  
  
  
}
#endif
#if MEAUPDATE_METHOD==MEMBLKCP
void AiProcess(unsigned char *pDatacopied)
{
  memcpy((u8*)I2C_MeasureTab,pDatacopied,92);	
}
#endif 
/*
遥测量上传报文控制字
bit7 bit6 bit5	bit4~bit0
|    |    |	   | 召唤次数 |

*/
void Deal_I2CComming(void)
{
  u16 CRCValue;
  //if(1==MasterReceptionComplete)
  {
    if(HEADCHECK	(RxBuffer[0],RxBuffer[1]))
    {
      switch(RxBuffer[2])
      {
      case AI_RES:
        
#if MEAUPDATE_METHOD == MEMBLKCP
        
        //CRCValue =checksum16(RxBuffer, 100);
        CRC_ResetDR();
        CRCValue =CRC_CalcBlockCRC((uint32_t *)RxBuffer,24);
        if((RxBuffer[96] == (CRCValue&0xFF))&&(RxBuffer[97] ==(CRCValue>>8&0xFF)))
          AiProcess(RxBuffer+4);
#endif 
#if MEAUPDATE_METHOD == SINGLEBYTE	 
        AiProcess(RxBuffer[3]);
#endif
        break;
        
      default:
        break;	          
      }	
    }
    //MasterReceptionComplete=0;
  }
}

bool BinSemPend(uint8_t Sem)
{
  uint32_t wait;
  for(wait=TOLERER;Sem;wait--)
  {
    if(0==wait) return 0;
  }
  Sem=0x01;
  return 1;
}
void BinSemPost(uint8_t Sem)
{
  Sem=0x00;
}

void I2CHW_Maintain(void)
{
  __IO uint32_t temp = 0;
  
  if (1==I2C_BUSYHOLDING_FLAG)
  {
    return;
  } 
  else
  {
    temp=I2C1->SR2;
    if(temp&0x0002)
    {
      BusBusyCounter++;
      GPIO_WriteBit(ALARM_LED,  Bit_RESET);
    }
    else
    {
      if(BusBusyCounter != 0)
        BusBusyCounter = 0;
      GPIO_WriteBit(ALARM_LED,  Bit_SET);
      
      
    }
    
  }
  
  if(BusBusyCounter>0x08)
  {
    I2C_Cmd(I2C1,DISABLE);
    I2C_Cmd(I2C1,ENABLE);
    I2CHW_Reset();
    BusBusyCounter=0;   
    //NVIC_SystemReset();
  }
} 

/*------------------------------End of File------------------------------------------*/
