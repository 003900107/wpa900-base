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


�ֽ�	����	����
1	A5	    ͬ����ͷ
2	5A	    ͬ����ͷ
3	TYPE	֡����
4	SEQ	    ���
5	DATA1	����1
6	DATA2	����2
7	DATA3	����3
8	DATA4	����4

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

#define I2C_SWRST_COUNT   7  //i2c�����λ������  tyh:20130806

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

__IO uint8_t I2C_BUSYHOLDING_FLAG=0x00;//holdס�����ź���


uint8_t BusBusyCounter=0;

extern I2C_InitTypeDef I2C_InitStruct;
float I2C_MeasureTab[MEANUM];
uint8_t Burst_Measure[MEANUM];
uint8_t Burst_Di[16];

//tyh:20130730 ����AI��λ������¼
float AI_Reset_Count = 0;

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

bool DoExecute(unsigned char DoSeq)
{
  bool result = Error;
  __IO uint32_t Timeout;
  
  if( (DoSeq!=0)&&(DoSeq<9) )
  {
    TxBuffer[0]=0xA5;
    TxBuffer[1]=0x5A;
    TxBuffer[2]=DO_EXE;
    TxBuffer[3]=DoSeq;
    TxBuffer[4]=0x00;
    TxBuffer[5]=0x00;
    TxBuffer[6]=0x00;
    TxBuffer[7]=0x00;
    
    /*tyh:20130801  ȥ��������� BinSemPend() BinSemPost()����
                   ��Ϊ������״̬�ж��Ƿ�������    */
    //if(BinSemPend(I2C_BUSYHOLDING_FLAG))
    if( Check_i2c_State() )     //ң�ر�������ִ�У������������״̬���ԣ�
    {                           //���ȸ�λI2C��Ȼ���ٴγ���ִ��ң������
      I2CHW_Reset();            
    }
    
    if(!Check_i2c_State())
    {
      result = I2C_Master_BufferWrite(TxBuffer, 8, DO);
      /*TYH: ��ӳ����� 20130831*/
      if(result != Success)
      {
        //tyh 20130801  ������ʧ�ܺ�, ����ֹͣλ������֪ͨ�����Ӱ����ݽ������̽���
        I2C_GenerateSTOP(I2C1, ENABLE);
        I2C_GenerateSTART(I2C1, ENABLE);
        
        //��λI2C����
        I2CHW_Reset();
      } 
      
      //BinSemPost(I2C_BUSYHOLDING_FLAG);
    }
    else
      result = Error;
  }
  
  return result;
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
  
  //tyh:20130801  ȥ��������� BinSemPend() BinSemPost()����
  //              ��Ϊ������״̬�ж��Ƿ�������     
  //if(BinSemPend(I2C_BUSYHOLDING_FLAG))
  if(!Check_i2c_State())
  {
    Wstatus = I2C_Master_BufferWrite(TxBuffer,8,AIN);
    /*TYH: ��ӳ����� 20130831*/
    if(Wstatus != Success)
    {
      //tyh 20130801  ������ʧ�ܺ�, ����ֹͣλ������֪ͨ�����Ӱ����ݽ������̽���
      I2C_GenerateSTOP(I2C1, ENABLE);
      I2C_GenerateSTART(I2C1, ENABLE);
      
      //��λI2C����
      I2CHW_Reset();  
    }    
    //BinSemPost(I2C_BUSYHOLDING_FLAG);
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
  
  //tyh:20130801  ȥ��������� BinSemPend() BinSemPost()����
  //              ��Ϊ������״̬�ж��Ƿ�������   
  //if(BinSemPend(I2C_BUSYHOLDING_FLAG))
  if(!Check_i2c_State())
  {
    Wstatus = I2C_Master_BufferWrite(TxBuffer,8,AIN);
    /*TYH: ��ӳ����� 20130831*/
    if(Wstatus != Success)
    {
      //tyh 20130801  ������ʧ�ܺ�, ����ֹͣλ������֪ͨ�����Ӱ����ݽ������̽���
      I2C_GenerateSTOP(I2C1, ENABLE);
      I2C_GenerateSTART(I2C1, ENABLE);
      
      //��λI2C����
      I2CHW_Reset();        
    }      
    //BinSemPost(I2C_BUSYHOLDING_FLAG);
  }
  
  if(Wstatus)
  {
    return 1;
  }
  
  return 0;
}

bool AiQuerry(void)
{
  bool result = Error;
  static uint8_t error_count = 0;   //i2c ����������������
  
  u8 othello_cnt4i2cerr = 0;
  
  memset(RxBuffer, 0, 127);
  
  //tyh:20130801  ȥ��������� BinSemPend() BinSemPost()����
  //              ��Ϊ������״̬�ж��Ƿ�������
  //if( BinSemPend(I2C_BUSYHOLDING_FLAG) )
  if(!Check_i2c_State())    //�������״̬��Ϊæ����������������
  {
    if( (error_count>7)&&(error_count%7) )    //������7��AI���ݲ��ɹ�ʱ����������Ƶ��
    {                                         //��Ϊ7����������һ�Σ�ֻҪ����һ���ɹ����ָ�����Ƶ��
      error_count++;
      result = Error;
      
      return result;
    }  
    
#if MEAUPDATE_METHOD==SINGLEBYTE
    result=I2C_Master_BufferRead(RxBuffer,8,AIN);
#elif MEAUPDATE_METHOD==MEMBLKCP
    result=I2C_Master_BufferRead(RxBuffer,/*98*/102,AIN);  //tyh20130730 ���AI�帴λ�����Ĵ���
#endif
    
    /*TYH: ��ӳ����� 20120928*/
    if(result != Success)
    {
      if(0 == othello_cnt4i2cerr)
      {
        error_engine(I2C1_BASE);
        othello_cnt4i2cerr++;
      }
      
      error_count++;
      
      //tyh 20130801  ������ʧ�ܺ�, ����ֹͣλ������֪ͨ�����Ӱ����ݽ������̽���
      I2C_GenerateSTOP(I2C1, ENABLE);
      I2C_GenerateSTART(I2C1, ENABLE);       
      
      //�������, ��λI2C�豸
      I2CHW_Reset();
    } 
    else
    {
      othello_cnt4i2cerr=0;
      error_count = 0;
    }
    
    //BinSemPost(I2C_BUSYHOLDING_FLAG);
  }
  
  return result;
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
  
  //tyh:20130801  ȥ��������� BinSemPend() BinSemPost()����
  //              ��Ϊ������״̬�ж��Ƿ�������     
  //if(BinSemPend(I2C_BUSYHOLDING_FLAG))
  if(!Check_i2c_State())    //�������״̬��Ϊæ����������������  
  {
    Wstatus = I2C_Master_BufferWrite(TxBuffer,8,AIN);
    /*TYH: ��ӳ����� 20130831*/
    if(Wstatus != Success)
    {     
      //tyh 20130801  ������ʧ�ܺ�, ����ֹͣλ������֪ͨ�����Ӱ����ݽ������̽���
      I2C_GenerateSTOP(I2C1, ENABLE);
      I2C_GenerateSTART(I2C1, ENABLE);
      
      //��λI2C����
      I2CHW_Reset();        
    }      
    //BinSemPost(I2C_BUSYHOLDING_FLAG);
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
  //20130730 tyh ��¼AI��λ����
  memcpy(&AI_Reset_Count, pDatacopied+92, 4);
}
#endif 

/*
ң�����ϴ����Ŀ�����
bit7 bit6 bit5	bit4~bit0
|    |    |	   | �ٻ����� |

*/
void Deal_I2CComming(void)
{
  u16 CRCValue;
  //if(1==MasterReceptionComplete)
  {
    if( HEADCHECK(RxBuffer[0], RxBuffer[1]) )
    {
      switch(RxBuffer[2])
      {
      case AI_RES:
#if MEAUPDATE_METHOD == MEMBLKCP
        //CRCValue =checksum16(RxBuffer, 100);
        CRC_ResetDR();
        CRCValue =CRC_CalcBlockCRC((uint32_t *)RxBuffer,/*24*/25);
        if((RxBuffer[/*96*/100] == (CRCValue&0xFF))&&(RxBuffer[/*97*/101] ==(CRCValue>>8&0xFF)))  //tyh:20130730 ���Ӹ�λ�����Ĵ���
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
  for(wait=TOLERER; Sem; wait--)
  {
    if(0==wait) 
      return 0;
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
  static uint8_t i2c_rst_cnt = 0;
  
  if (1==I2C_BUSYHOLDING_FLAG)
  {
    return;
  } 
  else
  {
    //temp=I2C1->SR2;
    //if(temp&0x0002)   
    if(Check_i2c_State())   //i2c bus busy
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
  
  if(i2c_rst_cnt > I2C_SWRST_COUNT)
  {
    ResetCpu(I2C_RESET);
    return;
  }
  
  if(BusBusyCounter > 0x07) //10s
  { 
    I2C_Cmd(I2C1,DISABLE);
    I2C_Cmd(I2C1,ENABLE);
    
    I2CHW_Reset();
    i2c_rst_cnt++;
    
    BusBusyCounter=0;
  }
  
  return;
} 

/*------------------------------End of File------------------------------------------*/
