/*

神是信实的，你们原是被他所召，好与他儿子，我们的主耶稣基督，共同得分
——————哥林多前
*/

#include "bsp.h"
#include "Proj_Config.h"
//#include "cpal_i2c.h"
//#include "cpal.h"

__IO  int timingdelay;
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 1ms */
u16 ADCConvertedValue[60];
u16 SelectValueTab[30];
/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/

#define PHY_ADDRESS       0x1F
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define SYSTICK_INTERVAL 1000

#define ON 1
#define OFF 0

#define ETH_SWRST_COUNT 10  //软件复位最大次数

#define LSE_READ_ERROR    ((uint32_t)0x7FFFF)  //TYH: RTC等待'LSE'超时 
#define RCT_CLK_LSE   1   //RCT时钟为LSE
#define RCT_CLK_HSE   2   //RCT时钟为HSE

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
I2C_InitTypeDef I2C_InitStruct;
ETH_InitTypeDef ETH_InitStructure;
#ifdef CAN_APP
CAN_InitTypeDef  CAN_InitStructure;
CAN_FilterInitTypeDef  CAN_FilterInitStructure;
#endif
extern CanRxMsg RxMessage;
extern CanTxMsg TxMessage;
extern Setting SetCurrent;

/*唯一身份标识*/
u32 DeviceSerial[3];
extern uint8_t macaddress[6];
DiStatus_Type DiStatus_DI[16];
__IO uint8_t turnned[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}	;
__IO uint8_t Trip_turnned[NonElectro_Nbr];
extern uint8_t EthInitState;
uint8_t NonElectr_TripMap[NonElectro_Nbr]={TRIPDOMAP};   //the element write as the number of Trip
uint8_t NonElectr_TripDi[NonElectro_Nbr]={TRIPDIMAP};
char UART_RxBuffer[24];


struct tcp_pcb *hello_pcb;
struct udp_pcb *pad_pcb;

#ifdef TCPMODBUSEN
struct tcp_pcb *modbus_pcb;
struct udp_pcb *timing_upcb;
#endif
#ifdef PT103EN
struct tcp_pcb *ns103_pcb;
#endif

Count_Type eth_link_count;
Count_Type eth_recv_count;
static uint8_t eth_reset_flag;

/* Private function prototypes -----------------------------------------------*/
void UartProcess(char *RxBuffer, char RxCounter);
uint8_t RTC_Init(const uint8_t clk_source);

/**
* @brief  Configures the different system clocks.
* @param  None
* @retval None
*/
void RCC_Configuration(void)
{
  RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1|RCC_APB1Periph_USART2|
                         RCC_APB1Periph_PWR |RCC_APB1Periph_BKP, ENABLE);
#ifdef CAN_APP
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1|RCC_APB1Periph_CAN2, ENABLE);
#endif
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC|
                         RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB|
                           RCC_APB2Periph_GPIOD| RCC_APB2Periph_GPIOE|
                             RCC_APB2Periph_USART1|RCC_APB2Periph_ADC1 , ENABLE);
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
                        RCC_AHBPeriph_ETH_MAC_Rx | RCC_AHBPeriph_CRC|RCC_AHBPeriph_DMA1, ENABLE);
}

/**
* @brief  Configures the different GPIO ports.
* @param  None
* @retval None
*/
void GPIO_Configuration(void)
{
  /* Configure PC.02, PC.03 and PC.04 (ADC Channel12, ADC Channel13 and 
  ADC Channel14) as analog inputs */
  //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
  //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  //  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 /*| GPIO_Pin_11 | GPIO_Pin_12*/;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_9|GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|\
    GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);					    
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
}


/**
* @brief  Configures Vector Table base location.
* @param  None
* @retval None
*/
void NVIC_Configuration(void)
{
  //TYH:2013-05-10设置程序启动地址
  //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0xA000); 
  
  /* Configure and enable ADC interrupt */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);	  
  
  NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
  
  NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_Init(&NVIC_InitStructure);
  
#ifdef CAN_APP
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_Init(&NVIC_InitStructure);
#endif
  
#ifndef TEST  
  //20130312 TYH:添加 uart 接收中断, 用于处理串口数据
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);
#endif
  
}

void COM1_Configuration(void)
{
  /* USARTx configured as follow:
  - BaudRate = 9600 baud 
  - Word Length = 8 Bits
  - One Stop Bit
  - No parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
  /* Configure USART1 Tx (PA9) as alternate function push-pull */
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure USART1 Rx (PA10) as input floating */
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(USART1, &USART_InitStructure);
  /* Enable the USART Receive interrupt: this interrupt is generated when the
  USART1 receive data register is not empty */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); 
  /* Enable USART1 */
  USART_Cmd(USART1, ENABLE);
}


void COM2_Configuration(void)
{
  /* USARTx configured as follow:
  - BaudRate = 9600 baud 
  - Word Length = 8 Bits
  - One Stop Bit
  - No parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
  /* Configure USART2 Tx (PD5) as alternate function push-pull */
  
  
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /* Configure USART2 Rx (PD6) as input floating */
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(USART2, &USART_InitStructure);
  /* Enable USART2 */
  USART_Cmd(USART2, ENABLE);
}

void I2C_Configuration(void)
{
  __IO uint16_t timeout;
  GPIO_InitTypeDef GPIO_InitStructure; 
  DMA_InitTypeDef  I2CDMA_InitStructure;
  
  /* Enable I2C2 reset state */
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
  /* Release I2C2 from reset state */
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  
  GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;   //复用开漏输出
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStruct.I2C_ClockSpeed = 100000;
  I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStruct.I2C_OwnAddress1 = 0xA0;
  
  timeout=0x1ff;
  while(timeout--);
  I2C_Init(I2C1,&I2C_InitStruct); 
  
  timeout=0x1ff;
  while(timeout--);
  I2C_DeInit(I2C1);
  
  I2C_Init(I2C1,&I2C_InitStruct);
  
  I2C_Cmd(I2C1,ENABLE);
  
#if I2CMETHOD ==INTERRUPT
  I2C_ITConfig(I2C1, I2C_IT_EVT|I2C_IT_BUF|I2C_IT_ERR, ENABLE);
#endif
  
#if I2CMETHOD ==DMA		
  DMA_DeInit(DMA1_Channel6);
  
  I2CDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)I2C1_DR_Address;
  I2CDMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)0;   
  I2CDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    
  I2CDMA_InitStructure.DMA_BufferSize = 0xFFFF;            
  I2CDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  I2CDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  I2CDMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
  I2CDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  I2CDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  I2CDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  I2CDMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel6, &I2CDMA_InitStructure);
  
  DMA_DeInit(DMA1_Channel7);
  DMA_Init(DMA1_Channel7, &I2CDMA_InitStructure);
#endif
  
}

void RTC_Configuration(void)
{
  __IO uint32_t timeout = 0;  
  uint16_t bak_dr = 0; 
  uint8_t  result = 0;  //用于判断上一次初始化采用的时钟种类
  uint8_t rtc_clk_src;
  
  //tyh:130410 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  PWR_BackupAccessCmd(ENABLE);
  RCC_RTCCLKCmd(ENABLE);  
  
  /* Enable LSE */
  RCC_LSEConfig(RCC_LSE_ON); 
  /* Wait, is LSE ready ? */
  do
  {
    timeout++;
  } while ( (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (timeout < LSE_READ_ERROR) );
  if(timeout == LSE_READ_ERROR)
  {
    printf(" ******* RTC wait LSE ERROR *******\r\n");  
    rtc_clk_src = RCT_CLK_HSE;
  }
  else
  {
    printf(" ******* RTC wait LSE SUCCESS *******\r\n"); 
    rtc_clk_src = RCT_CLK_LSE;
  }
  
  bak_dr = BKP_ReadBackupRegister(BKP_DR1);
  printf(" ******* BKP_DR1=0x%x *******\r\n", bak_dr);
  if( bak_dr !=  0xA5A5 )
  {
    result = RTC_Init(rtc_clk_src);
    
    //写入初始化标志
    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    //写入初始化RTC的时钟类型
    BKP_WriteBackupRegister(BKP_DR2, result);
  }
  else
  {
    bak_dr = BKP_ReadBackupRegister(BKP_DR2);
    printf(" ******* BKP_DR2=%d *******\r\n", bak_dr);
    if(rtc_clk_src == RCT_CLK_HSE)
    {
      if(bak_dr != RCT_CLK_HSE)
      {
        result = RTC_Init(RCT_CLK_HSE);
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
        BKP_WriteBackupRegister(BKP_DR2, result);
      }
      else
      {
        return;
      }
    }
    else if(rtc_clk_src == RCT_CLK_LSE)
    {
      if(bak_dr != RCT_CLK_LSE)
      {
        result = RTC_Init(RCT_CLK_LSE);
        if(result == 0)                   //LSE未准备好，改用HSE初始化时钟
          result = RTC_Init(RCT_CLK_HSE);
        
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
        BKP_WriteBackupRegister(BKP_DR2, result);
      }
      else
      {
        return;
      }
    }
    
    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();    
  }
  
  return;
}

uint8_t RTC_Init(const uint8_t clk_source)
{
  uint8_t result = 0;
  __IO uint32_t timeout = 0; 
  
  /* Reset Backup Domain */
  BKP_DeInit();

  if(clk_source == RCT_CLK_HSE)
  { 
    RCC_HSEConfig(RCC_HSE_ON);
    
    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
    
    printf(" ******* RTC 时钟改为采用'HSE' *******\r\n");     
  }
  else if(clk_source == RCT_CLK_LSE)
  {
    /* Enable LSE */
    RCC_LSEConfig(RCC_LSE_ON);
    
    //再次判断LSE是否已经准备好
    do
    {
      timeout++;
    } while ( (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (timeout < LSE_READ_ERROR) );
    //LSE未准备好, 退出初始化
    if(timeout == LSE_READ_ERROR) 
    {
      printf(" ******* RTC wait LSE ERROR *******\r\n");  
      return 0;
    }    
    
    /* Select LSE as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
    printf(" ******* RTC 时钟采用'LSE' *******\r\n");     
  }
  
  /* Enable RTC Clock */
  RCC_RTCCLKCmd(ENABLE);
  
  /* Wait for RTC registers synchronization */
  RTC_WaitForSynchro();
  
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();   
  
  /* Enable the RTC Second */
  //RTC_ITConfig(RTC_IT_SEC, ENABLE);
  
  /* Wait until last write operation on RTC registers has finished */
  // RTC_WaitForLastTask();
  
  /* Set RTC prescaler: set RTC period to 1sec */
  if(clk_source == RCT_CLK_HSE)
  {    
    /* RTC period = RTCCLK/RTC_PR = (25MHz/128)/(195312+1) = 1  (HSE)*/
    RTC_SetPrescaler(195312);
    
    result = RCT_CLK_HSE;
  }
  else if(clk_source == RCT_CLK_LSE)
  {
    /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) add 1 every second  (LSE),
    as the time source of soe engine and RTC system  	 
    this RTC timer need timming sync by protocol from host station! 
    it's another timer system in parellel with SysTick_Handler()*/
    RTC_SetPrescaler(32767);
    
    result = RCT_CLK_LSE;
  }    
  
  /* Wait until last write operation on RTC registers has finished */
  /* 等待上一次对RTC寄存器的写操作完成 */
  RTC_WaitForLastTask();
  printf(" ******* RTC CLK IS %d\r\n *******", result);
  
  return result;
}

//void SysTick_Config(void)
//{
//    /* Disable SysTick Counter */
//    SysTick_CounterCmd(SysTick_Counter_Disable);        //失能计数器 
//  
//    /* Disable the SysTick Interrupt */
//    SysTick_ITConfig(DISABLE);   //关闭中断
//  
//    /* Configure HCLK clock as SysTick clock source */
//    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);   //8分频
//    /* SysTick interrupt each 1000 Hz with HCLK equal to 72MHz */
//    SysTick_SetReload(9000);   //周期1ms
//    /* Enable the SysTick Interrupt */
//    SysTick_ITConfig(ENABLE);  //打开中断
//
// /* Enable the SysTick Counter */
// //   SysTick_CounterCmd(SysTick_Counter_Enable); //使能计数器
//}

//void Delay(u32 nTime)
//{
//  /* Enable the SysTick Counter */
//  SysTick_CounterCmd(SysTick_Counter_Enable);
//  
//  TimingDelay = nTime;
//
//  while(TimingDelay != 0);
//
//  /* Disable SysTick Counter */
//  SysTick_CounterCmd(SysTick_Counter_Disable);
//  /* Clear SysTick Counter */
//  SysTick_CounterCmd(SysTick_Counter_Clear);
//}
void Ethernet_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  /* ETHERNET pins configuration */
  /* AF Output Push Pull:
  - ETH_MII_MDIO / ETH_RMII_MDIO: PA2
  - ETH_MII_MDC / ETH_RMII_MDC: PC1
  
  - ETH_MII_TX_EN / ETH_RMII_TX_EN: PB11
  - ETH_MII_TXD0 / ETH_RMII_TXD0: PB12
  - ETH_MII_TXD1 / ETH_RMII_TXD1: PB13	*/
  
  // Configure PA2 as alternate function push-pull 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure PC1, PC2 and PC3 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /* Configure PB5, PB8, PB11, PB12 and PB13 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 |GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* Input (Reset Value):
  --------------------------------------------
  - ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1
  
  - ETH_MII_RX_DV / ETH_RMII_CRS_DV: PA7
  
  - ETH_MII_RXD0 / ETH_RMII_RXD0: PC4
  - ETH_MII_RXD1 / ETH_RMII_RXD1: PC5
  
  - ETH_MII_RX_ER: PB10	 
  - ETH_MII_RX_INT: PB9 *
  */
  
  
  /* ETHERNET pins remapp in STM3210C-EVAL board: RX_DV and RxD[3:0] */
  /* Configure PA0, PA1 and PA3 as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure); /**/
  /* Configure PD8, PD9, PD10, PD11 and PD12 as input */
  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure); /**/
  
  // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  // GPIO_Init(GPIOB, &GPIO_InitStructure); /**/
  
  
  
  /* MII/RMII Media interface selection ------------------------------------------*/
  GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);
  
  //  /* Set PLL3 clock output to 50MHz (25MHz /5 *10 =50MHz) */
  //  RCC_PLL3Config(RCC_PLL3Mul_10);
  //  /* Enable PLL3 */
  //  RCC_PLL3Cmd(ENABLE);
  //  /* Wait till PLL3 is ready */
  //  while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET)
  //  {}
  //
  //  /* Get PLL3 clock on PA8 pin (MCO) */
  //  RCC_MCOConfig(RCC_MCO_PLL3CLK);
  
  
  /* Reset ETHERNET on AHB Bus */
  ETH_DeInit();
  
  /* Software reset */
  ETH_SoftwareReset();
  
  /* Wait for software reset */
  printf(" >>>>>>> ETH config [Wait for software reset] begin >>>>>>>\r\n");
  while (ETH_GetSoftwareResetStatus() == SET);
  printf(" <<<<<<< ETH config [Wait for software reset] complete <<<<<<<\r\n");
  
  /* ETHERNET Configuration ------------------------------------------------------*/
  /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
  ETH_StructInit(&ETH_InitStructure);
  
  /* Fill ETH_InitStructure parametrs */
  /*------------------------   MAC   -----------------------------------*/
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
  //ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
  // ETH_InitStructure.ETH_Mode = ETH_Mode_HalfDuplex;
#ifdef CHECKSUM_BY_HARDWARE
  ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif
  /*------------------------   DMA   -----------------------------------*/  
  
  /* When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
  the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
  if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
  ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; 
  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;         
  ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;     
  
  ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;       
  ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;   
  ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;                                                          
  ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;      
  ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;                
  ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;          
  ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;                                                                 
  ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
  
  /* Configure Ethernet */
  if(1==ETH_Init(&ETH_InitStructure, PHY_ADDRESS)) 
  {
    EthInitState=0x01;
    printf(" >>>>>>> ethernet init success <<<<<<<\r\n") ;
    
  }
  else
  {
    printf(" ******* ethernet init fail *******\r\n");
    EthInitState=0x00;
    // ETH_DeInit();
  }
  
  /* Enable the Ethernet Rx Interrupt */
  ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, DISABLE);
  ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);
  
  //TYH:20130407 初始化以太网判断变量
  Ethernet_parameter_init();
}


#ifdef CAN_APP
void CAN_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  uint8_t i = 0;
  /* Configure CAN1 RX pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /* Configure CAN2 RX pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* Configure CAN1 TX pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /* Configure CAN2 TX pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* Remap CAN1 and CAN2 GPIOs */
  GPIO_PinRemapConfig(GPIO_Remap2_CAN1 , ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_CAN2, ENABLE);
  
  CAN_DeInit(CAN1);
  CAN_DeInit(CAN2);
  CAN_StructInit(&CAN_InitStructure);
  
  /* CAN1 cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;
  CAN_InitStructure.CAN_ABOM = DISABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = DISABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = DISABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
  CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
  CAN_InitStructure.CAN_Prescaler = 200;
  CAN_Init(CAN1, &CAN_InitStructure);
  CAN_Init(CAN2, &CAN_InitStructure);
  //72MHZ/2/200/（1+9+8）=0.01 10K波特率
  /* CAN1 filter init */
  CAN_FilterInitStructure.CAN_FilterNumber = 0;
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
  
  /* CAN2 filter init */
  CAN_FilterInitStructure.CAN_FilterNumber = 14;
  CAN_FilterInit(&CAN_FilterInitStructure);
  
  /* IT Configuration for CAN1 */  
  CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
  /* IT Configuration for CAN2 */  
  CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE);
  /* Transmit */
  TxMessage.StdId = 0x321;
  TxMessage.ExtId = 0x01;
  TxMessage.RTR = CAN_RTR_DATA;
  TxMessage.IDE = CAN_ID_STD;
  TxMessage.DLC = 8;
  
  RxMessage.StdId = 0x00;
  RxMessage.ExtId = 0x00;
  RxMessage.IDE = CAN_ID_STD;
  RxMessage.DLC = 0;
  RxMessage.FMI = 0;
  for (i = 0; i < 8; i++)
  {
    RxMessage.Data[i] = 0x00;
  }
}
#endif

void TEMPMEA_Confitguration(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;	
  
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)ADCConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 60;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 2;
  ADC_Init(ADC1, &ADC_InitStructure);
  /* ADC1 regular channel14 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 2, ADC_SampleTime_239Cycles5);
  /* Enable the temperature sensor and vref internal channel */ 
  ADC_TempSensorVrefintCmd(ENABLE); 
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  /* Enable ADC1 reset calibaration register */ 
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));
  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
  
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void IWDG_Configuration(void)
{
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  
  /* IWDG counter clock: 32KHz(LSI) / 32 = 1KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_32);
  
  /* Set counter reload value to 349 */
  IWDG_SetReload(/*349*/0x0fff); //tyh:20130312  修改开门狗计数时间, 
  //            避免串口收发中断时间过长造成的设备重启
  
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
  
}

void Time_Update(void)
{
  LocalTime++;
}

void SysTick_Configuration(void)
{
  if (SysTick_Config(SystemCoreClock / SYSTICK_INTERVAL))
  { 
    /* Capture error */ 
    while (1);
  }
  
}

void Delay(volatile uint32_t nCount)

{ 
  timingdelay = LocalTime + nCount;  
  
  /* wait until the desired delay finish */  
  while(timingdelay > LocalTime)
  {     
  }
}

void GPIO_DealInputData(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,uint8_t pos)
{
  if(Bit_SET==GPIO_ReadInputDataBit(GPIOx,GPIO_Pin))				 //无读取开入
  {
    if(DiStatus_DI[pos].Retcnt>3) 
      DiStatus_DI[pos].Actcnt=0;	
    
    if(DiStatus_DI[pos].Retcnt<DiRetThreshold) 
      DiStatus_DI[pos].Retcnt++;
    else 
      DiStatus_DI[pos].Value=0;
  }
  else 															 //有读取开入
  {
    if(DiStatus_DI[pos].Actcnt>3) 
      DiStatus_DI[pos].Retcnt=0;
    
    if(DiStatus_DI[pos].Actcnt<DiActThreshold) 
      DiStatus_DI[pos].Actcnt++;
    else 
      DiStatus_DI[pos].Value=1;
  }
}

void Di_PostWork(void)
{
  uint8_t i;
  
#ifdef SPECIALDITRIP
  for(i=0;i<NonElectro_Nbr;i++)
  {		 
    if(1==DiStatus_DI[NonElectr_TripDi[i]-1].Value)
    {
      if(Trip_turnned[i]==0)
      {      
        if( DoExecute(NonElectr_TripMap[i]))
        {          
          //printlog("input%d occur",i-7);
          turnned[i]=1;
        }        
      }		  
    }
    else
    {
      Trip_turnned[i]=0;
    }
  } 
  for(i=0;i<16;i++)
  {
    if(ON==DiStatus_DI[i].Value)
    {
      if(turnned[i]==OFF)
      {
        soe_engine(i,ON);
        turnned[i]=ON;
      }		  
    }
    else if(OFF==DiStatus_DI[i].Value)
    {
      if(turnned[i]==ON)
      {
        soe_engine(i,OFF);
        turnned[i]=OFF;
      }	
    }
  }
#endif
  
#ifdef FRONTDITRIP
  for(i=0;i<NonElectro_Nbr;i++)
  {
    if(1==DiStatus_DI[i].Value)
    {
      if(turnned[i]==0)
      {
        if( DoExecute(NonElectr_TripMap[i]))
        {          
          //printlog("input%d occur",i-7);
          turnned[i]=1;
        }
      }		  
    }
    else
    {
      turnned[i]=0;
    }
  }
  for(i=NonElectro_Nbr;i<16;i++)
  {
    if(ON==DiStatus_DI[i].Value)
    {
      if(turnned[i]==OFF)
      {
        soe_engine(i,ON);
        turnned[i]=ON;
      }		  
    }
    else if(OFF==DiStatus_DI[i].Value)
    {
      if(turnned[i]==ON)
      {
        soe_engine(i,OFF);
        turnned[i]=OFF;
      }	
    }
  }
#endif
  
#ifdef REARDITRIP
  for(i=16-NonElectro_Nbr;i<16;i++)
  {
    if(1==DiStatus_DI[i].Value)
    {
      if(turnned[i]==0)
      {
        if( DoExecute(NonElectr_TripMap[i]) )
        {
          //printlog("input%d occur",i-7);
          turnned[i]=1;
        }
      }		  
    }
    else
    {
      turnned[i]=0;
    }
  }
  for(i=0;i<16-NonElectro_Nbr;i++)
  {
    if(ON==DiStatus_DI[i].Value)
    {
      if(turnned[i]==OFF)
      {
        soe_engine(i,ON);
        turnned[i]=ON;
      }		  
    }
    else if(OFF==DiStatus_DI[i].Value)
    {
      if(turnned[i]==ON)
      {
        soe_engine(i,OFF);
        turnned[i]=OFF;
      }	
    }
  }
#endif	    
  
}

void DiScanning(void)
{
  GPIO_DealInputData(DI0,0);
  GPIO_DealInputData(DI1,1);
  GPIO_DealInputData(DI2,2);
  GPIO_DealInputData(DI3,3);
  GPIO_DealInputData(DI4,4);
  GPIO_DealInputData(DI5,5);
  GPIO_DealInputData(DI6,6);
  GPIO_DealInputData(DI7,7);
  GPIO_DealInputData(DI8,8);
  GPIO_DealInputData(DI9,9);
  GPIO_DealInputData(DI10,10);
  GPIO_DealInputData(DI11,11);
  GPIO_DealInputData(DI12,12);
  GPIO_DealInputData(DI13,13);
  GPIO_DealInputData(DI14,14);
  GPIO_DealInputData(DI15,15);
}


int fgetc(FILE *f)
{
  /* Loop until received a char */
  while(!(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET))
  {
  }
  
  /* Read a character from the USART and RETURN */
  return (USART_ReceiveData(USART1));
}

void WDGFeeding(void)
{
  IWDG_ReloadCounter();
}

//void I2CHW_Reset(void)
//{
//	__IO uint8_t Timeout;
//	uint8_t i;
//    GPIO_InitTypeDef GPIO_InitStructure; 
//    
//    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9 | GPIO_Pin_8;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
//
//	GPIO_SetBits(GPIOB,GPIO_Pin_9);
//	GPIO_SetBits(GPIOB,GPIO_Pin_8);
//
//	GPIO_WriteBit(ALARM_LED,  Bit_RESET);
//	Timeout=0xff;
//	while(Timeout--);
//	GPIO_WriteBit(ALARM_LED,  Bit_SET);
//	for(i=0;i<9;i++)
//	{
//	 GPIO_ResetBits(GPIOB,GPIO_Pin_8);
//	 Timeout=0xa0;
//	 while(Timeout--);
//	 GPIO_SetBits(GPIOB,GPIO_Pin_8);
//	 Timeout=0xa0;
//	 while(Timeout--);	
//	}
//	GPIO_WriteBit(ALARM_LED,  Bit_RESET);
//}  

void I2CHW_Reset(void)
{
  __IO uint32_t Timeout;
  uint8_t i;
  GPIO_InitTypeDef GPIO_InitStructure; 
  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
  I2C_DeInit(I2C1);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
  
  
  //  /* Enable I2C2 reset state */
  //  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
  //  /* Release I2C2 from reset state */
  //  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);
  //  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);  
  
  I2C_Configuration();
  Timeout=0x1ff;
  while(Timeout--);
  
  GPIO_WriteBit(ALARM_LED,  Bit_SET);
}   

void Get_ChipSerialNum(void)
{
  DeviceSerial[0] = *(__IO uint32_t*)(0x1FFFF7E8);
  DeviceSerial[1] = *(__IO uint32_t*)(0x1FFFF7EC);
  DeviceSerial[2] = *(__IO uint32_t*)(0x1FFFF7F0);
  macaddress[4]=(uint8_t)(DeviceSerial[2]&0xFF);
  macaddress[5]=(uint8_t)((DeviceSerial[2]>>8)&0xFF);
}

u32 Filter_Soft(u16 *pTab,u8 samnbr)
{
  u32 result=0;
  u16 tempvalue;
  u8 i,j;
  u16 calctemp[30];
  
  memcpy(calctemp,pTab,60);
  for(i=0;i<samnbr;i++)
  {
    for(j=0;j<samnbr-i-1;j++)
    {
      if(calctemp[j]<calctemp[j+1])
      {
        tempvalue=calctemp[j+1];
        calctemp[j+1]=pTab[j];
        calctemp[j]=tempvalue;
      }
    }
  }
  i=samnbr/5;
  while(i<(samnbr-samnbr/5))
    result += calctemp[i++];
  return(result/(samnbr*3/5));
  
}

float GetTemperature(void)
{
  float Temperature;
  float ValueFilted;
  u16 Refint;
  u8 i;
  
  for(i=0;i<30;i++)
  {
    SelectValueTab[i]=ADCConvertedValue[i*2];
  }
  ValueFilted=Filter_Soft(SelectValueTab,30);
  for(i=0;i<30;i++)
  {
    SelectValueTab[i]=ADCConvertedValue[i*2+1];
  }
  Refint=Filter_Soft(SelectValueTab,30);
  ValueFilted=1.2*ValueFilted/Refint;
  Temperature= (1.43 - ValueFilted)/4.3*1000 + 25;
  return Temperature;
}
void simpledelay(u32 timeout)
{
  while(timeout--);
}

void _USART1_IRQHandler(void)
{
  uint16_t bytereaded;
  static char RxCounter;
  
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
    bytereaded = USART_ReceiveData(USART1);
    
    //回显输入数据
    
    USART_SendData(USART1, bytereaded);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {
    } 
    
    if((bytereaded=='\r')||(bytereaded=='\n'))
    {
      if((uint8_t)bytereaded == '\r')
      {
        //添加换行显示
        USART_SendData(USART1, '\n');
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
      }   
      
      //调用shell处理程序
      UartProcess(UART_RxBuffer,RxCounter);
      
      //添加换行显示
      USART_SendData(USART1, '\r');
      while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
      USART_SendData(USART1, '\n');
      while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
      
      //复归接收数组计数器
      RxCounter=0;	
    }
    else
    {
      UART_RxBuffer[RxCounter++] = bytereaded;
    }
    
  }
}

void Ethernet_parameter_init(void)
{
  memset(&eth_link_count, 0, sizeof(Count_Type));
  memset(&eth_recv_count, 0, sizeof(Count_Type));
  eth_reset_flag = 0;
  
  return;
}

uint8_t EthStateCheck(void)
{
  uint8_t result = 0;
  
  if( EthLinkCheck() ) //判断是否有链接, 是否初始化
  {
    if(EthInitState == 0) //是否初始化过
    {
      //初始化 以太网
      Ethernet_Configuration();
      
      if(EthInitState == 1)
      {  
        //初始化socket端口
        CommunicationInit();
        result = 1 ;
      }
    }
    else //已经初始化完成
    {
      //判断以太网收发状态
      if(EthRecvCheck())
      {
        //是否要进行以太网复位
        if(eth_recv_count.count > SetCurrent.eth_recv_time) //大于指定的时间, 复位以太网
        {
          if(eth_recv_count.SWRST_count < ETH_SWRST_COUNT)
          {
            Set_eth_reset_flag(Ethernet_SWRST_FLAG);
            eth_recv_count.SWRST_count++;
          }
          else
          {
            Set_eth_reset_flag(Ethernet_HWRST_FLAG);
            eth_recv_count.SWRST_count = 0;
          }
        }
      }      
    }
  }
  else  //1.没有链接  2.没有链接,且以太网曾经初始化过(此种情况是否要复位)
  {
    //是否要进行以太网复位
    if(eth_link_count.count > SetCurrent.eth_link_time) //大于指定的时间, 复位以太网
    {
      Set_eth_reset_flag(Ethernet_HWRST_FLAG);
    }
  }
  
  return result;
}

uint8_t EthLinkCheck(void)
{
  uint8_t result = 0;
  struct tm t;
  
  if((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR) & PHY_Linked_Status)) 
  { 
    eth_link_count.count = 0;
    result = 1;
  }
  else
  {
    t = Time_GetCalendarTime();
    
    if(eth_link_count.t_begin.tm_year == 0)
      eth_link_count.t_begin = t;
    else
    {
      eth_link_count.t_now = t;
      
      //      if( (eth_link_count.t_now.tm_mon > eth_link_count.t_begin.tm_mon)
      //         ||(eth_link_count.t_now.tm_mday > eth_link_count.t_begin.tm_mday) )
      if( (eth_link_count.t_now.tm_hour > eth_link_count.t_begin.tm_hour)
         ||(eth_link_count.t_now.tm_min > eth_link_count.t_begin.tm_min) )      
      {
        eth_link_count.count++;
        eth_link_count.t_begin = eth_link_count.t_now;
      }
    }
  }
  
  return result;
}

uint8_t EthRecvCheck(void)
{
  uint8_t result = 0;
  struct tm t;
  
  t = Time_GetCalendarTime();
  
  if(eth_recv_count.t_begin.tm_year == 0)
    eth_recv_count.t_begin = t;
  else
  {
    eth_recv_count.t_now = t;
    
    //    if( (eth_recv_count.t_now.tm_mon > eth_recv_count.t_begin.tm_mon)
    //       ||(eth_recv_count.t_now.tm_mday > eth_recv_count.t_begin.tm_mday) )
    if( (eth_recv_count.t_now.tm_hour > eth_recv_count.t_begin.tm_hour)
       ||(eth_recv_count.t_now.tm_min > eth_recv_count.t_begin.tm_min) )    
    {
      eth_recv_count.count++;
      eth_recv_count.t_begin = eth_recv_count.t_now;
      
      result = 1;
    }
  }
  
  return result;
}

void CloseEth(void)
{
  err_t err;
  
  err = tcp_close(hello_pcb);
  
  //udp_remove(pad_pcb);
  
#ifdef TCPMODBUSEN
  err = tcp_close(modbus_pcb);
  udp_timing_close(timing_upcb);
#endif
  
#ifdef PT103EN
  //err = tcp_close(ns103_pcb);
#endif 
  
  tcp_close_all_active();
  
  LwIP_Close();
  
  return;
}

void CommunicationInit(void)
{
  LwIP_Init(); 
  
  hello_pcb = HelloWorld_init();
  
  //pad_pcb = Pad_client_init();
  
#ifdef TCPMODBUSEN
  modbus_pcb = main_TcpModbus_init();
  timing_upcb = main_udp_timing_init();
#endif
  
#ifdef PT103EN
  //ns103_pcb = lw103_client_init();
#endif 
  
  //printf(" ******* CommunicationInit() <hello_pcb:0x%X> <pad_pcb:0x%X> <modbus_pcb:0x%X> *******\r\n",
  //       hello_pcb, pad_pcb, modbus_pcb);
  
  return;
}

void Ethernet_SWRST()
{
  if(EthInitState == 1)
  {
    CloseEth();
    Delay(10);
  }
  
  EthInitState = 0;  
  Ethernet_parameter_init();
  
  printf(" ******* 以太网[软件]复位完成, 等待重新初始化... *******\r\n");
  
  return;
}

void Ethernet_HWRST()
{
  Ethernet_SWRST();
  
  //启用硬件复位
  GPIO_WriteBit(ETH_RESET,  Bit_RESET);
  Delay(100);
  GPIO_WriteBit(ETH_RESET,  Bit_SET);
  Delay(1000);  
  
  printf(" ******* 以太网[硬件]复位完成, 等待连接信号重新初始化... *******\r\n");
}


void Udp_timing_test()
{
  udp_test_init();
  
  return;
}

uint8_t Set_eth_reset_flag(const uint8_t flag)
{
  uint8_t result = 0;
  
  eth_reset_flag = flag;
  result = 1;
  
  return result;
}

uint8_t Get_eth_reset_flag()
{
  uint8_t flag = 0;
  
  flag = eth_reset_flag;
  
  return flag;  
}

uint8_t Reset_eth_recv_count()
{
  eth_recv_count.count = 0;
  eth_recv_count.SWRST_count = 0;
  
  return 1;
}


uint8_t Check_i2c_State()
{
  uint8_t result = 0;
  
  if(I2C1->SR2 & 0x0002)
    result = 1;
    
  return result;
}














//void I2C_Configuration_cpal(void)
//{
///* Configure the device structure */
//  CPAL_I2C_StructInit(&I2C1_DevStructure);      /* Set all fields to default values */
//  
//  I2C_DevStructure.CPAL_Mode = CPAL_MODE_MASTER;
//  
//#ifdef CPAL_I2C_DMA_PROGMODEL
//  I2C_DevStructure.wCPAL_Options =  CPAL_OPT_NO_MEM_ADDR | CPAL_OPT_DMATX_TCIT | CPAL_OPT_DMARX_TCIT;
//  I2C_DevStructure.CPAL_ProgModel = CPAL_PROGMODEL_DMA;
//#elif defined (CPAL_I2C_IT_PROGMODEL)
//  I2C_DevStructure.wCPAL_Options =  CPAL_OPT_NO_MEM_ADDR;
//  I2C_DevStructure.CPAL_ProgModel = CPAL_PROGMODEL_INTERRUPT;
//#else
// #error "Please select one of the programming model (in main.h)"
//#endif
//  
//  I2C_DevStructure.pCPAL_I2C_Struct->I2C_ClockSpeed = I2C_SPEED;
//  I2C_DevStructure.pCPAL_I2C_Struct->I2C_OwnAddress1 = OWN_ADDRESS;
//  ///I2C_DevStructure.pCPAL_TransferRx = &sRxStructure;
//  //I2C_DevStructure.pCPAL_TransferTx = &sTxStructure;
//  
//  /* Initialize CPAL device with the selected parameters */
//  CPAL_I2C_Init(&I2C_DevStructure);   
//}


