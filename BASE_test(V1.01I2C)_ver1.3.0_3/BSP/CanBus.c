#include "stm32f10x_i2c.h"
#include "bsp.h"
#include "CanBus.h"

/*
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

CanRxMsg RxMessage;
CanTxMsg TxMessage;
__IO uint8_t Can_Online=0x00;
__IO uint32_t Can_OnlineCnt;
u16 CAN_MeasureTab[4];
void _CAN1_RX0_IRQHandler(void)
{ 
	static uint8_t ErrAnswerCnt; 
  CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
  if ((RxMessage.StdId == 0x321)&&(RxMessage.IDE == CAN_ID_STD)&&(RxMessage.DLC == 8)&&(RxMessage.Data[0] == 0xA5)&&(RxMessage.Data[1] == 0x5A))
  {
    CanTextPreceive(RxMessage.Data);
	ErrAnswerCnt=0x00;
  }
  else
  {
  	ErrAnswerCnt++;
	if(ErrAnswerCnt&0x01) Can_Online=0x00;
  }
}

void _CAN2_RX0_IRQHandler(void)
{
  CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
  if ((RxMessage.StdId == 0x321)&&(RxMessage.IDE == CAN_ID_STD)&&(RxMessage.DLC == 8)&&(RxMessage.Data[0] == 0xA5)&&(RxMessage.Data[1] == 0x5A))
  
    CanTextPreceive(RxMessage.Data);
}

void CanTextPreceive(uint8_t *TextPtr)
{
	static uint8_t LED;
	if(LED++%2)
		GPIO_WriteBit(CANRX_LED,  Bit_RESET);
	else 
		GPIO_WriteBit(CANRX_LED,  Bit_SET);
  	
	
	switch(*(TextPtr+2))
		{
  	  			case TI_RES:
  	             			TiProcess(TextPtr+3);
							  break;
				case COM_DI:
  	             			ComDiProcess(*(TextPtr+3));
							break;
				case COM_AI:
  	             			ComAiProcess(*(TextPtr+3));
							break;
				case TESTLED:
  	             			TestProcess(*(TextPtr+3));
							break;
  	  			default:
				              break;	          
  	  	}	
}

void ComAiProcess(uint8_t AiNbr)
{

}

void ComDiProcess(uint8_t DidNbr)
{

}

void TiProcess(uint8_t* pRxIndex)
{
	Can_Online=0x01;
	memcpy((u8*)(&(CAN_MeasureTab[*pRxIndex-1])),pRxIndex+1,2);
	Can_OnlineCnt++;
}

void TestProcess(uint8_t LedNbr)
{
  printf("input %d actting\n\r ",LedNbr);
}

void TestTransmit(uint8_t LedNbr)
{
    TxMessage.Data[0] = 0xA5;
	TxMessage.Data[1] = 0x5A;
	TxMessage.Data[2] = TESTLED;
	TxMessage.Data[3] = LedNbr;
	TxMessage.Data[4] = 0x00;
	TxMessage.Data[5] = 0x00;
	TxMessage.Data[6] = 0x00;
	TxMessage.Data[7] = 0x00;

    CAN_Transmit(CAN2, &TxMessage);
}
float Convert_mV2mA(uint8_t i)
{
	float mA;
	mA=(CAN_MeasureTab[i-1]+2)/240.0;
	return mA;
}

void Can_Ticalib(CAN_TypeDef* CANx)
{
  static uint8_t errNum;
 
  TxMessage.Data[0]=0xA5;
  TxMessage.Data[1]=0x5A;
  TxMessage.Data[2]=TI_CAL;
      
  //添加发送错误判断
  if(CAN_GetLastErrorCode(CANx))
  {
//    if(CAN1->MCR & CAN_MCR_SLEEP)
//    {
//      CAN1->MCR &= ~CAN_MCR_SLEEP /*0xfffd*/;
//      errNum = 0;
//    }
    
    if(errNum++ > 7)  //连续出错达到上限值,复位设备
    {
      NVIC_SystemReset();
      //CAN1->MCR |= CAN_MCR_RESET;   
    }
  }
  else
  {
    CAN_Transmit(CANx, &TxMessage);
    errNum = 0;  
  }
  
  return;
}

uint8_t CAN_GetLastErrorCode(CAN_TypeDef* CANx)
{
  uint8_t errorcode=0;
  
  /* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));
  
  /* Get the error code*/
  errorcode = (((uint8_t)CANx->ESR) & (uint8_t)CAN_ESR_LEC);
  
  /* Return the error code*/
  return errorcode;
}			
			
				