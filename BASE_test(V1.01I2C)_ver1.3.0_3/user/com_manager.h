#ifndef __COM_MANAGER_H
#define __COM_MANAGER_H

#include <time.h>
#include "bsp.h"

typedef struct tag_NetState
{
	char name[6];
	u32 foreignIP;
	u8 state;
	u8 offline_cnt;
}NetState;

u8 netstat(char *p);

 #define TERM 0
 #define P103 1
 #define PAD 2
 #define NETMBUS 3
 //#define CDT 4
 //#define WEB
#endif


/******************* 风电工程部chinuxer *****la fin de document****/
