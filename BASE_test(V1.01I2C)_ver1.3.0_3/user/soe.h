#ifndef __SOE_H
#define __SOE_H
#include <time.h>

typedef struct tag_SOEFLAG
{
	unsigned int state:1;	
	unsigned int p103trig:1;	
	unsigned int PADtrig:1;
	unsigned int termtrig:1;	
	unsigned int reserved:4;	
}soeflag;

typedef struct tag_SOESTRUCT
{	
	u8 state;
	u8 diseq;
	soeflag flag;
	time_t timetamp;
	u16 mscd;
	
}SOE_Struct;

typedef struct tag_ERRORSTRUCT
{	
	u32 Perirh_baseaddr;
	u32 Register_1;
	u32 Register_2;
	u32 Register_3;
	u32 Register_4;
	time_t timetamp;
	u16 mscd;
}ERROR_Struct;

void soe_engine( u8 i, u8 state);
void Soe_enqueue(SOE_Struct *insert);
void Soe_tirg(SOE_Struct *trig);
void error_enqueue(ERROR_Struct *insert);
void error_engine(uint32_t PERH_ZONE);
#endif


/******************* 风电工程部chinuxer *****la fin de document****/
