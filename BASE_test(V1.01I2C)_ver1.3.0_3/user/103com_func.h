#ifndef __103comfunc_h
#define __103comfunc_h
#include "bsp.h"
#include "soe.h"

#define ASDU10 10
#define ASDU21 21
#define ASDU7 7
#define ASDU20 20
#define ASDU6 6
#define ASDU5 5

u8 ANSWER_ASDU20(char *ReadBuf,char *WriteBuf);
u8 ANSWER_ASDU7(char *ReadBuf,char *WriteBuf);
u8 ANSWER_ASDU10(char *ReadBuf,char *WriteBuf);

#endif