/*
�����ǵ��Ų��ں��˵��ǻۣ�ֻ�ں���Ĵ���
���������������ֶ�ǰ
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
/*��ɫ���ƶ���*/
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
  {"��Ua","��",360},
  {"Ub","V",120},
  {"��Ub","��",360},
  {"Uc","V",120},
  {"��Uc","��",360},
  {"Ia","A",6},
  {"��Ia","��",360},
  {"Ib","A",6},
  {"��Ib","��",360},
  {"Ic","A",6},
  {"��Ic","��",360},
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
  {"Thm","��C",100},
  {"Th1","mV",6000},
  {"Th2","mV",6000},
  {"Th3","mV",6000},
  {"Th4","mV",6000},
};
const DisignItem DisignTab[16]=
{
  //{"ң�ſ���һ",0},
  //{"ң�ſ����",0},
  //{"ң�ſ�����",0},
  //{"ң�ſ�����",0},
  //{"ң�ſ�����",0},
  //{"ң�ſ�����",0},
  //{"ң�ſ�����",0},
  //{"ң�ſ����",0},
  //{"ң�ſ����",0},
  //{"ң�ſ���ʮ",0},
  //{"ң�ſ���ʮһ",0},
  //{"ң�ſ���ʮ��",0},
  //{"ң�ſ���ʮ��",0},
  //{"ң�ſ���ʮ��",0},
  //{"ң�ſ���ʮ��",0},
  //{"ң�ſ���ʮ��",0},
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
char isCom = 1;   //tyh:������ʾ����


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
        //tyh:20130312 ������������ڴ���  
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
    sprintf(b, "�������\r\n");
    p+=strlen("�������\r\n");
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
  
  //20130312 TYH:���IP�����ж�
  if(Shell_Msg.m_type == CMD_RESTOREIP) //�ָ�Ĭ��IP
  {
    for(i=0; i<4; i++)
    {
      DevIPAddressTab[i] = DefaultIP[i];
    }
  }
  else //������IP
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
        sprintf(b, "��ַ����\r\n");
        p+=strlen("��ַ����\r\n");
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
  
  memset(&t, 0, sizeof(t));//tyh:20130308 ���ӳ�ʼ��Ϊ'0', ����ʱ��������
  
  t.tm_year =atoi(p);
  p=strpbrk(p,"-")+1;
  t.tm_mon =atoi(p)/*-1*/;//tyh:20130308 ȥ��"-1", ���� Time_SetCalendarTime ͳһ����
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
    sprintf(b, "ʱ�䷶Χ����\r\n");
    p=strlen("ʱ�䷶Χ����\r\n")+b;
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
      sprintf(b,"�²���д��ROM�ɹ�\r\n");
      p=strlen("�²���д��ROM�ɹ�\r\n")+b;
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
      sprintf(p, "�X�T�T�T�T�T�T�T�T�j�T�T�T�T�Ttotal meas:%3d�T�T�T�T�T�j�T�T�T�T�T�T�T�T�[\r\n",MEANUM);
      p+=strlen("�X�T�T�T�T�T�T�T�T�j�T�T�T�T�Ttotal meas:%3d�T�T�T�T�T�j�T�T�T�T�T�T�T�T�[\r\n");
#endif
#if LINGUA==CH
      sprintf(p, "�X�T�T�T�T�T�T�T�T�j�T�T�T�T�Tң��������:%3d�T�T�T�T�T�j�T�T�T�T�T�T�T�T�[\r\n",MEANUM);
      p+=strlen("�X�T�T�T�T�T�T�T�T�j�T�T�T�T�Tң��������:%3d�T�T�T�T�T�j�T�T�T�T�T�T�T�T�[\r\n");
#endif
      for(i=0;i<MEANUM;i++)
      {
        if ((i+1) % 4)
        {
          if((16==i)||(17==i)||(21==i)||(22==i))
          {
            //											if((I2C_MeasureTab[i]-MeaRecordTab[i])>Compress_Factor*MeaPropTab[i].scale)
            //											   {
            //											    sprintf(p, "�U%-4s:%s%6.1f%s %4s",MeaPropTab[i].Title,rouge_mutation,\
              //												I2C_MeasureTab[i],normale,MeaPropTab[i].Unit);
              //											   p+=strlen(rouge_mutation);
              //                  							   p+=strlen(normale);
              //											   }
              //											else 
              sprintf(p, "�U%-4s:%6.1f %-4s",MeaPropTab[i].Title,I2C_MeasureTab[i],MeaPropTab[i].Unit);
              p+=strlen("�UXXXX:01234.5XXXX");
          }
          else if(18==i)
          {
            //											if((I2C_MeasureTab[i]-MeaRecordTab[i])>Compress_Factor*MeaPropTab[i].scale)
            //											    {
            //												sprintf(p, "�U%-4s:%s%6.1f %s%4s",MeaPropTab[i].Title,rouge_mutation,\
              //												I2C_MeasureTab[i],normale,MeaPropTab[i].Unit);
              //												p+=strlen(rouge_mutation);
              //                  							    p+=strlen(normale);
              //											    }
              //											else 
              sprintf(p, "�U%-4s:%6.4f %-4s",MeaPropTab[i].Title,I2C_MeasureTab[i],MeaPropTab[i].Unit);
              p+=strlen("�UXXXX:01.2345XXXX");
          }
          else
          {
            //											if((I2C_MeasureTab[i]-MeaRecordTab[i])>Compress_Factor*MeaPropTab[i].scale)
            //											    {
            //												sprintf(p, "�U%-4s:%s%6.1f%s %4s",MeaPropTab[i].Title,rouge_mutation,\
              //												I2C_MeasureTab[i],normale,MeaPropTab[i].Unit);
              //												p+=strlen(rouge_mutation);
              //                  							    p+=strlen(normale);
              //											   }
              //											else 
              sprintf(p, "�U%-4s:%6.2f %-4s",MeaPropTab[i].Title,I2C_MeasureTab[i],MeaPropTab[i].Unit);
              p+=strlen("�UXXXX:0123.45XXXX");
          }
        }
        else
        {	
          //									if((I2C_MeasureTab[i]-MeaRecordTab[i])>Compress_Factor*MeaPropTab[i].scale)
          //											    {
          //												sprintf(p, "�U%-4s:%s%6.1f %s%4s",MeaPropTab[i].Title,rouge_mutation,\
            //												I2C_MeasureTab[i],normale,MeaPropTab[i].Unit);
            //												p+=strlen(rouge_mutation);
            //                  							    p+=strlen(normale);
            //											    }
            //											else 
            sprintf(p, "�U%-4s:%6.2f %-4s�U\r\n",MeaPropTab[i].Title,I2C_MeasureTab[i],MeaPropTab[i].Unit);
            p+=strlen("�UXXXX:0123.45XXXX�U\r\n");
            //sprintf(p, "�d�T�T�T�T�T�T�T�T�p�T�T�T�T�T�T�T�T�p�T�T�T�T�T�T�T�T�p�T�T�T�T�T�T�T�T�g\r\n");
            if((MEANUM-1)==i)  sprintf(p, "�^�T�T�T�T�T�T�T�T�m�T�T�T�T�T�T�T�T�m�T�T�T�T�T�T�T�T�m�T�T�T�T�T�T�T�T�a\r\n");
            p+=strlen("�d�T�T�T�T�T�T�T�T�p�T�T�T�T�T�T�T�T�p�T�T�T�T�T�T�T�T�p�T�T�T�T�T�T�T�T�g\r\n");
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
      sprintf(outputstr,"�޷���ȡAI���� \r\n");
      p+=strlen("�޷���ȡAI���� \r\n");	
#endif
    }
#endif
    break;
    
  case CMD_DI:
#if LINGUA== EN	
    sprintf(p, "�X�T�Ttotaldis%2d�T�T�[\r\n",16);
    p+=strlen("�X�T�Ttotaldis%2d�T�T�[\r\n");
#endif
#if LINGUA ==CH	
    //sprintf(p, "�X�T�Tң������%2d�T�T�[\r\n",16);
    //p+=strlen("�X�T�Tң������%2d�T�T�[\r\n");
    
    sprintf(p, "�X�T�Tң������%2d�T�T�[",16);
    p+=strlen("�X�T�Tң������%2d�T�T�[");
    
#endif    
    for(i=0;i<8;i++)
    {  
      if(1==DiStatus_DI[i].Value)
      {
        
        sprintf(p, "�U %-3d %s��%s ",i+1,rouge,normale);
        if(DiRecording[i]!=DiStatus_DI[i].Value)  sprintf(p, "�U %-3d %s��%s ",i+1,rouge_mutation,normale);
        p+=strlen("�U %-3d �� ");
        p+=strlen(rouge);
        p+=strlen(normale);
        
      }  
      else 
      {
        
        sprintf(p, "�U %-3d %s��%s ",i+1,vert,normale); 
        if(DiRecording[i]!=DiStatus_DI[i].Value)  sprintf(p, "�U %-3d %s��%s ",i+1,vert_mutation,normale);
        p+=strlen("�U %-3d �� ");
        p+=strlen(vert);
        p+=strlen(normale);
        
      } 
      DiRecording[i]=DiStatus_DI[i].Value;                  
      
      if(1==DiStatus_DI[i+8].Value)
      {
        
        sprintf(p, "�U %-3d %s��%s �U\r\n",i+9,rouge,normale);
        if(DiRecording[i+8]!=DiStatus_DI[i+8].Value)  sprintf(p, "�U %-3d %s��%s �U\r\n",i+9,rouge_mutation,normale);
        p+=strlen("%�U -3d �� �U\r\n");
        p+=strlen(rouge);
        p+=strlen(normale);
        
      }  
      else 
      {
        
        sprintf(p, "�U %-3d %s��%s �U\r\n",i+9,vert,normale); 
        if(DiRecording[i+8]!=DiStatus_DI[i+8].Value)  sprintf(p, "�U %-3d %s��%s �U\r\n",i+9,vert_mutation,normale);
        p+=strlen("�U %-3d �� �U\r\n");
        p+=strlen(vert);
        p+=strlen(normale);
        
      }
      if(7==i)
      {
        sprintf(p, "�^�T�T�T�T�m�T�T�T�T�a\r\n");
        p+=strlen("�^�T�T�T�T�m�T�T�T�T�a\r\n");
      }
      else
      {
        sprintf(p, "�d�T�T�T�T�p�T�T�T�T�g\r\n");
        p+=strlen("�^�T�T�T�T�m�T�T�T�T�a\r\n"); 
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
      sprintf(p,"����4��20mAͨ����Ų鿴������ʾ\r\n");
      p=p+strlen("����4��20mAͨ����Ų鿴������ʾ\r\n")+strlen(DevNameStr);
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
      sprintf(outputstr,"�޷���ȡTI����\r\n");
      p+=strlen("�޷���ȡTI����\r\n");	
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
    sprintf(p,"��ǰװ������Ϊ%s,������������\r\n",DevNameStr);
    p=p+strlen("��ǰװ������Ϊ,������������\r\n")+strlen(DevNameStr);
#endif
    Shell_State=MODIFY_NAME;
    break;
    
  case CMD_NET:
    i=netstat(p);
    p+=i;
    break;
    
  case CMD_SOE:
    sprintf(p,"***************SOE�¼���¼***************\r\n");
    p+=strlen("***************SOE�¼���¼***************\r\n");
    
    if(0==SetCurrent.TrigSoeQueue.total)
    {
      sprintf(p,"��SOE�¼���¼ *_*||\r\n");
      p+=strlen("��SOE�¼���¼ *_*||\r\n");
    }
    else
    {
      for(i=0;i<SetCurrent.TrigSoeQueue.total;i++)
      {
        //tyh:20130312 �жϵ�ǰ��ʾ���Ƿ������¼�¼,������ǻ�������
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
        sprintf(p, "%4d-%02d-%02d %02d:%02d:%02d %4dms DI%02d�ǵ���%4s\r\n",time_now.tm_year,time_now.tm_mon,time_now.tm_mday,\
          time_now.tm_hour,time_now.tm_min,time_now.tm_sec,SetCurrent.TrigSoeQueue.SOE_Queue_Flash[i].mscd,\
            SetCurrent.TrigSoeQueue.SOE_Queue_Flash[i].diseq+1,(SetCurrent.TrigSoeQueue.SOE_Queue_Flash[i].state?"����":"����"));
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
        //tyh:20130312 �жϵ�ǰ��ʾ���Ƿ������¼�¼,������ǻ�������
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
    
  case CMD_SHOWIP://20130312 TYH:���IP��ʾ�����
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
        sprintf(outputstr,"����ͨ��ϵ����Ϊ1000\r\n");
        p+=strlen("����ͨ��ϵ����Ϊ1000\r\n");
      }
      else
      {
        sprintf(outputstr,"ͨ��[%s%s%s]ϵ���趨�ɹ�\r\n",vert,CoefTab[message->m_intdata[0]-1].Title,normale);
        p+=strlen("ͨ��[XXX]ϵ���趨�ɹ�\r\n");
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
      sprintf(outputstr,"�趨����ʧ��\r\n");
      p+=strlen("�趨����ʧ��\r\n");
#endif
    }
    break;
    
  case CMD_PROT:
    message->m_type=CMD_NULL;
    AiCoefSet(message->m_intdata[0],message->m_intdata[1]);
    
    if(100==message->m_intdata[0])
    {
      sprintf(outputstr,"����ͨ���ϴ�ԭʼֵ��Ϊ2048\r\n");
      p+=strlen("����ͨ���ϴ�ԭʼֵ��Ϊ2048\r\n");
    }
    else
    {
      sprintf(outputstr,"ͨ��[%s%s%s]ģ�����ͣ���˲���ң��ֵ\r\n",vert,CoefTab[message->m_intdata[0]-1].Title,normale);
      p+=strlen("ͨ��[XXX]ģ�����ͣ���˲���ң��ֵ\r\n");
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
        sprintf(outputstr,"��IPд��ROM�ɹ�\r\n");
        p+=strlen("��IPд��ROM�ɹ�\r\n");
#endif
      }
      else
      {
#if LINGUA == EN
        sprintf(outputstr,"the new IP written to ROM fault\r\n");
        p+=strlen("the new IP written to ROM fault\r\n");
#endif
#if LINGUA == CH
        sprintf(outputstr,"��IPд��ROMʧ��\r\n");
        p+=strlen("��IPд��ROMʧ��\r\n");
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
        sprintf(outputstr,"������д��ROM�ɹ�\r\n");
        p+=strlen("������д��ROM�ɹ�\r\n");
#endif
      }
      else
      {
#if LINGUA == EN
        sprintf(outputstr,"the new name written to ROM fault\r\n");
        p+=strlen("the new name written to ROM fault\r\n");
#endif
#if LINGUA == CH
        sprintf(outputstr,"������д��ROMʧ��\r\n");
        p+=strlen("������д��ROMʧ��\r\n");
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
        sprintf(outputstr,"��IPд��ROM�ɹ�\r\n");
        p+=strlen("��IPд��ROM�ɹ�\r\n");
        
      }
      else
      {
        //infoNum = ShowInfo(outputstr, strSetIPFaul);
        //p += infoNum;
        sprintf(outputstr,"��IPд��ROMʧ��\r\n");
        p+=strlen("��IPд��ROMʧ��\r\n");     
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
    sprintf(outputstr,"��������!\r\n");
    p+=strlen("��������!\r\n");
#endif
    
    Shell_State=PASSWORD_ASSERT;
    break; 
    
  case CMD_DATE: 
#if LINGUA == EN 
    sprintf(outputstr,"Enter date,Formatted as YYYY-MM-DD HH-MM-SS!\r\n");
    p+=strlen("Enter date,Formatted as YYYY-MM-DD HH-MM-SS!\r\n");
#endif
#if LINGUA == CH
    sprintf(outputstr,"�������� ��ʽ��YYYY-MM-DD HH-MM-SS!\r\n");
    p+=strlen("�������� ��ʽ��YYYY-MM-DD HH-MM-SS!\r\n");
#endif
    Shell_State=SET_DATE;
    break;  
    
  case CMD_PROT: 
    sprintf(outputstr,"����ͨ����� ͨѶ����ԭʼֵ\r\n");
    p+=strlen("����ͨ����� ͨѶ����ԭʼֵ\r\n");
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
    sprintf(outputstr,"ң���Ѳ�������鿴ң��״̬\r\n");
    p+=strlen("ң���Ѳ�������鿴ң��״̬\r\n");
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
    sprintf(outputstr,"���롰ͨ����� ϵ����\r\n");     
    p+=strlen("���롰ͨ����� ϵ����\r\n");
#endif
    
    Shell_State=ENTER_DATA;
    break;
    
  case CMD_ADDR:
#if LINGUA == EN
    sprintf(outputstr,"Please Enter IP address\r\n");     
    p+=strlen("Please Enter IP address\r\n");
#endif
#if LINGUA == CH
    sprintf(outputstr,"����IP��ַ\r\n");     
    p+=strlen("����IP��ַ\r\n");
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
    sprintf(outputstr,"У׼��ϣ��˶�ң��ֵ\r\n");
    p+=strlen("У׼��ϣ��˶�ң��ֵ\r\n");
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
    sprintf(outputstr,"��׼ԴУ׼��ϣ��˶�ֱ����\r\n");
    p+=strlen("��׼ԴУ׼��ϣ��˶�ֱ����\r\n");
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
    sprintf(outputstr,"����ͨѶ����\r\n");     
    p+=strlen("����ͨѶ����\r\n");
#endif
    
    Shell_State=ENTER_ADDR;
    break; 
    
    //TYH:��ӻ֏�Ĭ��IP 
  case CMD_RESTOREIP:
    //infoNum = ShowInfo(outputstr, strRestoIP);   
    //p+=infoNum;
    sprintf(outputstr,"�Ƿ�ȷ�ϻָ�Ĭ��IP, ȷ���밴<Y>, ������������\r\n");     
    p+=strlen("�Ƿ�ȷ�ϻָ�Ĭ��IP, ȷ���밴<Y>, ������������\r\n");
    
    Shell_State=ENTER_RESTOREADDR;
    break;
    
    //TYH:20130508 �ָ���̫�� 
  case CMD_RESET_ETH:
    //��λ��̫��
    Ethernet_HWRST();
    
    sprintf(outputstr,"�ǿ�ʼ�����豸��̫��,��ȴ�.......\r\n");     
    p+=strlen("��ʼ�����豸��̫��,��ȴ�.......\r\n");    
    
    Shell_State=INIT;
    message->m_type=CMD_NULL;
    break;
    
    //TYH:20130508 ���ù̼����� 
  case CMD_UPDATE_FIRMWARE:  
    //д���ݼĴ��� 'KP_DR10', �������������̼�
    bak_dr = BKP_ReadBackupRegister(BKP_DR10);
    if(bak_dr != 0x0707)
      BKP_WriteBackupRegister(BKP_DR10, 0x0707);    
    
    sprintf(outputstr,"bak_dr=0x%4x ���ù̼�����, �������豸��������!\r\n", bak_dr);     
    p+=strlen("bak_dr=0xffff ���ù̼�����, �������豸��������!\r\n");  
    
    Shell_State=INIT;
    message->m_type=CMD_NULL;    
    break;
    
    //TYH:20130530 ������̫��״̬�ж�ʱ��  
  case CMD_ETH_LINK_TIME:
  case CMD_ETH_RECV_TIME:
    sprintf(outputstr,"������̫���ж�ʱ�����\r\n");     
    p+=strlen("������̫���ж�ʱ�����\r\n");
    
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
  sprintf(p,"�豸��������λ����,��ȴ�.......\r\n");
  p = p+strlen("�豸��������λ����,��ȴ�.......\r\n");
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
  
  
  sprintf(b, "ȡ���ָ�Ĭ��IP\r\n");
  
  infoNum =  strlen("ȡ���ָ�Ĭ��IP\r\n");
  
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
  sprintf(p,"��ǰװ��IPΪ: %03d.%03d.%03d.%03d\r\n", 
          DevIPAddressTab[0], DevIPAddressTab[1], DevIPAddressTab[2], DevIPAddressTab[3]);
  p = p+strlen("��ǰװ��IPΪ: 255.255.255.255\r\n");
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
  else //ʹ��Ĭ������
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
        sprintf(b,"����̫�������ж�ʱ��д��ROM�ɹ�\r\n");
        p+=strlen("����̫�������ж�ʱ��д��ROM�ɹ�\r\n");
      }
      else if(type == 2)
      {
        sprintf(b,"����̫�������շ��ж�ʱ��д��ROM�ɹ�\r\n");
        p+=strlen("����̫�������շ��ж�ʱ��д��ROM�ɹ�\r\n");
      }
      else //ʹ��Ĭ������
      {
        sprintf(b,"Ĭ����̫��ʱ�����д��ROM�ɹ�\r\n");
        p+=strlen("Ĭ����̫��ʱ�����д��ROM�ɹ�\r\n");
      }
    }
    else
    {
      sprintf(b,"����̫��ʱ�����д��ROMʧ��\r\n");
      p+=strlen("����̫��ʱ�����д��ROMʧ��\r\n");
    }
  
  message->m_type=CMD_NULL;
  
  return (p-b);
}