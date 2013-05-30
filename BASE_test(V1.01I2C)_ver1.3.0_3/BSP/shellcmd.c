/*
叫你们的信不在乎人的智慧，只在乎神的大能
——————哥林多前
*/

#include "shellcmd.h"
#include "BaseBus_Protocol.h"
#include "Realtimedb.h"
#include "RTC_Time.h"
#include "CanBus.h"
#include "com_manager.h"
#include <string.h> 
#include <stdlib.h>

extern float I2C_MeasureTab[MEANUM];
extern u16 CAN_MeasureTab[4];
extern DiStatus_Type DiStatus_DI[16];
extern uint8_t DevIPAddressTab[4] ;
extern const uint8_t DefaultIP[4];
static T_MESSAGE Shell_Msg;
extern Setting SetCurrent;
extern uint8_t DevNameStr[16];
extern uint8_t Compress_Factor;
extern __IO uint8_t Can_Online;
//extern __IO struct tm time_now;
/*颜色控制定义*/
const char *const vert ="\033[0;32;40m" ;
const char *const rouge="\033[0;31;40m" ;
const char *const jaune ="\033[0;33;40m" ;
const char *const bleu="\033[0;34;40m" ;
const char *const normale ="\033[0m";
const char *const rouge_mutation ="\033[0;31;36m" ;
const char *const vert_mutation ="\033[0;32;36m" ;


uint32_t Shell_State;
const char Weekchar[7][4]=
{
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};
const MeasureItem MeaPropTab[28]=
{
  {"Ua","V",120},
  {"∠Ua","°",360},
  {"Ub","V",120},
  {"∠Ub","°",360},
  {"Uc","V",120},
  {"∠Uc","°",360},
  {"Ia","A",6},
  {"∠Ia","°",360},
  {"Ib","A",6},
  {"∠Ib","°",360},
  {"Ic","A",6},
  {"∠Ic","°",360},
  {"Uab","V",120},
  {"Ubc","V",120},
  {"Uca","V",120},
  {"Freq","Hz",60},
  {"P","W",1247},
  {"Q","Var",1247},
  {"Cos","",1},
  {"I0","A",6},
  {"I2","A",6},
  {"Ph","Wh",0},
  {"Qh","Varh",0},
  {"Thm","°C",100},
  {"Th1","mV",6000},
  {"Th2","mV",6000},
  {"Th3","mV",6000},
  {"Th4","mV",6000},
};
const DisignItem DisignTab[16]=
{
  //{"遥信开入一",0},
  //{"遥信开入二",0},
  //{"遥信开入三",0},
  //{"遥信开入四",0},
  //{"遥信开入五",0},
  //{"遥信开入六",0},
  //{"遥信开入七",0},
  //{"遥信开入八",0},
  //{"遥信开入九",0},
  //{"遥信开入十",0},
  //{"遥信开入十一",0},
  //{"遥信开入十二",0},
  //{"遥信开入十三",0},
  //{"遥信开入十四",0},
  //{"遥信开入十五",0},
  //{"遥信开入十六",0},
  {"DI1",0},
  {"DI2",0},
  {"DI3",0},
  {"DI4",0},
  {"DI5",0},
  {"DI6",0},
  {"DI7",0},
  {"DI8",0},
  {"DI9",0},
  {"DI10",0},
  {"DI11",0},
  {"DI12",0},
  {"DI13",0},
  {"DI14",0},
  {"DI15",0},
  {"DI16",0},
};

const CoefItem CoefTab[9]=
{
  {"Ua"},
  {"Ub"},
  {"Uc"},
  {"Ia"},
  {"Ib"},
  {"Ic"},
  {"Uab"},
  {"Ubc"},
  {"Uca"},
} ;

const ShellCmd CmdTable[]=
{
  {"measure",CmdShowData,0},//1
  {"setcoef",CmdQNA,0},//2
  {"di",CmdShowData,0},//3
  {"do1",CmdQNA,1},//4
  {"do2",CmdQNA,1},//5
  {"do3",CmdQNA,1},//6
  {"do4",CmdQNA,1},//7
  {"do5",CmdQNA,1},//8
  {"do6",CmdQNA,1},//9
  {"do7",CmdQNA,1},//10
  {"do8",CmdQNA,1},//11
  {"reset",CmdRestart,1},//12
  {"setaddr",CmdQNA,1},//13
  {"coef",CmdShowData,1},//14
  {"date",CmdShowData,1},//15
  {"setdate",CmdQNA,1},//16
  {"ti",CmdShowData,0},//17
  {"setname",CmdShowData,0},//18
  {"calib",CmdQNA,1},//19
  {"setpara",CmdQNA,1},//20
  {"netstat",CmdShowData,1},//21
  {"dccalib",CmdQNA,1},//22
  {"protest",CmdQNA,1},//23
  {"relaysoe",CmdShowData,1},//24
  {"almrec",CmdShowData,1},//25
  {"mac",CmdShowData,1},//26
  {"tripmap",CmdShowData,1},//27
  {"ip", CmdShowIP,0}, //28
  {"restoreip", CmdQNA, 1}, //29
  {"ethreset", CmdQNA, 1}, //30
  {"updatefirmware", CmdQNA, 1}, //31
  {"setlinktime", CmdQNA, 1}, //32
  {"setrecvtime", CmdQNA, 1}, //33
  {0,0,0}
};

char PASSWORD[]= "sac";
char SYSPW[]= "chinuxer" ;
char YES[] = "Y";
char isCom = 1;   //tyh:串口显示开关


bool ShellCmdCmp(char *a,const char *b,uint8_t len)
{
  if(0==strncmp(a,b,len))
    return (1);
  else
    return (0);
}
uint32_t ShellCmdMatch(char *a,char *b,uint8_t len)
{
  uint8_t j;
  uint32_t result=0;
  
  for(j=0;CmdTable[j].CmdStr!=0;j++) 
  {
    if (ShellCmdCmp(a,CmdTable[j].CmdStr,strlen(CmdTable[j].CmdStr)))
    {
      switch(j)
      {
      case 0:  
        Shell_Msg.m_type=CMD_AI;
        break;
      case 1: 
        Shell_Msg.m_type=CMD_COEF;
        break;
      case 2:
        Shell_Msg.m_type=CMD_DI;
        break;
      case 3:
        Shell_Msg.m_type=CMD_DO;
        Shell_Msg.m_intdata[0]=1;
        break;
      case 4:
        Shell_Msg.m_type=CMD_DO;
        Shell_Msg.m_intdata[0]=2;
        break;
      case 5:
        Shell_Msg.m_type=CMD_DO;
        Shell_Msg.m_intdata[0]=3;
        break;
      case 6:
        Shell_Msg.m_type=CMD_DO;
        Shell_Msg.m_intdata[0]=4;
        break;
      case 7:
        Shell_Msg.m_type=CMD_DO;
        Shell_Msg.m_intdata[0]=5;
        break;
      case 8:
        Shell_Msg.m_type=CMD_DO;
        Shell_Msg.m_intdata[0]=6;
        break; 
      case 9:
        Shell_Msg.m_type=CMD_DO;
        Shell_Msg.m_intdata[0]=7;
        break;
      case 10:
        Shell_Msg.m_type=CMD_DO;
        Shell_Msg.m_intdata[0]=8;
        break; 
      case 11:
        Shell_Msg.m_type=CMD_REBOOT;
        break;
      case 12:  
        Shell_Msg.m_type=CMD_ADDR; 
        break;     
      case 13:  
        Shell_Msg.m_type=CMD_COEF;
        break;
      case 14:
        //break;
      case 15:  
        Shell_Msg.m_type=CMD_DATE;
        break;
      case 16:  
        Shell_Msg.m_type=CMD_TI;
        break;
      case 17:  
        Shell_Msg.m_type=CMD_NAME;
        break;
      case 18:  
        Shell_Msg.m_type=CMD_CALB;
        break;
      case 19:  
        Shell_Msg.m_type=CMD_SET;
        break;
      case 20:  
        Shell_Msg.m_type=CMD_NET;
        break;
      case 21:  
        Shell_Msg.m_type=CMD_DCCAL;
        break;
      case 22:  
        Shell_Msg.m_type=CMD_PROT;
        break;
      case 23:  
        Shell_Msg.m_type=CMD_SOE;
        break;
      case 24:  
        Shell_Msg.m_type=CMD_REC;
        break;
        //tyh:20130312 以下命令仅用于串口  
      case 27:
        Shell_Msg.m_type=CMD_SHOWIP;
        break;
      case 28:
        Shell_Msg.m_type=CMD_RESTOREIP;
        break;
      case 29:
        Shell_Msg.m_type=CMD_RESET_ETH;
        break;
      case 30:
        Shell_Msg.m_type=CMD_UPDATE_FIRMWARE;
        break;
      case 31:
        Shell_Msg.m_type=CMD_ETH_LINK_TIME;
        break;
      case 32:
        Shell_Msg.m_type=CMD_ETH_RECV_TIME;
        break;
        
      default: 
        return 0; 
      }
      
      if(!isCom)
      {
        if( (Shell_Msg.m_type != CMD_RESTOREIP)&&(Shell_Msg.m_type != CMD_RESET_ETH) )
        {
          result=CmdTable[j].CmdFunc(b,&Shell_Msg);
          break;
        }
      }
      else
      {
        if((Shell_Msg.m_type == CMD_RESTOREIP)||(Shell_Msg.m_type == CMD_SHOWIP)
           ||(Shell_Msg.m_type == CMD_REBOOT)||(Shell_Msg.m_type == CMD_DI)
             ||(Shell_Msg.m_type == CMD_RESET_ETH)||(Shell_Msg.m_type == CMD_UPDATE_FIRMWARE))
        {
          result=CmdTable[j].CmdFunc(b,&Shell_Msg);
          break;
        }
      }
    }
  }
  
  return result;                
}

uint32_t ShellPwdMatch(char *a,char *b,uint8_t len)
{
  char *p;
  p=b;
  if (ShellCmdCmp(a,PASSWORD,strlen(PASSWORD)))
  {
    return (CmdPwdAsserted(b,&Shell_Msg));
  }
  else
  {
#if LINGUA==EN
    sprintf(b, "Password Identified Fault\r\n");
    p+=strlen("Password Identified Fault\r\n");
#endif
#if LINGUA==CH
    sprintf(b, "密码错误\r\n");
    p+=strlen("密码错误\r\n");
#endif
    Shell_State=INIT;
    return(p-b); 
  }  
}

uint32_t ShellEtrData(char *entry,char *b,uint8_t len)
{
  char *p;
  uint8_t i=0;
  uint32_t result;
  
  p=entry-1;
  result=ShellCmdMatch(entry,b,len);
  if(result)
  { 
    return result; 
  }
  else
  {
    
    do  {
      Shell_Msg.m_intdata[i++] =atoi(p+1);
    }
    while((p=strpbrk(entry," "))&&(i<len)) ;
    
    return (CmdSetting(b,&Shell_Msg));
    
  }
}

uint32_t ShellEtrStr(char *entry,char *b,uint8_t len)
{
  char *p;
  uint32_t result;
  
  
  result=ShellCmdMatch(entry,b,len);
  if(result)
  { 
    return result; 
  }
  else
  {
    do  {
      p=strpbrk(entry,"\n");
      memset (DevNameStr,0,16);
      memcpy (DevNameStr,entry,p-entry-1);
    }
    while(0) ;
    
    return (CmdSetting(b,&Shell_Msg));
  }
}

uint32_t ShellEtrAddr(char *entry,char *b,uint8_t len)
{
  uint16_t ipaddr;
  uint8_t i=0;
  char *p;
  p=entry;
  
  //20130312 TYH:添加IP命令判断
  if(Shell_Msg.m_type == CMD_RESTOREIP) //恢复默认IP
  {
    for(i=0; i<4; i++)
    {
      DevIPAddressTab[i] = DefaultIP[i];
    }
  }
  else //设置新IP
  {  
    while((p)&&(i<len))
    {
      ipaddr=atoi(p);
      if( ipaddr>255)
      { 
        p=b;
#if LINGUA==EN
        sprintf(b, "Enter Address Fault\r\n");
        p+=strlen("Enter Address Fault\r\n");
#endif
#if LINGUA==CH
        sprintf(b, "地址错误\r\n");
        p+=strlen("地址错误\r\n");
#endif
        return(p-b);  
      }
      DevIPAddressTab[i++]=ipaddr;
      p=strpbrk(p,".")+1;
    }
  }
  
  return (CmdSetting(b,&Shell_Msg));
} 


uint32_t ShellEtrDate(char *entry,char *b,uint8_t len)
{
  struct tm t;
  char *p;
  p=entry;
  
  memset(&t, 0, sizeof(t));//tyh:20130308 增加初始化为'0', 避免时间计算出错
  
  t.tm_year =atoi(p);
  p=strpbrk(p,"-")+1;
  t.tm_mon =atoi(p)/*-1*/;//tyh:20130308 去除"-1", 交由 Time_SetCalendarTime 统一处理
  p=strpbrk(p,"-")+1;
  t.tm_mday =atoi(p);
  p=strpbrk(p," ")+1;
  t.tm_hour=atoi(p);
  p=strpbrk(p,"-")+1;
  t.tm_min =atoi(p);
  p=strpbrk(p,"-")+1;
  t.tm_sec =atoi(p);
  if((t.tm_mon>11)||(t.tm_mday>31)||(t.tm_hour>23)||(t.tm_min>59)||(t.tm_sec>59))
  { 
#if LINGUA==EN
    sprintf(b, "Enter Time Fault\r\n");
    p=strlen("Enter Time Fault\r\n")+b;
#endif
#if LINGUA==CH
    sprintf(b, "时间范围错误\r\n");
    p=strlen("时间范围错误\r\n")+b;
#endif
    return (p-b);   
  }
  
  Time_SetCalendarTime(t);
  Time_Calib();
  sprintf(b, "\r\n");
  
  return 2;
}

uint32_t ShellEtrPara(char *entry,char *b,uint8_t len)
{
  char *p;
  ComPara compara;
  Setting *setaddr;
  p=entry;
  setaddr=&SetCurrent;
  
  compara.Baud =atoi(p);
  p=strpbrk(p,",")+1;
  compara.Parity =atoi(p);
  p=strpbrk(p,",")+1;
  compara.WordLength =atoi(p);
  p=strpbrk(p,",")+1;
  compara.StopBits=atoi(p);
  p=strpbrk(p,",")+1;
  compara.Protocol=atoi(p);
  p=strpbrk(p,"-")+1;
  memcpy(&(SetCurrent.Com1Para),&compara,sizeof(ComPara));
  
  compara.Baud =atoi(p);
  p=strpbrk(p,",")+1;
  compara.Parity =atoi(p);
  p=strpbrk(p,",")+1;
  compara.WordLength =atoi(p);
  p=strpbrk(p," ")+1;
  compara.StopBits=atoi(p);
  p=strpbrk(p,"-")+1;
  compara.Protocol=atoi(p);
  
  memcpy(&(SetCurrent.Com2Para),&compara,sizeof(ComPara));
  
#if STORE_METHOD == FLASH_METHOD 
  if( DataBase_Write(STORAGE_ROMADDR,(u32 *)setaddr,sizeof(Setting)))
#endif
#if STORE_METHOD == BKP_METHOD
    if( DataBase_Write(STORAGE_ROMADDR,(u16 *)setaddr,sizeof(Setting)))
#endif
    {
#if LINGUA == EN
      sprintf(b,"the new Parameters written to ROM successfully\r\n");
      p=strlen("the new Parameters written to ROM successfully\r\n")+b;
#endif
#if LINGUA == CH
      sprintf(b,"新参数写入ROM成功\r\n");
      p=strlen("新参数写入ROM成功\r\n")+b;
#endif
    }
  return (p-b);
}
uint32_t CmdShowData(char *outputstr,T_MESSAGE *message)
{
  uint8_t i;
  uint8_t status=0;
  char *p;
  struct tm time_now;
  time_t old_time;
  
  static float MeaRecordTab[MEANUM];
  static uint8_t DiRecording[16];
  
  old_time = 0;
  memset (outputstr,0,1314);
  
  p=outputstr;
  
  switch(message->m_type)
  {
  case CMD_AI:  
#if MEAUPDATE_METHOD==MEMBLKCP
    status=AiQuerry();
    if(status)
    {
      I2C_MeasureTab[23]=GetTemperature();
#endif
#if LINGUA==EN
      sprintf(p, "╔════════╦═════total meas:%3d═════╦════════╗\r\n",MEANUM);
      p+=strlen("╔════════╦═════total meas:%3d═════╦════════╗\r\n");
#endif
#if LINGUA==CH
      sprintf(p, "╔════════╦═════遥测量总数:%3d═════╦════════╗\r\n",MEANUM);
      p+=strlen("╔════════╦═════遥测量总数:%3d═════╦════════╗\r\n");
#endif
      for(i=0;i<MEANUM;i++)
      {
        if ((i+1) % 4)
        {
          if((16==i)||(17==i)||(21==i)||(22==i))
          {
            //											if((I2C_MeasureTab[i]-MeaRecordTab[i])>Compress_Factor*MeaPropTab[i].scale)
            //											   {
            //											    sprintf(p, "║%-4s:%s%6.1f%s %4s",MeaPropTab[i].Title,rouge_mutation,\
              //												I2C_MeasureTab[i],normale,MeaPropTab[i].Unit);
              //											   p+=strlen(rouge_mutation);
              //                  							   p+=strlen(normale);
              //											   }
              //											else 
              sprintf(p, "║%-4s:%6.1f %-4s",MeaPropTab[i].Title,I2C_MeasureTab[i],MeaPropTab[i].Unit);
              p+=strlen("║XXXX:01234.5XXXX");
          }
          else if(18==i)
          {
            //											if((I2C_MeasureTab[i]-MeaRecordTab[i])>Compress_Factor*MeaPropTab[i].scale)
            //											    {
            //												sprintf(p, "║%-4s:%s%6.1f %s%4s",MeaPropTab[i].Title,rouge_mutation,\
              //												I2C_MeasureTab[i],normale,MeaPropTab[i].Unit);
              //												p+=strlen(rouge_mutation);
              //                  							    p+=strlen(normale);
              //											    }
              //											else 
              sprintf(p, "║%-4s:%6.4f %-4s",MeaPropTab[i].Title,I2C_MeasureTab[i],MeaPropTab[i].Unit);
              p+=strlen("║XXXX:01.2345XXXX");
          }
          else
          {
            //											if((I2C_MeasureTab[i]-MeaRecordTab[i])>Compress_Factor*MeaPropTab[i].scale)
            //											    {
            //												sprintf(p, "║%-4s:%s%6.1f%s %4s",MeaPropTab[i].Title,rouge_mutation,\
              //												I2C_MeasureTab[i],normale,MeaPropTab[i].Unit);
              //												p+=strlen(rouge_mutation);
              //                  							    p+=strlen(normale);
              //											   }
              //											else 
              sprintf(p, "║%-4s:%6.2f %-4s",MeaPropTab[i].Title,I2C_MeasureTab[i],MeaPropTab[i].Unit);
              p+=strlen("║XXXX:0123.45XXXX");
          }
        }
        else
        {	
          //									if((I2C_MeasureTab[i]-MeaRecordTab[i])>Compress_Factor*MeaPropTab[i].scale)
          //											    {
          //												sprintf(p, "║%-4s:%s%6.1f %s%4s",MeaPropTab[i].Title,rouge_mutation,\
            //												I2C_MeasureTab[i],normale,MeaPropTab[i].Unit);
            //												p+=strlen(rouge_mutation);
            //                  							    p+=strlen(normale);
            //											    }
            //											else 
            sprintf(p, "║%-4s:%6.2f %-4s║\r\n",MeaPropTab[i].Title,I2C_MeasureTab[i],MeaPropTab[i].Unit);
            p+=strlen("║XXXX:0123.45XXXX║\r\n");
            //sprintf(p, "╠════════╬════════╬════════╬════════╣\r\n");
            if((MEANUM-1)==i)  sprintf(p, "╚════════╩════════╩════════╩════════╝\r\n");
            p+=strlen("╠════════╬════════╬════════╬════════╣\r\n");
        }
        
        MeaRecordTab[i]=I2C_MeasureTab[i];
      }
#if MEAUPDATE_METHOD==MEMBLKCP
    }
    else
    {
#if LINGUA== EN
      sprintf(outputstr,"can't read data from AI board \r\n");
      p+=strlen("can't read data from AI board \r\n");	
#endif
#if LINGUA== CH
      sprintf(outputstr,"无法读取AI数据 \r\n");
      p+=strlen("无法读取AI数据 \r\n");	
#endif
    }
#endif
    break;
    
  case CMD_DI:
#if LINGUA== EN	
    sprintf(p, "╔══totaldis%2d══╗\r\n",16);
    p+=strlen("╔══totaldis%2d══╗\r\n");
#endif
#if LINGUA ==CH	
    //sprintf(p, "╔══遥信总数%2d══╗\r\n",16);
    //p+=strlen("╔══遥信总数%2d══╗\r\n");
    
    sprintf(p, "╔══遥信总数%2d══╗",16);
    p+=strlen("╔══遥信总数%2d══╗");
    
#endif    
    for(i=0;i<8;i++)
    {  
      if(1==DiStatus_DI[i].Value)
      {
        
        sprintf(p, "║ %-3d %s◆%s ",i+1,rouge,normale);
        if(DiRecording[i]!=DiStatus_DI[i].Value)  sprintf(p, "║ %-3d %s◆%s ",i+1,rouge_mutation,normale);
        p+=strlen("║ %-3d ◆ ");
        p+=strlen(rouge);
        p+=strlen(normale);
        
      }  
      else 
      {
        
        sprintf(p, "║ %-3d %s◇%s ",i+1,vert,normale); 
        if(DiRecording[i]!=DiStatus_DI[i].Value)  sprintf(p, "║ %-3d %s◇%s ",i+1,vert_mutation,normale);
        p+=strlen("║ %-3d ◇ ");
        p+=strlen(vert);
        p+=strlen(normale);
        
      } 
      DiRecording[i]=DiStatus_DI[i].Value;                  
      
      if(1==DiStatus_DI[i+8].Value)
      {
        
        sprintf(p, "║ %-3d %s◆%s ║\r\n",i+9,rouge,normale);
        if(DiRecording[i+8]!=DiStatus_DI[i+8].Value)  sprintf(p, "║ %-3d %s◆%s ║\r\n",i+9,rouge_mutation,normale);
        p+=strlen("%║ -3d ◆ ║\r\n");
        p+=strlen(rouge);
        p+=strlen(normale);
        
      }  
      else 
      {
        
        sprintf(p, "║ %-3d %s◇%s ║\r\n",i+9,vert,normale); 
        if(DiRecording[i+8]!=DiStatus_DI[i+8].Value)  sprintf(p, "║ %-3d %s◇%s ║\r\n",i+9,vert_mutation,normale);
        p+=strlen("║ %-3d ◇ ║\r\n");
        p+=strlen(vert);
        p+=strlen(normale);
        
      }
      if(7==i)
      {
        sprintf(p, "╚════╩════╝\r\n");
        p+=strlen("╚════╩════╝\r\n");
      }
      else
      {
        sprintf(p, "╠════╬════╣\r\n");
        p+=strlen("╚════╩════╝\r\n"); 
      }
      DiRecording[i+8]=DiStatus_DI[i+8].Value;
      
    }   
    break;
    
  case CMD_TI:
    if(Can_Online)
    {  
      for(i=0;i<4;i++)
      {
        if(CAN_MeasureTab[i]>6000) sprintf(p, "%s\r\n","OffSccope");
        sprintf(p, "%s:%5d %s\r\n",MeaPropTab[24+i].Title,CAN_MeasureTab[i],MeaPropTab[24+i].Unit);
        p+=strlen("XXXXX:01234XX\r\n");
      }
#if LINGUA ==EN
      sprintf(p,"enter the number of 4~20mA channel\r\n");
      p=p+strlen("enter the number of 4~20mA channel\r\n")+strlen(DevNameStr);
#endif
#if LINGUA ==CH
      sprintf(p,"输入4～20mA通道序号查看电流显示\r\n");
      p=p+strlen("输入4～20mA通道序号查看电流显示\r\n")+strlen(DevNameStr);
#endif
      Shell_State=TI_mA;
    }
    else
    {
#if LINGUA== EN
      sprintf(outputstr,"can't read data from TI board \r\n");
      p+=strlen("can't read data from TI board \r\n");	
#endif
#if LINGUA== CH
      sprintf(outputstr,"无法读取TI数据\r\n");
      p+=strlen("无法读取TI数据\r\n");	
#endif
    }	
    break;
    
  case CMD_COEF:
    break;
    
  case CMD_DATE:
    time_now = Time_GetCalendarTime();
    sprintf(p, "%d-%02d-%02d %02d:%02d:%02d %s\r\n",time_now.tm_year,time_now.tm_mon/*+1*/,time_now.tm_mday,\
      time_now.tm_hour,time_now.tm_min,time_now.tm_sec,Weekchar[time_now.tm_wday]);
    p+=strlen("1982-05-24 21:13:14 Tue\r\n");
    break;
    
  case CMD_NAME:
#if LINGUA ==EN
    sprintf(p,"device name now:%s,enter your new name\r\n",DevNameStr);
    p=p+strlen("device name now:,enter your new name\r\n")+strlen(DevNameStr);
#endif
#if LINGUA ==CH
    sprintf(p,"当前装置名称为%s,请输入新名称\r\n",DevNameStr);
    p=p+strlen("当前装置名称为,请输入新名称\r\n")+strlen(DevNameStr);
#endif
    Shell_State=MODIFY_NAME;
    break;
    
  case CMD_NET:
    i=netstat(p);
    p+=i;
    break;
    
  case CMD_SOE:
    sprintf(p,"***************SOE事件记录***************\r\n");
    p+=strlen("***************SOE事件记录***************\r\n");
    
    if(0==SetCurrent.TrigSoeQueue.total)
    {
      sprintf(p,"无SOE事件记录 *_*||\r\n");
      p+=strlen("无SOE事件记录 *_*||\r\n");
    }
    else
    {
      for(i=0;i<SetCurrent.TrigSoeQueue.total;i++)
      {
        //tyh:20130312 判断当前显示的是否是最新记录,如果不是画线区分
        if(old_time > SetCurrent.TrigSoeQueue.SOE_Queue_Flash[i].timetamp)
        {
          sprintf(p, "~~~~~~~~~~");
          p+=10;
          sprintf(p, "~~~~~~~~~~");
          p+=10;
          sprintf(p, "~~~~~~~~~~");
          p+=10;
          sprintf(p, "~~~~~~~~~~");
          p+=10;
          sprintf(p, "~\r\n");
          p+=3;
        }
        
        time_now = Time_ConvUnixToCalendar(SetCurrent.TrigSoeQueue.SOE_Queue_Flash[i].timetamp);
        sprintf(p, "%4d-%02d-%02d %02d:%02d:%02d %4dms DI%02d非电量%4s\r\n",time_now.tm_year,time_now.tm_mon,time_now.tm_mday,\
          time_now.tm_hour,time_now.tm_min,time_now.tm_sec,SetCurrent.TrigSoeQueue.SOE_Queue_Flash[i].mscd,\
            SetCurrent.TrigSoeQueue.SOE_Queue_Flash[i].diseq+1,(SetCurrent.TrigSoeQueue.SOE_Queue_Flash[i].state?"动作":"返回"));
        p+=43;
        
        old_time = SetCurrent.TrigSoeQueue.SOE_Queue_Flash[i].timetamp;
      }
      
      old_time = 0;
    }
    break;
    
  case CMD_REC:
    sprintf(p,"***************ERROR LIST***************\r\n");
    p+=strlen("***************ERROR LIST***************\r\n");
    if(0==SetCurrent.ErrorQueue.total)
    {
      sprintf(p,"no error recorded *_*||\r\n");
      p+=strlen("no error recorded *_*||\r\n");
    }
    else
    {
      for(i=0;i<SetCurrent.ErrorQueue.total;i++)
      {
        //tyh:20130312 判断当前显示的是否是最新记录,如果不是画线区分
        if(old_time > SetCurrent.ErrorQueue.ERROR_Queue_Flash[i].timetamp)
        {
          sprintf(p, "~~~~~~~~~~~~~~~~~~~~");
          p+=20;
          sprintf(p, "~~~~~~~~~~~~~~~~~~~~");
          p+=20;
          sprintf(p, "~~~~~~~~~~~~~~~~~~~~");
          p+=20;
          sprintf(p, "~~~~~~~~~~~~~~~~~~~~");
          p+=20;
          sprintf(p, "\r\n");
          p+=2;
        }        
        
        time_now = Time_ConvUnixToCalendar(SetCurrent.ErrorQueue.ERROR_Queue_Flash[i].timetamp);
        sprintf(p, "%4d-%02d-%02d %02d:%02d:%02d %4dms %08x's Reg=|%08x|%08x|%08x|%08x|\r\n",time_now.tm_year,time_now.tm_mon/*+1*/,time_now.tm_mday,\
          time_now.tm_hour,time_now.tm_min,time_now.tm_sec,SetCurrent.ErrorQueue.ERROR_Queue_Flash[i].mscd,\
            SetCurrent.ErrorQueue.ERROR_Queue_Flash[i].Perirh_baseaddr,\
              SetCurrent.ErrorQueue.ERROR_Queue_Flash[i].Register_1,\
                SetCurrent.ErrorQueue.ERROR_Queue_Flash[i].Register_2,\
                  SetCurrent.ErrorQueue.ERROR_Queue_Flash[i].Register_3,\
                    SetCurrent.ErrorQueue.ERROR_Queue_Flash[i].Register_4);
        p+=strlen("1982-05-24 13:14:10 1234ms 12345678's Reg=|00000400|00000003|00000401|00000124|\r\n");
        
        old_time = SetCurrent.ErrorQueue.ERROR_Queue_Flash[i].timetamp;
      }
      
      old_time = 0;
    }
    break;
    
  case CMD_SHOWIP://20130312 TYH:添加IP显示命令处理
    break;
    
  default:
    break;
  }     
  
  *(p++)=0x0A;
  *(p++)=0x0D;
  
  return (p-outputstr);	
}
uint32_t CmdSetting(char *outputstr,T_MESSAGE *message)
{
  char *p;
  Setting *setaddr;
  u16 i;
  float mA;
  p=outputstr;
  setaddr=&SetCurrent;
  //uint32_t infoNum;  
  
  switch(message->m_type)
  {
  case CMD_SET:
  case CMD_COEF:
    message->m_type=CMD_NULL;
    //  memcpy(setaddr->ChannelCoef+message->m_intdata[0]*2-2,&(message->m_intdata[1]),2);
    // if( DataBase_Write(STORAGE_ROMADDR,(u32 *)setaddr,sizeof(Setting)))
    //{
    
    if (AiCoefSet(message->m_intdata[0],message->m_intdata[1]))
    {
#if LINGUA == EN
      if(10==message->m_intdata[0])
      {
        sprintf(outputstr,"all coefficient setted as 1000\r\n");
        p+=strlen("all coefficient setted as 1000\r\n"); 
      }
      else
      {
        sprintf(outputstr,"setting coefficient of [%s%s%s] is OK\r\n",vert,CoefTab[message->m_intdata[0]-1].Title,normale);
        p+=strlen("setting coefficient of [XXX] is OK\r\n");
        p+=strlen(vert);
        p+=strlen(normale);
      }
#endif
#if LINGUA == CH
      if(10==message->m_intdata[0])
      {
        sprintf(outputstr,"所有通道系数设为1000\r\n");
        p+=strlen("所有通道系数设为1000\r\n");
      }
      else
      {
        sprintf(outputstr,"通道[%s%s%s]系数设定成功\r\n",vert,CoefTab[message->m_intdata[0]-1].Title,normale);
        p+=strlen("通道[XXX]系数设定成功\r\n");
        p+=strlen(vert);
        p+=strlen(normale);
      }
#endif
      
      return (p-outputstr);
    }
    else
    {  
#if LINGUA == EN
      sprintf(outputstr,"setting operation fault\r\n");
      p+=strlen("setting operation fault\r\n");
#endif
#if LINGUA == CH
      sprintf(outputstr,"设定操作失败\r\n");
      p+=strlen("设定操作失败\r\n");
#endif
    }
    break;
    
  case CMD_PROT:
    message->m_type=CMD_NULL;
    AiCoefSet(message->m_intdata[0],message->m_intdata[1]);
    
    if(100==message->m_intdata[0])
    {
      sprintf(outputstr,"所有通道上传原始值设为2048\r\n");
      p+=strlen("所有通道上传原始值设为2048\r\n");
    }
    else
    {
      sprintf(outputstr,"通道[%s%s%s]模拟上送，请核查监控遥测值\r\n",vert,CoefTab[message->m_intdata[0]-1].Title,normale);
      p+=strlen("通道[XXX]模拟上送，请核查监控遥测值\r\n");
      p+=strlen(vert);
      p+=strlen(normale);
    }
    break;
    
  case CMD_ADDR:
    message->m_type=CMD_NULL;
    setaddr->Mem_Used_Check=0xA55A;
    memcpy(setaddr->IPaddress,DevIPAddressTab,4);
#if STORE_METHOD == FLASH_METHOD 
    if( DataBase_Write(STORAGE_ROMADDR,(u32 *)setaddr,sizeof(Setting)))
#elif STORE_METHOD == BKP_METHOD
      if( DataBase_Write(STORAGE_ROMADDR,(u16 *)setaddr,sizeof(Setting)))
#endif
      {
#if LINGUA == EN
        sprintf(outputstr,"the new IP written to ROM successfully\r\n");
        p+=strlen("the new IP written to ROM successfully\r\n");
#endif
#if LINGUA == CH
        sprintf(outputstr,"新IP写入ROM成功\r\n");
        p+=strlen("新IP写入ROM成功\r\n");
#endif
      }
      else
      {
#if LINGUA == EN
        sprintf(outputstr,"the new IP written to ROM fault\r\n");
        p+=strlen("the new IP written to ROM fault\r\n");
#endif
#if LINGUA == CH
        sprintf(outputstr,"新IP写入ROM失败\r\n");
        p+=strlen("新IP写入ROM失败\r\n");
#endif
      } 
    break; 
    
  case CMD_TI:
    message->m_type=CMD_NULL;
    if((message->m_intdata[0]<5)&&(message->m_intdata[0]>0))
    {
      for(i=1;i<5;i++)
      {
        if(message->m_intdata[0]==i) 
        {
          mA=Convert_mV2mA(i);
          sprintf(p, "%s:%5.2f %s\r\n",MeaPropTab[23+i].Title,mA,"mA");
          p+=strlen("XXXXX:01.23 XX\r\n");	
        }
        else
        {
          sprintf(p, "%s:%5d %s\r\n",MeaPropTab[23+i].Title,CAN_MeasureTab[i-1],MeaPropTab[23+i].Unit);
          p+=strlen("XXXXX:01235 XX\r\n");
        }      
      }
      
    }
    break;
    
  case CMD_NAME:
    message->m_type=CMD_NULL;
    setaddr->Mem_Used_Check=0xA55A;
    memset (setaddr->DevNameChar,0,16);
    memcpy(setaddr->DevNameChar,DevNameStr,strlen(DevNameStr));
#if STORE_METHOD == FLASH_METHOD 
    if( DataBase_Write(STORAGE_ROMADDR,(u32 *)setaddr,sizeof(Setting)))
#endif
#if STORE_METHOD == BKP_METHOD
      if( DataBase_Write(STORAGE_ROMADDR,(u16 *)setaddr,sizeof(Setting)))
#endif
      {
#if LINGUA == EN
        sprintf(outputstr,"the new name written to ROM successfully\r\n");
        p+=strlen("the new name written to ROM successfully\r\n");
#endif
#if LINGUA == CH
        sprintf(outputstr,"新名称写入ROM成功\r\n");
        p+=strlen("新名称写入ROM成功\r\n");
#endif
      }
      else
      {
#if LINGUA == EN
        sprintf(outputstr,"the new name written to ROM fault\r\n");
        p+=strlen("the new name written to ROM fault\r\n");
#endif
#if LINGUA == CH
        sprintf(outputstr,"新名称写入ROM失败\r\n");
        p+=strlen("新名称写入ROM失败\r\n");
#endif
      }  
    break;
    
  case CMD_RESTOREIP:
    message->m_type=CMD_NULL;
    setaddr->Mem_Used_Check=0xA55A;
    memcpy(setaddr->IPaddress,DevIPAddressTab,4);
    
#if STORE_METHOD == FLASH_METHOD 
    if( DataBase_Write(STORAGE_ROMADDR,(u32 *)setaddr,sizeof(Setting)))
#elif STORE_METHOD == BKP_METHOD
      if( DataBase_Write(STORAGE_ROMADDR,(u16 *)setaddr,sizeof(Setting)))
#endif
      {
        //infoNum = ShowInfo(outputstr, strSetIPSuc);
        //p += infoNum;
        sprintf(outputstr,"新IP写入ROM成功\r\n");
        p+=strlen("新IP写入ROM成功\r\n");
        
      }
      else
      {
        //infoNum = ShowInfo(outputstr, strSetIPFaul);
        //p += infoNum;
        sprintf(outputstr,"新IP写入ROM失败\r\n");
        p+=strlen("新IP写入ROM失败\r\n");     
      }
    break;    
    
  default:
    return 0;
  } 
  
  return (p-outputstr); 
}

uint32_t CmdQNA(char *outputstr,T_MESSAGE *message)
{
  char *p;
  p=outputstr;
  switch(message->m_type)
  {
  case CMD_DO:
  case CMD_COEF:
  case CMD_ADDR: 
  case CMD_CALB:
  case CMD_DCCAL:
  case CMD_SET: 
  case CMD_RESTOREIP:
  case CMD_RESET_ETH:
  case CMD_UPDATE_FIRMWARE:
  case CMD_ETH_LINK_TIME:
  case CMD_ETH_RECV_TIME:
#if LINGUA == EN 
    sprintf(outputstr,"Enter Password Please!\r\n");
    p+=strlen("Enter Password Please!\r\n");
#endif
    
#if LINGUA == CH
    sprintf(outputstr,"输入密码!\r\n");
    p+=strlen("输入密码!\r\n");
#endif
    
    Shell_State=PASSWORD_ASSERT;
    break; 
    
  case CMD_DATE: 
#if LINGUA == EN 
    sprintf(outputstr,"Enter date,Formatted as YYYY-MM-DD HH-MM-SS!\r\n");
    p+=strlen("Enter date,Formatted as YYYY-MM-DD HH-MM-SS!\r\n");
#endif
#if LINGUA == CH
    sprintf(outputstr,"输入日期 格式如YYYY-MM-DD HH-MM-SS!\r\n");
    p+=strlen("输入日期 格式如YYYY-MM-DD HH-MM-SS!\r\n");
#endif
    Shell_State=SET_DATE;
    break;  
    
  case CMD_PROT: 
    sprintf(outputstr,"输入通道序号 通讯测试原始值\r\n");
    p+=strlen("输入通道序号 通讯测试原始值\r\n");
    Shell_State=ENTER_DATA;
    break; 
    
  default:
    break;
  } 
  
  return (p-outputstr);	  
}

uint32_t CmdPwdAsserted(char *outputstr,T_MESSAGE *message)
{
  char *p;
  p=outputstr;
  uint16_t bak_dr = 0;  
  
  switch(message->m_type)
  {
  case CMD_DO:
    DoExecute((unsigned char)(message->m_intdata[0]));
    
#if LINGUA == EN  
    sprintf(outputstr,"DO Executed!Please Check the Di Change\r\n");
    p+=strlen("DO Executed!Please Check the Di Change\r\n");
#endif
#if LINGUA == CH  
    sprintf(outputstr,"遥控已操作，请查看遥信状态\r\n");
    p+=strlen("遥控已操作，请查看遥信状态\r\n");
#endif
    
    Shell_State=INIT;
    message->m_type=CMD_NULL;
    break;
    
  case CMD_SET:
  case CMD_COEF:
#if LINGUA == EN
    sprintf(outputstr,"Please Enter Item Resetting Parameter\r\n");     
    p+=strlen("Please Enter Item Resetting Parameter\r\n");
#endif 
#if LINGUA == CH
    sprintf(outputstr,"输入“通道序号 系数”\r\n");     
    p+=strlen("输入“通道序号 系数”\r\n");
#endif
    
    Shell_State=ENTER_DATA;
    break;
    
  case CMD_ADDR:
#if LINGUA == EN
    sprintf(outputstr,"Please Enter IP address\r\n");     
    p+=strlen("Please Enter IP address\r\n");
#endif
#if LINGUA == CH
    sprintf(outputstr,"输入IP地址\r\n");     
    p+=strlen("输入IP地址\r\n");
#endif
    
    Shell_State=ENTER_ADDR;
    break;  
    
  case CMD_CALB:
    Calibration();
    
#if LINGUA == EN  
    sprintf(outputstr,"Please Check the result of calibration\r\n");
    p+=strlen("Please Check the result of calibration\r\n");
#endif
#if LINGUA == CH  
    sprintf(outputstr,"校准完毕，核对遥测值\r\n");
    p+=strlen("校准完毕，核对遥测值\r\n");
#endif
    
    Shell_State=INIT;
    message->m_type=CMD_NULL;
    break;   
    
  case CMD_DCCAL:
    Can_Ticalib(CAN1);
    
#if LINGUA == EN  
    sprintf(outputstr,"Please Check the result of calibration\r\n");
    p+=strlen("Please Check the result of calibration\r\n");
#endif
#if LINGUA == CH  
    sprintf(outputstr,"标准源校准完毕，核对直流量\r\n");
    p+=strlen("标准源校准完毕，核对直流量\r\n");
#endif
    
    Shell_State=INIT;
    message->m_type=CMD_NULL;
    break;
    
  case CMD_PARA:
#if LINGUA == EN
    sprintf(outputstr,"Please Enter communiation settings\r\n");     
    p+=strlen("Please Enter communiation settings\r\n");
#endif
#if LINGUA == CH
    sprintf(outputstr,"输入通讯参数\r\n");     
    p+=strlen("输入通讯参数\r\n");
#endif
    
    Shell_State=ENTER_ADDR;
    break; 
    
    //TYH:添加恢復默认IP 
  case CMD_RESTOREIP:
    //infoNum = ShowInfo(outputstr, strRestoIP);   
    //p+=infoNum;
    sprintf(outputstr,"是否确认恢复默认IP, 确认请按<Y>, 按其它键返回\r\n");     
    p+=strlen("是否确认恢复默认IP, 确认请按<Y>, 按其它键返回\r\n");
    
    Shell_State=ENTER_RESTOREADDR;
    break;
    
    //TYH:20130508 恢复以太网 
  case CMD_RESET_ETH:
    //复位以太网
    Ethernet_HWRST();
    
    sprintf(outputstr,"是开始重置设备以太网,请等待.......\r\n");     
    p+=strlen("开始重置设备以太网,请等待.......\r\n");    
    
    Shell_State=INIT;
    message->m_type=CMD_NULL;
    break;
    
    //TYH:20130508 设置固件升级 
  case CMD_UPDATE_FIRMWARE:  
    //写备份寄存器 'KP_DR10', 用于启动升级固件
    bak_dr = BKP_ReadBackupRegister(BKP_DR10);
    if(bak_dr != 0x0707)
      BKP_WriteBackupRegister(BKP_DR10, 0x0707);    
    
    sprintf(outputstr,"bak_dr=0x%4x 设置固件升级, 请重启设备启动升级!\r\n", bak_dr);     
    p+=strlen("bak_dr=0xffff 设置固件升级, 请重启设备启动升级!\r\n");  
    
    Shell_State=INIT;
    message->m_type=CMD_NULL;    
    break;
    
    //TYH:20130530 设置以太网状态判断时间  
  case CMD_ETH_LINK_TIME:
  case CMD_ETH_RECV_TIME:
    sprintf(outputstr,"输入以太网判断时间参数\r\n");     
    p+=strlen("输入以太网判断时间参数\r\n");
    
    Shell_State=SET_ETH_TIME;
    break;    
    
  default:
    break;  
    
  }
  return (p-outputstr);	  
}

uint32_t CmdRestart(char *outputstr,T_MESSAGE *message)
{
  char *p;
  
  memset (outputstr,0,1314);
  p=outputstr;  
  
#if LINGUA ==EN
  sprintf(p,"Device will be restart, please wait.......\r\n");
  p = p+strlen("Device will be restart, please wait.......\r\n");
#elif LINGUA ==CH
  sprintf(p,"设备即将被复位重启,请等待.......\r\n");
  p = p+strlen("设备即将被复位重启,请等待.......\r\n");
#endif  
  
  NVIC_SystemReset();
  return 1;
}

void CmdnShellInit(void)
{
  Shell_State=INIT;
  memset (&Shell_Msg,0,sizeof(T_MESSAGE));
}

uint32_t ShellEtrRestoreIP(char *entry, char *b,uint8_t len)
{
  uint32_t infoNum;
  
  if (ShellCmdCmp(entry, YES, strlen(YES)))
  {
    return ShellEtrAddr(entry, b, 4);
  }
  
  
  sprintf(b, "取消恢复默认IP\r\n");
  
  infoNum =  strlen("取消恢复默认IP\r\n");
  
  return infoNum;    
}


uint32_t CmdShowIP(char *outputstr,T_MESSAGE *message)
{
  char *p;
  
  memset (outputstr,0,1314);
  p=outputstr;    
  
#if LINGUA ==EN
  sprintf(p,"Device IP now: %03d.%03d.%03d.%03d\r\n", 
          DevIPAddressTab[0], DevIPAddressTab[1], DevIPAddressTab[2], DevIPAddressTab[3]);
  p = p+strlen("Device IP now: 255.255.255.255\r\n");
#elif LINGUA ==CH
  sprintf(p,"当前装置IP为: %03d.%03d.%03d.%03d\r\n", 
          DevIPAddressTab[0], DevIPAddressTab[1], DevIPAddressTab[2], DevIPAddressTab[3]);
  p = p+strlen("当前装置IP为: 255.255.255.255\r\n");
#endif
  
  return (p-outputstr);	
}

uint32_t ShellSetEthTime(char *entry, char *b, uint8_t len)
{
  uint16_t time;
  uint8_t type;
  T_MESSAGE *message;
  Setting *setaddr;
  char *p;
  
  setaddr=&SetCurrent;
  message = &Shell_Msg;
  type = 0;
  
  p = entry;
  time=atoi(p);
  
  p = b;
  
  setaddr->Mem_Used_Check=0xA55A;
  if(message->m_type == CMD_ETH_LINK_TIME)
  {
    setaddr->eth_link_time = time;
    type = 1;
  }
  else if(message->m_type == CMD_ETH_RECV_TIME)
  {
    setaddr->eth_recv_time = time;
    type = 2;
  }
  else //使用默认设置
  {
    setaddr->eth_recv_time = ETH_RECV_TIME;
    setaddr->eth_link_time = ETH_LINK_TIME;
    type = 0xff;
  }
  
#if STORE_METHOD == FLASH_METHOD 
  if( DataBase_Write(STORAGE_ROMADDR,(u32 *)setaddr,sizeof(Setting)))
#elif STORE_METHOD == BKP_METHOD
    if( DataBase_Write(STORAGE_ROMADDR,(u16 *)setaddr,sizeof(Setting)))
#endif
    {      
      if(type == 1)
      {
        sprintf(b,"新以太网链接判断时间写入ROM成功\r\n");
        p+=strlen("新以太网链接判断时间写入ROM成功\r\n");
      }
      else if(type == 2)
      {
        sprintf(b,"新以太网数据收发判断时间写入ROM成功\r\n");
        p+=strlen("新以太网数据收发判断时间写入ROM成功\r\n");
      }
      else //使用默认设置
      {
        sprintf(b,"默认以太网时间参数写入ROM成功\r\n");
        p+=strlen("默认以太网时间参数写入ROM成功\r\n");
      }
    }
    else
    {
      sprintf(b,"新以太网时间参数写入ROM失败\r\n");
      p+=strlen("新以太网时间参数写入ROM失败\r\n");
    }
  
  message->m_type=CMD_NULL;
  
  return (p-b);
}