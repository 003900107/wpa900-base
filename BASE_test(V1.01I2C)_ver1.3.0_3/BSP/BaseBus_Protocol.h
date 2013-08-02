#ifndef BASEBUSPROTCOL
#define BASEBUSPROTCOL


#include "stm32f10x_i2c.h"
#include "bsp.h"
#include "shellcmd.h"
#include "I2CRoutines.h"
#include <stdbool.h>

#define AI_SET 0XC4
#define AI_COF 0XC3
#define AI_RES 0XC2
#define AI_CALB 0XC6
#define AI_TIME 0XCA
#define AI_QRY 0xC1
#define DO_EXE 0xB3
#define DO_CHK 0xB2
#define DO_SEL 0xB1
#define MEANUM 24

#define MEACALLMASK 3

#define DMA	1
#define POLLING	2
#define INTERRUPT 3
#define I2C_METHOD POLLING

#define MEMBLKCP  1
#define SINGLEBYTE  2
#define MEAUPDATE_METHOD MEMBLKCP

#define CR1_PE_Set              ((uint16_t)0x0001)
#define CR1_PE_Reset            ((uint16_t)0xFFFE)

void _I2C1_EV_IRQHandler(void);
bool DoExecute(unsigned char DoSeq);
void DoSelect(unsigned char DoSeq);
#if MEAUPDATE_METHOD==SINGLEBYTE
void AiProcess(unsigned char AiSeq);
#endif
#if MEAUPDATE_METHOD==MEMBLKCP
void AiProcess(unsigned char *pDatacopied);
#endif
bool AiQuerry(void);
bool AiCoefSet(uint8_t AiSeq,uint32_t coef);
bool Calibration(void); 
bool Time_Calib(void);
uint8_t i2c_buffer_read(void);
uint8_t i2c_buffer_write(void);

void Deal_I2CComming(void);
bool BinSemPend(uint8_t Sem);
void BinSemPost(uint8_t Sem);

void I2CHW_Maintain(void);

#endif
