/**
******************************************************************************
* @file    ADC/3ADCs_DMA/stm32f10x_it.c 
* @author  MCD Application Team
* @version V3.4.0
* @date    10/15/2010
* @brief   Main Interrupt Service Routines.
*          This file provides template for all exceptions handler and peripherals
*          interrupt service routine.
******************************************************************************
* @copy
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
*/ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "stm32_eth.h"
#include "I2CRoutines.h"
#include "bsp.h"

void Time_Update(void);
void DiScanning(void);
void _CAN1_RX0_IRQHandler(void);
void _CAN2_RX0_IRQHandler(void);
void LwIP_Pkt_Handle(void);

extern I2C_InitTypeDef I2C_InitStruct;
/** @addtogroup STM32F10x_StdPeriph_Examples
* @{
*/

/** @addtogroup ADC_3ADCs_DMA
* @{
*/ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/



/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
* @brief  This function handles NMI exception.
* @param  None
* @retval None
*/
void NMI_Handler(void)
{
}

/**
* @brief  This function handles Hard Fault exception.
* @param  None
* @retval None
*/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
* @brief  This function handles Memory Manage exception.
* @param  None
* @retval None
*/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
* @brief  This function handles Bus Fault exception.
* @param  None
* @retval None
*/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
* @brief  This function handles Usage Fault exception.
* @param  None
* @retval None
*/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
* @brief  This function handles SVCall exception.
* @param  None
* @retval None
*/
void SVC_Handler(void)
{
}

/**
* @brief  This function handles Debug Monitor exception.
* @param  None
* @retval None
*/
void DebugMon_Handler(void)
{
}

/**
* @brief  This function handles PendSV_Handler exception.
* @param  None
* @retval None
*/
void PendSV_Handler(void)
{
}

/**
* @brief  This function handles SysTick Handler.
* @param  None
* @retval None
*/
void SysTick_Handler(void)
{
  Time_Update(); 
  DiScanning();
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/*******************************************************************************
* Function Name  : I2C1_ER_IRQHandler
* Description    : This function handles I2C1 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_ER_IRQHandler(void)
{
  //	 if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF))
  //    {
  //        I2C_ClearFlag(I2C1, I2C_FLAG_AF);
  //
  //    }
  //
  //    if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BERR))
  //    {
  //        I2C_ClearFlag(I2C1, I2C_FLAG_BERR);
  //
  //    }
  //
  //	//I2C_ClearITPendingBit(I2C2, );
  //	I2C_Cmd(I2C1, DISABLE);
  //	I2C_DeInit(I2C1);
  //	I2C_Init(I2C1,&I2C_InitStruct);	
  //	I2C_Cmd(I2C1, ENABLE);
  _I2C1_ER_IRQHandler();
}

/**
* @brief  This function handles I2C1_EV interrupts requests.
* @param  None
* @retval None
*/

void I2C1_EV_IRQHandler(void)
{
#if I2C_METHOD==INTERRUPT
  _I2C1_EV_IRQHandler();
#endif
}

/*
* @brief  This function handles CAN1 RX0 Handler.
* @param  None
* @retval None
*/

#ifdef CAN_APP
void CAN1_RX0_IRQHandler(void)
{
  _CAN1_RX0_IRQHandler();
}

/**
* @brief  This function handles CAN2 RX0 Handler.
* @param  None
* @retval None
*/

void CAN2_RX0_IRQHandler(void)
{
  _CAN2_RX0_IRQHandler();
}
#endif

void USART1_IRQHandler(void)
{
  _USART1_IRQHandler();
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
* @brief  This function handles ETH interrupt request.
* @param  None
* @retval None
*/
void ETH_IRQHandler(void)
{
  //uint32_t len;
  /* Handles all the received frames */
  while(ETH_GetRxPktSize() != 0) 
  {	
    Set_eth_recv_flag(1);   //设置以太网接收标志位
    
    LwIP_Pkt_Handle();
  }
  
  /* Clear the Eth DMA Rx IT pending bits */
  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}

void TIM2_IRQHandler(void)
{
  //清除TIM2的中断待处理位
  TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
  
  EthStateCheck();
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
* @brief  This function handles PPP interrupt request.
* @param  None
* @retval None
*/
/*void PPP_IRQHandler(void)
{
}*/

/**
* @}
*/ 

/**
* @}
*/ 

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
