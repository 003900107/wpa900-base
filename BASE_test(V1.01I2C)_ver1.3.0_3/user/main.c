/*
�������ǲ��ɶ����¸ҵ��ģ����������ıصô��ʹ͡�
�����������ͣ�ʹ�������������ּ�⣬�Ϳ��Ե�����Ӧ��ġ�
������������ϣ������
*/
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "bsp.h"
#include "lwp103client.h"
#include "Proj_Config.h"
/* Private variables ---------------------------------------------------------*/
extern __IO uint32_t LocalTime;
extern __IO uint32_t Can_OnlineCnt;
extern __IO uint8_t Can_Online;
extern DiStatus_Type DiStatus_DI[16];

//__IO struct tm time_now;
uint8_t DevIPAddressTab[4]={172,20,100,1};
const uint8_t DefaultIP[4]={172,20,100,1};

__IO uint8_t EthInitState=0;
__IO uint8_t RunStatturn;
__IO uint32_t QRYTimer = 0;
__IO uint32_t CANTimer = 0;
__IO uint32_t P103Timer = 0;
__IO uint32_t SECTimer = 0;
__IO uint8_t Msec_Value=0;

FlagStatus status;
uint8_t reset_flag = 0;


/* Private functions ---------------------------------------------------------*/

/**
* @brief   Main program
* @param  None
* @retval None
*/
void Task_Periodic_Handle(__IO uint32_t localtime)
{  
  static uint8_t Can_Online_Local;
  
  if ((localtime - QRYTimer) >= QRY_TMR_INTERVAL)
  {
    QRYTimer =  localtime;
    //  #if MEAUPDATE_METHOD==SINGLEBYTE
    AiQuerry();
    //  #endif
    
    I2CHW_Maintain();
    if(RunStatturn++&0x01)
    {
      GPIO_WriteBit(RUNSTAT_LED,  Bit_RESET);
      
    }
    else
    {
      GPIO_WriteBit(RUNSTAT_LED,  Bit_SET);
    } 
  }
  if ((localtime - CANTimer) >= 3500)
  {
    CANTimer =  localtime;
    if((Can_OnlineCnt-Can_Online_Local)>0)
      Can_Online_Local=Can_OnlineCnt;
    else
      Can_Online=0x00;
  }
#ifdef PT103EN
  if((localtime - P103Timer) >= 5000)
  {
    P103Timer=localtime;
    Task_Pt103();
  }
#endif
}
int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
  this i
  s done through SystemInit() function which is called from startup
  file (startup_stm32f10x_xx.s) before to branch to application main.
  To reconfigure the default setting of SystemInit() function, refer to
  system_stm32f10x.c file
  */     
  //GPIO_InitTypeDef GPIO_InitStructure;
  //uint32_t irq; //test IRQ
  
  
  /* System clocks configuration ---------------------------------------------*/
  SystemInit();
  RCC_Configuration();
  
  /* GPIO configuration ------------------------------------------------------*/
  GPIO_Configuration();
  GPIO_WriteBit(CANTX_LED,  Bit_SET);
  GPIO_WriteBit(CANRX_LED,  Bit_SET);
  GPIO_WriteBit(COMTX_LED,  Bit_SET);
  GPIO_WriteBit(COMRX_LED,  Bit_SET);
  GPIO_WriteBit(PWR_LED,  Bit_RESET);
  GPIO_WriteBit(ALARM_LED,  Bit_SET);
  GPIO_WriteBit(RUNSTAT_LED,  Bit_SET);

  GPIO_WriteBit(ETH_RESET,  Bit_RESET);//����DM9000 nRST, ��ʱ��λ����
  
  DataBase_Init(DevIPAddressTab);
  
#ifndef TEST  
  /* com1 configuration ------------------------------------------------------*/
  COM1_Configuration();
#ifdef TEST_PARTS  
  GPIO_WriteBit(CANTX_LED,  Bit_RESET);
  GPIO_WriteBit(CANRX_LED,  Bit_SET);
  GPIO_WriteBit(COMTX_LED,  Bit_SET);
  GPIO_WriteBit(COMRX_LED,  Bit_SET);
#endif
  printf(" <<<<<<< COM1 config complete <<<<<<<\r\n\r\n");  
  
  
  /* com2 configuration ------------------------------------------------------*/
  //COM2_Configuration();
#endif 
  
  /* i2c configuration ------------------------------------------------------*/
  printf(" >>>>>>> I2C config begin >>>>>>>\r\n");
  I2C_Configuration();
#ifdef TEST_PARTS  
  GPIO_WriteBit(CANTX_LED,  Bit_SET);
  GPIO_WriteBit(CANRX_LED,  Bit_RESET);
  GPIO_WriteBit(COMTX_LED,  Bit_SET);
  GPIO_WriteBit(COMRX_LED,  Bit_SET); 
#endif 
  printf(" <<<<<<< I2C config complete <<<<<<<\r\n\r\n");
  
#ifndef TEST  
  /* RTC configuration--------------------------------------------------------*/
  printf(" >>>>>>> RTC config begin >>>>>>>\r\n");
  RTC_Configuration();
#ifdef TEST_PARTS  
  GPIO_WriteBit(CANTX_LED,  Bit_RESET);
  GPIO_WriteBit(CANRX_LED,  Bit_RESET);
  GPIO_WriteBit(COMTX_LED,  Bit_SET);
  GPIO_WriteBit(COMRX_LED,  Bit_SET);
#endif  
  printf(" <<<<<<< RTC config complete <<<<<<<\r\n\r\n");
#endif
  
#ifdef CAN_APP  
  /* can configuration ------------------------------------------------------*/
  printf(" >>>>>>> CAN config begin >>>>>>>\r\n");
  CAN_Configuration();
#ifdef TEST_PARTS  
  GPIO_WriteBit(CANTX_LED,  Bit_SET);
  GPIO_WriteBit(CANRX_LED,  Bit_SET);
  GPIO_WriteBit(COMTX_LED,  Bit_RESET);
  GPIO_WriteBit(COMRX_LED,  Bit_SET);
#endif  
  printf(" <<<<<<< CAN config complete <<<<<<<\r\n\r\n");  
#endif
  
#ifndef TEST  
  /*temperature configuration------------------------------------------------------*/
  printf(" >>>>>>> TEMPMEA config begin >>>>>>>\r\n");
  TEMPMEA_Confitguration();
#ifdef TEST_PARTS  
  GPIO_WriteBit(CANTX_LED,  Bit_SET);
  GPIO_WriteBit(CANRX_LED,  Bit_RESET);
  GPIO_WriteBit(COMTX_LED,  Bit_RESET);
  GPIO_WriteBit(COMRX_LED,  Bit_SET);
#endif 
  printf(" <<<<<<< TEMPMEA config complete <<<<<<<\r\n\r\n");
#endif
  
  /* SysTick configuration ------------------------------------------------------*/
  printf(" >>>>>>> SysTick config begin >>>>>>>\r\n");
  SysTick_Configuration();
#ifdef TEST_PARTS  
  GPIO_WriteBit(CANTX_LED,  Bit_RESET);
  GPIO_WriteBit(CANRX_LED,  Bit_RESET);
  GPIO_WriteBit(COMTX_LED,  Bit_RESET);
  GPIO_WriteBit(COMRX_LED,  Bit_SET);
#endif  
  printf(" <<<<<<< SysTick config complete <<<<<<<\r\n\r\n");

  /* NVIC configuration ------------------------------------------------------*/
  printf(" >>>>>>> NVIC config begin >>>>>>>\r\n");
  NVIC_Configuration();
#ifdef TEST_PARTS  
  GPIO_WriteBit(CANTX_LED,  Bit_SET);
  GPIO_WriteBit(CANRX_LED,  Bit_SET);
  GPIO_WriteBit(COMTX_LED,  Bit_SET);
  GPIO_WriteBit(COMRX_LED,  Bit_RESET);
#endif  
  printf(" <<<<<<< NVIC config complete <<<<<<<\r\n\r\n");
  
  /* Update the SysTick IRQ priority should be higher than the Ethernet IRQ */
  /* The Localtime should be updated during the Ethernet packets processing */
  NVIC_SetPriority (SysTick_IRQn, 1); 
  
  /* test IRQ*/
//  irq = NVIC_GetPriority(SysTick_IRQn);
//  irq = NVIC_GetPriority(ETH_IRQn);
//  irq = NVIC_GetPriority(I2C1_ER_IRQn);
//  irq = NVIC_GetPriority(I2C1_EV_IRQn);
  
  
  /* ethernet configuration ------------------------------------------------------*/
  //�������ʱ, ����ȷ��DM9000����ʱ��ѹ����оƬҪ��
  //"nRST must not go high until after the VDDIO and VDD_CORE supplies are stable"  �ֲ�P51
  GPIO_WriteBit(ETH_RESET,  Bit_SET);   //����DM9000 nRST, ��λ����
  
  printf(" >>>>>>> ETH config begin >>>>>>>\r\n");
  Ethernet_Configuration();
#ifdef TEST_PARTS  
  GPIO_WriteBit(CANTX_LED,  Bit_RESET);
  GPIO_WriteBit(CANRX_LED,  Bit_SET);
  GPIO_WriteBit(COMTX_LED,  Bit_RESET);
  GPIO_WriteBit(COMRX_LED,  Bit_SET);
#endif  
  printf(" <<<<<<< ETH config complete <<<<<<<\r\n\r\n");  

    
#ifdef WATCHDOG
  /* WATCHDOG configuration ------------------------------------------------------*/
  IWDG_Configuration();
#endif
  
  //�ж��Ƿ�����̫������
  if(EthInitState)
  {
    CommunicationInit();
  }
  
  printf(" ******* ����'$'���������ڿ���̨!*******\r\n\r\n");
  GPIO_WriteBit(CANTX_LED,  Bit_SET);
  GPIO_WriteBit(CANRX_LED,  Bit_SET);
  GPIO_WriteBit(COMTX_LED,  Bit_SET);
  GPIO_WriteBit(COMRX_LED,  Bit_SET); 
  
  
  //�������, ���볣������
  uint8_t test = 0;
  while (1)
  {
    Di_PostWork();
    LwIP_Periodic_Handle(LocalTime);
    Task_Periodic_Handle(LocalTime);
    

    //�ж���̫��״̬, �����Ƿ�Ҫ��λ��̫��
    EthStateCheck();
    
    reset_flag = Get_eth_reset_flag();
    if(reset_flag == Ethernet_SWRST_FLAG)
      Ethernet_SWRST();
    else if(reset_flag == Ethernet_HWRST_FLAG)
      Ethernet_HWRST();
    
    //tyh:20130407 eth reset test
    if((DiStatus_DI[1].Value != test)&&(DiStatus_DI[1].Value == 1))
    {
      //eth_reg = ETH_ReadPHYRegister(0x1F, 17);
      Ethernet_HWRST();
    }
    test = DiStatus_DI[1].Value;

//    else
//    {
//      if( EthLinkCheck() )
//      {
//        //tyh:20130403 send udp test_message
//        if((DiStatus_DI[1].Value != test)&&(DiStatus_DI[1].Value == 1))
//        {
//          Udp_timing_test();
//        }
//        
//        test = DiStatus_DI[1].Value;
//      }
//    }
    
#ifdef WATCHDOG 
    WDGFeeding();
#endif
    
  }
}


#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  printf("\n\r Wrong parameter value detected on "); 
  printf("       file  %s", file); 
  printf("       line  %d\n", line); 
}

/* Infinite loop */
//  while (1)
//  {
//  }
#endif

int fputc(int ch, FILE *f) 
{ 
  /* Place your implementation of fputc here */ 
  /* e.g. write a character to the USART */
  //  uint8_t _ch=ch; 
  uint32_t Timeout=0x3fff;
  USART_SendData(USART1, (u8)ch); 
  
  /* Loop until the end of transmission */ 
  
  while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) 
  { 
    if(!Timeout--) break;
  } 
  return ch; 
}
/**
* @}
*/ 

/**
* @}
*/ 

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
