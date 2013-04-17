#include "stm32f10x_flash.h"
#include "Realtimedb.h"
#include "stm32f10x_bkp.h"
#include "Proj_Config.h"

#define     FLASH_PAGE_SIZE         0x400         // 1K per page 
#define     FLASH_WAIT_TIMEOUT      100000 
//#define IPCHECK(a,b,c,d)  ((a==0xAC&&b==0x14&&c!=0xFF&&d!=0xFF)?1:0)
#define IPCHECK(a,b,c,d)  ((a!=0xFF&&b!=0xFF&&c!=0xFF&&d!=0xFF)?1:0)   //IP地址可以在172.20以外	 Feb.27 2013
#define MEMCHECK(a,b,c,d)  ((a!=0xFF&&b!=0xFF&&c!=0xFF&&d!=0xFF)?1:0)
Setting SetCurrent;
uint8_t DevNameStr[16]="Unnamed Device";

u8 MALLSTORAGE_Write(u8 *status, u32 FLASH_Offset, u32 *Writebuff, u16 Transfer_Length) 
{ 
  u16 i;
  if(*status == 0){ 
    for(i=0; i<Transfer_Length;i+=FLASH_PAGE_SIZE)
    { 
      if(FLASH_WaitForLastOperation(FLASH_WAIT_TIMEOUT)!=FLASH_TIMEOUT)
      { 
        FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR); 
      } 
      FLASH_ErasePage(FLASH_Offset + i); 
    } 
    for(i=0;i<Transfer_Length;i+=4){ 
      if(FLASH_WaitForLastOperation(FLASH_WAIT_TIMEOUT)!=FLASH_TIMEOUT)
      { 
        FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR); 
      } 
      FLASH_ProgramWord(FLASH_Offset + i , Writebuff[i>>2]); 
    } 
    return 1; 
  } 
  return 0; 
}
#if STORE_METHOD == FLASH_METHOD
u8 DataBase_Write(u32 FLASH_Offset, u32 *Writebuff, u16 Transfer_Length) 
{ 
  u16 i;
  u8 status=0x01;
  
  if(FLASH_WaitForLastOperation(FLASH_WAIT_TIMEOUT)!=FLASH_TIMEOUT)
  { 
    FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR); 
  } 
  FLASH_ErasePage(FLASH_Offset); 
  
  for(i=0;i<Transfer_Length;i+=4)
  { 
    if(FLASH_WaitForLastOperation(FLASH_WAIT_TIMEOUT)!=FLASH_TIMEOUT)
    { 
      FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR); 
    } 
    if(FLASH_COMPLETE==FLASH_ProgramWord(FLASH_Offset + i , Writebuff[i>>2])) 
    {
      status&=0x01;
    } 
    else
    {
      status&=0x00;
    }  
  } 
  return status;    
}
#endif
#if STORE_METHOD == BKP_METHOD
u8 DataBase_Write(u32 FLASH_Offset, u16 *Writebuff, u16 Transfer_Length) 
{
  u16 i;
  for(i=0;i<Transfer_Length;i+=2)
  { 
    BKP_WriteBackupRegister(4+i*2,Writebuff[i>>1]);
  }
}   
#endif
#if STORE_METHOD == FLASH_METHOD
void DataBase_read(u32 FLASH_Offset, u32 *Readbuff, u16 Receive_Length)
{
  u16 i;
  for(i=0;i<Receive_Length;i+=4)
  { 
    Readbuff[i>>2] = ((vu32*)(FLASH_Offset))[i>>2]; 	//定义volatile意义何在？
  } 
  
}
#endif
#if STORE_METHOD == BKP_METHOD
void DataBase_read(u32 FLASH_Offset, u16 *Readbuff, u16 Receive_Length)
{
  u16 i;
  if(Receive_Length<=42)
  {
    for(i=0;i<Receive_Length;i+=4)
    { 
      Readbuff[i>>2]=BKP_ReadBackupRegister(4+i);	
    }
  } 
}
#endif

void DataBase_Init(uint8_t *IPTab)
{
  FLASH_Unlock();
  
  PWR_BackupAccessCmd(ENABLE);
#if STORE_METHOD == FLASH_METHOD
  DataBase_read(STORAGE_ROMADDR,(u32*)(&SetCurrent),sizeof(Setting));
#endif
  
#if STORE_METHOD == BKP_METHOD
  DataBase_read(STORAGE_ROMADDR,(u16*)(&SetCurrent),sizeof(Setting));
#endif
  
  if(0xA55A==SetCurrent.Mem_Used_Check)  //assert whether the setting memory was written 
  {	
    if(IPCHECK(SetCurrent.IPaddress[0],SetCurrent.IPaddress[1],SetCurrent.IPaddress[2],SetCurrent.IPaddress[3]))
    {
      IPTab[0]=SetCurrent.IPaddress[0];
      IPTab[1]=SetCurrent.IPaddress[1];
      IPTab[2]=SetCurrent.IPaddress[2];
      IPTab[3]=SetCurrent.IPaddress[3];
    }
    if(MEMCHECK(SetCurrent.DevNameChar[0],SetCurrent.DevNameChar[4],SetCurrent.DevNameChar[8],SetCurrent.DevNameChar[12]))
      memcpy(DevNameStr,SetCurrent.DevNameChar,16);
  }
  
  if(SetCurrent.TrigSoeQueue.total>SOE_FLASH_MAXNUM)	
  {
    SetCurrent.TrigSoeQueue.total=0;
  }
  
  if(SetCurrent.ErrorQueue.total>ERR_FLASH_MAXNUM)	
  {
    SetCurrent.ErrorQueue.total=0;
  }
  
  Get_ChipSerialNum();
  
  SetCurrent.vers = VERSION;
}
