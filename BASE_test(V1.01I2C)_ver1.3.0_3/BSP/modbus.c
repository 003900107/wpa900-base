/*TCP_Modbus公共函数集*/


#include "modbus.h"
#include "BaseBus_Protocol.h"
#include "shellcmd.h"
//modbus错误定义
#define ILLEGAL_FUNCTION 	0x01
#define ILLEGAL_DATA_ADDRESS 	0x02
#define ILLEGAL_DATA_VALUE 	0x03
#define SLAVE_DEVICE_FAILURE 	0x04
#define ACKNOWLEDGE 		0x05
#define SLAVE_DEVICE_BUSY 	0x06
#define NEGATIVE_ACKNOWLEDGE 	0x07
#define MEMORY_PARITY_ERROR 	0x08

#define MEA_FUNCODE 0x04
#define TI_FUNCODE 0x03
#define DI_FUNCODE 0x02
#define DO_FUNCODE 0x05
extern u16 CAN_MeasureTab[4];
extern DiStatus_Type DiStatus_DI[16];
extern float I2C_MeasureTab[MEANUM];
extern const MeasureItem MeaPropTab[28]; 
__IO uint32_t CallMeasureTime=1;
uint16_t CRC16(uint8_t *puchMsg , uint16_t usDataLen )
{
  
  uint8_t uchCRCHi = 0xFF ;	/* high CRC byte initialized */
  uint8_t uchCRCLo = 0xFF ;	/* low CRC byte  initialized  */
  uint16_t uIndex ;		/* will index into CRC lookup*/
  /* table */
  
  while (usDataLen--)	/* pass through message buffer  */
  {
    uIndex = uchCRCHi ^ *puchMsg++ ;	/* calculate the CRC   */
    uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
    uchCRCLo = auchCRCLo[uIndex] ;
  }
  
  return (uchCRCHi << 8 | uchCRCLo) ;
}

uint8_t process_rx_data( uint8_t *byReadBuf,uint8_t *tx_buff,uint32_t rx_size )
{
  uint16_t Bitmap;
  uint8_t j,uiTxLen=0;
  short value;
  
  tx_buff+=6;
  //		if(0==CallMeasureTime++%MEACALLMASK)
  //		{
  //			AiQuerry();
  //		}
  
  if(byReadBuf[1]==MEA_FUNCODE)
  {
    tx_buff[0]=byReadBuf[0];
    tx_buff[1]=byReadBuf[1];
    tx_buff[2] =byReadBuf[5]*2;
    uiTxLen = tx_buff[2]+3;
    if ((byReadBuf[3]+byReadBuf[5]) <= (MEANUM+5))
    {
      //usCheck=CRC16(byReadBuf,rx_size-2);
      //	if ((((usCheck>>8) & 0xff)==byReadBuf[rx_size-2]) &&((usCheck&0xff)==byReadBuf[rx_size-1]))
      if((byReadBuf[3]>0)&&(byReadBuf[3]<29))
      {
        for(j=0;j<byReadBuf[5];j++)
        {
          value=Modbus_Meas_Prescale(I2C_MeasureTab[byReadBuf[3]+j-1],MeaPropTab[j].scale);
          tx_buff[3+j*2]=value&0x00ff;												  
          tx_buff[4+j*2]=value>>8&0x00ff;
          if ((byReadBuf[3]+j)>24)   
          {
            value=CAN_MeasureTab[byReadBuf[3]+j-25];
            tx_buff[3+j*2]=value&0x00ff;												  
            tx_buff[4+j*2]=value>>8&0x00ff;
          }
        }
      }
      else
      {
        tx_buff[1] |= 0x80;
        tx_buff[2] = NEGATIVE_ACKNOWLEDGE;
        uiTxLen = 5;
      }	
    }
    else
    {
      tx_buff[1] |= 0x80;
      tx_buff[2] = ILLEGAL_DATA_ADDRESS;
      tx_buff[3]=0x00;
      tx_buff[4]=0x00;
      uiTxLen = 5;
    }
    MODBUS_Send(tx_buff,uiTxLen);    	
  }
  else if(byReadBuf[1]==DI_FUNCODE)
  {
    tx_buff[0]=byReadBuf[0];
    tx_buff[1]=byReadBuf[1];	
    tx_buff[2]=0x02;
    uiTxLen = tx_buff[2]+3;
    if ((byReadBuf[3]+byReadBuf[5]) <= 17)
    {
      //usCheck=CRC16(byReadBuf,rx_size-2);
      //if ((((usCheck>>8) & 0xff)==byReadBuf[rx_size-2]) &&((usCheck&0xff)==byReadBuf[rx_size-1]))
      if((byReadBuf[3]>0)&&(byReadBuf[3]<17))
      {
        Bitmap=0x0000;
        for(j=0;j<16;j++)
        {
          if(1==DiStatus_DI[j].Value) Bitmap|=(0x01<<j);
        }  
        
        tx_buff[3]=Bitmap&0xff;
        tx_buff[4]=Bitmap>>8&0xff;  
      }			
      else
      {
        tx_buff[1] |= 0x80;
        tx_buff[2] = NEGATIVE_ACKNOWLEDGE;
        tx_buff[3]=0x00;
        tx_buff[4]=0x00;
        uiTxLen = 5;
      }	
    }
    else
    {
      tx_buff[1] |= 0x80;
      tx_buff[2] = ILLEGAL_DATA_ADDRESS;
      uiTxLen = 5;
    }
    MODBUS_Send(tx_buff,uiTxLen);   	
  }
  else if(byReadBuf[1]==DO_FUNCODE)
  {
    tx_buff[0]=byReadBuf[0];
    tx_buff[1]=byReadBuf[1];	
    
    uiTxLen = 6;
    if ((0x00==byReadBuf[2])&&(byReadBuf[3]<5)&&(byReadBuf[3]!=0))
    {
      //usCheck=CRC16(byReadBuf,rx_size-2);
      //	if ((((usCheck>>8) & 0xff)==byReadBuf[rx_size-2]) &&((usCheck&0xff)==byReadBuf[rx_size-1]))
      {
        if((0x00==byReadBuf[4])&&(0xaa==byReadBuf[5]))	 
        {
          if( !DoExecute((u8)(byReadBuf[3]*2-1)))
          {
            //修改应答报文, 通知主站遥控失败  
            tx_buff[1] |= 0x80;
            tx_buff[2] = SLAVE_DEVICE_FAILURE;
            uiTxLen =3;            
          }
          else
          {
            tx_buff[2]=byReadBuf[2];
            tx_buff[3]=byReadBuf[3];
            tx_buff[4]=byReadBuf[4];
            tx_buff[5]=byReadBuf[5];
          }
        }
        else if((0x00==byReadBuf[4])&&(0xcc==byReadBuf[5]))
        {
          if( !DoExecute((u8)(byReadBuf[3]*2)) )
          {
            //修改应答报文, 通知主站遥控失败 
            tx_buff[1] |= 0x80;
            tx_buff[2] = SLAVE_DEVICE_FAILURE;
            uiTxLen =3;            
          }
          else
          {
            tx_buff[2]=byReadBuf[2];
            tx_buff[3]=byReadBuf[3];
            tx_buff[4]=byReadBuf[4];
            tx_buff[5]=byReadBuf[5];
          }
        }	 
        else
        {
          tx_buff[1] |= 0x80;
          tx_buff[2] = SLAVE_DEVICE_FAILURE;
          uiTxLen =3;
        }	
      }			
      //  else
      //		{
      //			tx_buff[1] |= 0x80;
      //		tx_buff[2] = NEGATIVE_ACKNOWLEDGE;
      //		uiTxLen = 5;
      //		}	
    }
    else
    {
      tx_buff[1] |= 0x80;
      tx_buff[2] = ILLEGAL_DATA_ADDRESS;
      uiTxLen = 3;
    }
 
      MODBUS_Send(tx_buff,uiTxLen);   	
  }
  /*else if(byReadBuf[1]==TI_FUNCODE)
  {
  tx_buff[0]=byReadBuf[0];
  tx_buff[1]=byReadBuf[1];
  tx_buff[2] =byReadBuf[5]*2;
  uiTxLen = tx_buff[2]+3;
  if ((byReadBuf[3]+byReadBuf[5]) <= 4)
  {
  //usCheck=CRC16(byReadBuf,rx_size-2);
  //	if ((((usCheck>>8) & 0xff)==byReadBuf[rx_size-2]) &&((usCheck&0xff)==byReadBuf[rx_size-1]))
  {
  for(j=0;j<byReadBuf[5];j++)
  {
  value=CAN_MeasureTab[j];
  tx_buff[3+j*2]=value&0x00ff;												  
  tx_buff[4+j*2]=value>>8&0x00ff;
  
}
}
  //  else
  //	{
  //		tx_buff[1] |= 0x80;
  //	tx_buff[2] = NEGATIVE_ACKNOWLEDGE;
  //	uiTxLen = 5;
  //	}	
}
  else
  {
  tx_buff[1] |= 0x80;
  tx_buff[2] = ILLEGAL_DATA_ADDRESS;
  uiTxLen = 5;
}
  MODBUS_Send(tx_buff,uiTxLen);    	
} */
  else
  {
    tx_buff[0]=byReadBuf[0];
    tx_buff[1]=byReadBuf[1];
    tx_buff[1]|= 0x80;
    tx_buff[2]=ILLEGAL_FUNCTION;
    uiTxLen = 5;
    MODBUS_Send(tx_buff,uiTxLen);    		
  } 						
  
  return uiTxLen+6;
  /*
  if(byReadBuf[1] != DO_FUNCODE)
    return uiTxLen+6;
  else
    return 0;
  */
}

static void MODBUS_Send(uint8_t *pucBuf, uint8_t unLen)
{
  //uint16_t CRCval;
  //uint8_t i;
  pucBuf-=6;	
  pucBuf[0]=pucBuf[1]=pucBuf[2]=pucBuf[3]=pucBuf[4]=0x00;
  pucBuf[5]=unLen;
  //CRCval =CRC16(pucBuf, unLen-2);
  //pucBuf[unLen-1] =CRCval&0xFF;
  //pucBuf[unLen-2] =(CRCval>>8)&0xFF;	
}

int16_t Modbus_Meas_Prescale(float fmeasure,uint16_t scale)
{
  int16_t i16value;
  float fvalue;
  fvalue=(fmeasure)/scale;
  i16value=(int16_t)(fvalue*0x8000);
  return i16value;
}

//______________________la fin de document______________________________


