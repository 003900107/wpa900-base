#ifndef __CANBUS_H
#define __CANBUS_H

#define TI_RES 0xD1
#define COM_DI 0xE1
#define COM_AI 0xE2
#define TESTLED 0x99
#define TI_CAL 0x50

void _CAN1_RX0_IRQHandler(void);
void _CAN2_RX0_IRQHandler(void);
void CanTextPreceive(uint8_t *TextPtr);
void ComAiProcess(uint8_t AiNbr);
void ComDiProcess(uint8_t DidNbr);
void TiProcess(uint8_t* pRxIndex);
void TestProcess(uint8_t LedNbr);
void TestTransmit(uint8_t LedNbr);
float Convert_mV2mA(uint8_t i);
void Can_Ticalib(CAN_TypeDef* CANx);
uint8_t CAN_GetLastErrorCode(CAN_TypeDef* CANx);
#endif


