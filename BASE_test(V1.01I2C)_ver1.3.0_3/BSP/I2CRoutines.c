/**
******************************************************************************
神为爱他的人所预备的是眼睛未曾看见的，耳朵未曾听见的，人心所未曾想过的
——————哥林多前
/* Includes ------------------------------------------------------------------*/
#include "I2CRoutines.h"
#include "BaseBus_Protocol.h"

/** @addtogroup Optimized I2C examples
* @{
*/


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

__IO uint32_t NumbOfBytes;

__IO uint8_t Address;

extern unsigned char TxBuffer[8];
#if MEAUPDATE_METHOD == SINGLEBYTE
extern unsigned char RxBuffer[8];
#endif
#if MEAUPDATE_METHOD == MEMBLKCP
extern unsigned char RxBuffer[127];
#endif
extern __IO uint8_t MasterReceptionComplete;
__IO uint8_t Tx_Idx=0;
__IO uint8_t Rx_Idx=0;

__IO uint32_t I2CDirection = I2C_DIRECTION_TX;
__IO uint8_t BusErrorCnt;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
* @brief  Reads buffer of bytes  from the slave.
* @param pBuffer: Buffer of bytes to be read from the slave.
* @param NumByteToRead: Number of bytes to be read by the Master.
* @param Mode: Polling or DMA or Interrupt having the highest priority in the application.
* @param SlaveAddress: The address of the slave to be addressed by the Master.
* @retval : None.
*/

void I2CDMA_Set(uint8_t* pBuffer,uint32_t BufferSize, uint8_t Direction)
{  
  DMA_InitTypeDef  I2CDMA_InitStructure;
  
  I2CDMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuffer;
  I2CDMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;
  
  if (Direction == I2C_DIRECTION_TX)
  {
    I2CDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    
    DMA_Cmd(DMA1_Channel6, DISABLE);
    DMA_Init(DMA1_Channel6, &I2CDMA_InitStructure);
    DMA_Cmd(DMA1_Channel6, ENABLE);
  }
  else 
  {
    I2CDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_Init(DMA1_Channel7, &I2CDMA_InitStructure);
    DMA_Cmd(DMA1_Channel7, ENABLE);
  } 
}
Status I2C_Master_BufferRead(uint8_t* pBuffer,  uint32_t NumByteToRead, uint8_t SlaveAddress)
{
  __IO uint32_t temp = 0;
  __IO uint32_t Timeout = 0;
  
  /* Enable I2C errors interrupts (used in all modes: Polling, DMA and Interrupts */
  I2C1->CR2 |= I2C_IT_ERR;
  
#if I2C_METHOD == DMA /* I2C1 Master Reception using DMA */
  {
    /* Configure I2C1 DMA channel */
    I2CDMA_Set(pBuffer,NumByteToRead, I2C_DIRECTION_RX);
    /* Set Last bit to have a NACK on the last received byte */
    I2C1->CR2 |= CR2_LAST_Set;	  //该位在主接收模式使用， 使得在最后一次接收数据时可以产生一个NACK。
    /* Enable I2C DMA requests */
    I2C1->CR2 |= CR2_DMAEN_Set;
    /* Send START condition */
    I2C1->CR1 |= CR1_START_Set;
    /* Wait until SB flag is set: EV5  */
    Timeout = 0xFFFF;
    while ((I2C1->SR1&0x0001) != 0x0001)
    {
      if (Timeout-- == 0)
        return Error;
    }
    Timeout = 0xFFFF;
    /* Send slave address */
    /* Set the address bit0 for read */
    SlaveAddress |= OAR1_ADD0_Set;
    Address = SlaveAddress;
    /* Send the slave address */
    I2C1->DR = Address;
    /* Wait until ADDR is set: EV6 */
    while ((I2C1->SR1&0x0002) != 0x0002)
    {
      if (Timeout-- == 0)
        return Error;
    }
    /* Clear ADDR flag by reading SR2 register */
    temp = I2C1->SR2;
    //	Timeout = 0xFFFF;
    while (!DMA_GetFlagStatus(DMA1_FLAG_TC7));
    // {
    // if (Timeout-- == 0)
    //     return Error;
    // }
    DMA_Cmd(DMA1_Channel7, DISABLE);
    
    DMA_ClearFlag(DMA1_FLAG_TC7);
    
    /* Program the STOP */
    I2C1->CR1 |= CR1_STOP_Set;
    /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
    Timeout = 0xFFFF;
    while ((I2C1->CR1&0x200) == 0x200);
    {
      if (Timeout-- == 0)
        return Error;
    }
    Deal_I2CComming();
  }
#endif
#if I2C_METHOD == POLLING /* I2C1 Master Reception using Polling */
  { 
    Timeout = 0xFFFF;
    /* Send START condition */
    I2C1->CR1 |= CR1_START_Set;
    /* Wait until SB flag is set: EV5 */
    while ((I2C1->SR1&0x0001) != 0x0001)
    {
      if (Timeout-- == 0)
        return Error;
    }
    Timeout = 0xFFFF;
    /* Send slave address */
    /* Reset the address bit0 for write */
    SlaveAddress |= OAR1_ADD0_Set;;
    Address = SlaveAddress;
    /* Send the slave address */
    I2C1->DR = Address;
    /* Wait until ADDR is set: EV6 */
    while ((I2C1->SR1&0x0002) != 0x0002)
    {
      if (Timeout-- == 0)
        return Error;
    }
    /* Clear ADDR by reading SR2 status register */
    temp = I2C1->SR2;
    /* While there is data to be read */
    while (NumByteToRead)
    {
      /* Receive bytes from first byte until byte N-3 */
      if (NumByteToRead != 3)
      {
        /* Poll on BTF to receive data because in polling mode we can not guarantee the
        EV7 software sequence is managed before the current byte transfer completes */
        Timeout=0xFFFF;
        while ((I2C1->SR1 & 0x00004) != 0x000004)
        {
          if (Timeout-- == 0)
            return Error;
        }
        /* Read data */
        *pBuffer = I2C1->DR;
        /* */
        pBuffer++;
        /* Decrement the read bytes counter */
        NumByteToRead--;
      }
      
      /* it remains to read three data: data N-2, data N-1, Data N */
      if (NumByteToRead == 3)	 
        //还剩三字节未读时，关ACK，关中断，读n-2字节，置STOP，读n-1字节，开中断，等字节到达DR，读n字节
        //为何不在读完倒数第二字节时候关ACK，读完所有之后stop？实践证明不提前关ACK,读完所有字节后依然会回ACK给从设备，
        //从设备会继续产生clk，送出一个FF值字节（因为sda此时为高）
      {
        
        /* Wait until BTF is set: Data N-2 in DR and data N -1 in shift register */
        while ((I2C1->SR1 & 0x00004) != 0x000004)
          Timeout-=0xFFFF;
        {
          if (Timeout-- == 0)
            return Error;
        }
        /* Clear ACK */
        I2C1->CR1 &= CR1_ACK_Reset;
        
        /* Disable IRQs around data reading and STOP programming because of the
        limitation ? */
        __disable_irq();
        /* Read Data N-2 */
        *pBuffer = I2C1->DR;
        /* Increment */
        pBuffer++;
        /* Program the STOP */
        I2C1->CR1 |= CR1_STOP_Set;
        /* Read DataN-1 */
        *pBuffer = I2C1->DR;
        /* Re-enable IRQs */
        __enable_irq();
        /* Increment */
        pBuffer++;
        /* Wait until RXNE is set (DR contains the last data) */
        Timeout=0xFFFF;
        while ((I2C1->SR1 & 0x00040) != 0x000040)
        {
          if (Timeout-- == 0)
            return Error;
        }
        /* Read DataN */
        *pBuffer = I2C1->DR;
        /* Reset the number of bytes to be read by master */
        NumByteToRead = 0;
        
      }
    }
    /* Make sure that the STOP bit is cleared by Hardware before CR1 write access */
    Timeout=0xFFFF;
    while ((I2C1->CR1&0x200) == 0x200)
    {
      if (Timeout-- == 0)
        return Error;
    }
    /* Enable Acknowledgement to be ready for another reception */
    I2C1->CR1 |= CR1_ACK_Set;
    Deal_I2CComming();
    //MasterReceptionComplete=0x01;
  }
#endif
#if I2C_METHOD == INTERRUPT  /* I2C1 Master Reception using Interrupts with highest priority in an application */
  {
    /* Enable EVT IT*/
    I2C1->CR2 |= I2C_IT_EVT;
    /* Enable BUF IT */
    I2C1->CR2 |= I2C_IT_BUF;
    /* Set the I2C direction to reception */
    I2CDirection = I2C_DIRECTION_RX;
    SlaveAddress |= OAR1_ADD0_Set;
    Address = SlaveAddress;
    NumbOfBytes = NumByteToRead;
    /* Send START condition */
    I2C1->CR1 |= CR1_START_Set;
    /* Wait until the START condition is generated on the bus: START bit is cleared by hardware */
    while ((I2C1->CR1&0x100) == 0x100);
    /* Wait until BUSY flag is reset (until a STOP is generated) */
    while ((I2C1->SR2 &0x0002) == 0x0002);
    /* Enable Acknowledgement to be ready for another reception */
    I2C1->CR1 |= CR1_ACK_Set;
  }
#endif
  return Success;
}



/**
* @brief  Writes buffer of bytes.
* @param pBuffer: Buffer of bytes to be sent to the slave.
* @param NumByteToWrite: Number of bytes to be sent by the Master.
* @param Mode: Polling or DMA or Interrupt having the highest priority in the application.
* @param SlaveAddress: The address of the slave to be addressed by the Master.
* @retval : None.
*/
Status I2C_Master_BufferWrite(uint8_t* pBuffer,  uint32_t NumByteToWrite, uint8_t SlaveAddress )
{
  
  __IO uint32_t temp = 0;
  __IO uint32_t Timeout = 0;
  
  /* Enable Error IT (used in all modes: DMA, Polling and Interrupts */
  I2C1->CR2 |= I2C_IT_ERR;
#if I2C_METHOD == DMA   /* I2C1 Master Transmission using DMA */
  {
    Timeout = 0xFFFF;
    /* Configure the DMA channel for I2C1 transmission */
    I2CDMA_Set (pBuffer,NumByteToWrite, I2C_DIRECTION_TX);
    /* Enable the I2C1 DMA requests */
    I2C1->CR2 |= CR2_DMAEN_Set;
    /* Send START condition */
    I2C1->CR1 |= CR1_START_Set;
    /* Wait until SB flag is set: EV5 */
    while ((I2C1->SR1&0x0001) != 0x0001)
    {
      if (Timeout-- == 0)
        return Error;
    }
    Timeout = 0xFFFF;
    /* Send slave address */
    /* Reset the address bit0 for write */
    SlaveAddress &= OAR1_ADD0_Reset;
    Address = SlaveAddress;
    /* Send the slave address */
    I2C1->DR = Address;
    /* Wait until ADDR is set: EV6 */
    while ((I2C1->SR1&0x0002) != 0x0002)
    {
      if (Timeout-- == 0)
        return Error;
    }
    
    /* Clear ADDR flag by reading SR2 register */
    temp = I2C1->SR2;
    while (!DMA_GetFlagStatus(DMA1_FLAG_TC6));
    DMA_Cmd(DMA1_Channel6, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_TC6);
    
    
    
    /* EV8_2: Wait until BTF is set before programming the STOP */
    while ((I2C1->SR1 & 0x00004) != 0x000004);
    /* Program the STOP */
    I2C1->CR1 |= CR1_STOP_Set;
    /* Make sure that the STOP bit is cleared by Hardware */
    while ((I2C1->CR1&0x200) == 0x200);
    
  }
#endif    
#if I2C_METHOD == POLLING /* I2C1 Master Transmission using Polling */
  {
    
    Timeout = 0xFFFF;
    /* Send START condition */
    I2C1->CR1 |= CR1_START_Set;
    /* Wait until SB flag is set: EV5 */
    while ((I2C1->SR1&0x0001) != 0x0001)
    {
      if (Timeout-- == 0)
        return Error;
    }
    
    /* Send slave address */
    /* Reset the address bit0 for write*/
    SlaveAddress &= OAR1_ADD0_Reset;
    Address = SlaveAddress;
    /* Send the slave address */
    I2C1->DR = Address;
    Timeout = 0xFFFF;
    /* Wait until ADDR is set: EV6 */
    while ((I2C1->SR1 &0x0002) != 0x0002)
    {
      if (Timeout-- == 0)
        return Error;
    }
    
    /* Clear ADDR flag by reading SR2 register */
    temp = I2C1->SR2;
    /* Write the first data in DR register (EV8_1) */
    I2C1->DR = *pBuffer;
    /* Increment */
    pBuffer++;
    /* Decrement the number of bytes to be written */
    NumByteToWrite--;
    /* While there is data to be written */
    while (NumByteToWrite--)
    {
      /* Poll on BTF to receive data because in polling mode we can not guarantee the
      EV8 software sequence is managed before the current byte transfer completes */
      Timeout = 0xFFFF;
      while ((I2C1->SR1 & 0x00004) != 0x000004)
      {
        if (Timeout-- == 0)
          return Error;
      }
      
      /* Send the current byte */
      I2C1->DR = *pBuffer;
      /* Point to the next byte to be written */
      pBuffer++;
    }
    /* EV8_2: Wait until BTF is set before programming the STOP */
    Timeout = 0xFFFF;
    while ((I2C1->SR1 & 0x00004) != 0x000004)
    {
      if (Timeout-- == 0)
        return Error;
    }
    /* Send STOP condition */
    I2C1->CR1 |= CR1_STOP_Set;
    /* Make sure that the STOP bit is cleared by Hardware */
    Timeout = 0xFFFF;
    while ((I2C1->CR1&0x200) == 0x200)
    {
      if (Timeout-- == 0)
        return Error;
    }
    
  }
#endif
#if I2C_METHOD == INTERRUPT /* I2C1 Master Transmission using Interrupt with highest priority in the application */
  
  {
    /* Enable EVT IT*/
    I2C1->CR2 |= I2C_IT_EVT;
    /* Enable BUF IT */
    I2C1->CR2 |= I2C_IT_BUF;
    /* Set the I2C direction to Transmission */
    I2CDirection = I2C_DIRECTION_TX;
    SlaveAddress &= OAR1_ADD0_Reset;
    Address = SlaveAddress;
    NumbOfBytes = NumByteToRead;
    /* Send START condition */
    I2C1->CR1 |= CR1_START_Set;
    /* Wait until the START condition is generated on the bus: the START bit is cleared by hardware */
    while ((I2C1->CR1&0x100) == 0x100);
    /* Wait until BUSY flag is reset: a STOP has been generated on the bus signaling the end
    of transmission */
    while ((I2C1->SR2 &0x0002) == 0x0002);
  }
#endif
  return Success;
  
}


/**
* @brief Prepares the I2C1 slave for transmission.
* @param I2C1: I2C1 or I2C2.
* @param Mode: DMA or Interrupt having the highest priority in the application.
* @retval : None.
*/

void _I2C1_EV_IRQHandler(void)
{
  __IO uint32_t SR1Register =0;
  __IO uint32_t SR2Register =0;
  
  /* Read the I2C1 SR1 and SR2 status registers */
  SR1Register = I2C1->SR1;
  SR2Register = I2C1->SR2;
  
  /* If I2C1 is slave (MSL flag = 0) */
  if ((SR2Register &0x0001) != 0x0001)
  {
    /* If ADDR = 1: EV1 */
    if ((SR1Register & 0x0002) == 0x0002)
    {
      /* Clear SR1Register and SR2Register variables to prepare for next IT */
      SR1Register = 0;
      SR2Register = 0;
      /* Initialize the transmit/receive counters for next transmission/reception
      using Interrupt  */
      Tx_Idx = 0;
      Rx_Idx = 0;
    }
    /* If TXE = 1: EV3 */
    if ((SR1Register & 0x0080) == 0x0080)
    {
      /* Write data in data register */
      I2C1->DR = TxBuffer[Tx_Idx++];
      SR1Register = 0;
      SR2Register = 0;
    }
    /* If RXNE = 1: EV2 */
    if ((SR1Register & 0x0040) == 0x0040)
    {
      /* Read data from data register */
      RxBuffer[Rx_Idx++] = I2C1->DR;
      SR1Register = 0;
      SR2Register = 0;
      
    }
    /* If STOPF =1: EV4 (Slave has detected a STOP condition on the bus */
    if (( SR1Register & 0x0010) == 0x0010)
    {
      I2C1->CR1 |= CR1_PE_Set;
      SR1Register = 0;
      SR2Register = 0;
      
    }
  } /* End slave mode */
  
  
  /* If SB = 1, I2C1 master sent a START on the bus: EV5) */
  if ((SR1Register &0x0001) == 0x0001)
  {
    /* Send the slave address for transmssion or for reception (according to the configured value
    in the write master write routine */
    I2C1->DR = Address;
    SR1Register = 0;
    SR2Register = 0;
  }
  /* If I2C1 is Master (MSL flag = 1) */
  
  if ((SR2Register &0x0001) == 0x0001)
  {
    /* If ADDR = 1, EV6 */
    if ((SR1Register &0x0002) == 0x0002)
    {
      /* Write the first data in case the Master is Transmitter */
      if (I2CDirection == I2C_DIRECTION_TX)
      {
        /* Initialize the Transmit counter */
        Tx_Idx = 0;
        /* Write the first data in the data register */
        I2C1->DR = TxBuffer[Tx_Idx++];
        /* Decrement the number of bytes to be written */
        NumbOfBytes--;
        /* If no further data to be sent, disable the I2C BUF IT
        in order to not have a TxE  interrupt */
        if (NumbOfBytes == 0)
        {
          I2C1->CR2 &= (uint16_t)~I2C_IT_BUF;
        }
        
      }
      else  /* Master Receiver */
      {
        /* Initialize Receive counter */
        Rx_Idx = 0;
        /* At this stage, ADDR is cleared because both SR1 and SR2 were read.*/
        /* EV6_1: used for single byte reception. The ACK disable and the STOP
        Programming should be done just after ADDR is cleared. */
        if (NumbOfBytes == 1)
        {
          /* Clear ACK */
          I2C1->CR1 &= CR1_ACK_Reset;
          /* Program the STOP */
          I2C1->CR1 |= CR1_STOP_Set;
        }
      }
      
      SR1Register = 0;
      SR2Register = 0;
    }
    
    /* Master transmits the remaing data: from data2 until the last one.  */
    /* If TXE is set */
    if ((SR1Register &0x0084) == 0x0080)
    {
      /* If there is still data to write */
      if (NumbOfBytes!=0)
      {
        /* Write the data in DR register */
        I2C1->DR = TxBuffer[Tx_Idx++];
        /* Decrment the number of data to be written */
        NumbOfBytes--;
        /* If  no data remains to write, disable the BUF IT in order
        to not have again a TxE interrupt. */
        if (NumbOfBytes == 0)
        {
          /* Disable the BUF IT */
          I2C1->CR2 &= (uint16_t)~I2C_IT_BUF;
        }
      }
      SR1Register = 0;
      SR2Register = 0;
    }
    
    /* If BTF and TXE are set (EV8_2), program the STOP */
    if ((SR1Register &0x0084) == 0x0084)
    {
      /* Program the STOP */
      I2C1->CR1 |= CR1_STOP_Set;
      /* Disable EVT IT In order to not have again a BTF IT */
      I2C1->CR2 &= (uint16_t)~I2C_IT_EVT;
      SR1Register = 0;
      SR2Register = 0;
    }
    
    /* If RXNE is set */
    if ((SR1Register &0x0040) == 0x0040)
    {
      /* Read the data register */
      RxBuffer[Rx_Idx++] = I2C1->DR;
      /* Decrement the number of bytes to be read */
      NumbOfBytes--;
      /* If it remains only one byte to read, disable ACK and program the STOP (EV7_1) */
      if (NumbOfBytes == 1)
      {
        /* Clear ACK */
        I2C1->CR1 &= CR1_ACK_Reset;
        /* Program the STOP */
        I2C1->CR1 |= CR1_STOP_Set;
      }
      SR1Register = 0;
      SR2Register = 0;
    }
  }
}


void _I2C1_ER_IRQHandler(void)
{
  
  __IO uint32_t SR1Register =0;
  
  /* Read the I2C1 status register */
  SR1Register = I2C1->SR1;
  /* If AF = 1 */
  if ((SR1Register & 0x0400) == 0x0400)
  {
    I2C1->SR1 &= 0xFBFF;
    SR1Register = 0;
  }
  /* If ARLO = 1 */
  if ((SR1Register & 0x0200) == 0x0200)
  {
    I2C1->SR1 &= 0xFBFF;
    SR1Register = 0;
  }
  /* If BERR = 1 */
  if ((SR1Register & 0x0100) == 0x0100)
  {
    I2C1->SR1 &= 0xFEFF;
    SR1Register = 0;
  }
  
  /* If OVR = 1 */
  
  if ((SR1Register & 0x0800) == 0x0800)
  {
    I2C1->SR1 &= 0xF7FF;
    SR1Register = 0;
  }
}

/**
* @}
*/


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
