/**
******************************************************************************
* @file    helloworld.c 
* @author  MCD Application Team
* @version V1.0.0
* @date    11/20/2009
* @brief   A hello world example based on a Telnet connection
*          The application works as a server which wait for the client request
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
* <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
*/ 

/* Includes ------------------------------------------------------------------*/
#include "helloworld.h"
#include "lwip/tcp.h"
#include "shellcmd.h"
#include <string.h>

/* Private typedef ���������������������������������������--------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/


const char VENDORGREETING[] = {"\r\n"\
  "  _____ _______ __  __ ____ ___  ______ __  ___\r\n"\
    " / ____|__   __|  \\/  |___ \\__ \\|  ____/_ |/ _ \\\r\n"\
      "| (___    | |  | \\  / | __) | ) | |__   | | | | |_  __\r\n"\
        " \\___ \\   | |  | |\\/| ||__ < / /|  __|  | | | | \\ \\/ /\r\n"\
          " ____) |  | |  | |  | |___) / /_| |     | | |_| |>  <\r\n"\
            "|_____/   |_|  |_|  |_|____/____|_|     |_|\\___//_/\\_\\"\
              "Release Your Creative\r\n"\
                "\r\n"};																		

const char VENDEURGREETING[] = {
  "\033[0;33;40m\r\n"
    " ������������������������������ ������������������������������\r\n"\
      "��   __        __  _____      __      ____      _      _     ��\r\n"\
        "��  |  |  __  |  ||   _  \\  / __ \\  /  __  \\  / _ \\  / _ \\   �� \r\n"\
          "��  |  | /  \\ |  ||  | |  || |  | ||  |__|  || | | || | | |  ��\r\n"\
            "��  |  ||    ||  ||  |_|  || |__| | \\____   || | | || | | |  ��\r\n"\
              "��  |  ||    ||  ||   __ / |  __  |  ____|  || |_| || |_| |  �� \r\n"\
                "��  \\_____/\\_____/|__|     |_|  |_| \\______/  \\ _ /  \\ _ /   �� \r\n"\
                  "��                                                           ��\r\n"   \
                    " ������������������������������ ������������������������������\r\n"\
                      "	��������Է���Զ����Ŷ� SOMMES UNE EQUIPE��\033[0m\r\n"\
};
const char USERMANUAL[] = {
  "�d�T�T�T�T�T�T�T�T�T����̨ʹ��˵���T�T�T�T�T�T�T�T�T�T�g\r\n" \
    "�U��ѯң��ֵ����:measure                              �U\r\n" \
      "�U��ѯң��ֵ����:di                                   �U\r\n" \
        "�U��ɫΪ��λ����ɫΪ��λ����ɫΪ��λ                  �U\r\n" \
          "�Uң�ز�������:do+ң�ؼ̵�����ţ��������밴����ִ��  �U\r\n" \
            "�U��ѯ��������:date                                   �U\r\n" \
              "�U��ѯֱ������ֵ����:ti�������ֱ���ɼ�ģ��           �U\r\n" \
                "�U���õ�ַ����:setaddr���������밴����ִ��            �U\r\n"  \
                  "�U����ͨ��ϵ������:setcoef���������밴����ִ��        �U\r\n"  \
                    "�U��������ͨ��ϵ���ָ���1�������벽����������10���� �U\r\n"  \
                      "�U�����������setdate,����ʾ��ʽ����Ԥ������        �U\r\n"  \
                        "�U����װ���������setname,�س�ֱ������������        �U\r\n"  \
                          "�U�鿴װ��IP:ip                                       �U\r\n"  \
                            "�U(��֧�ִ���)�ָ�װ��Ĭ��ip(172.20.100.1):restoreip  �U\r\n"  \
                              "�U�鿴SOE��¼:relaysoe                                �U\r\n"  \
                                "�U�鿴ϵͳ���ߴ�����־:almrec                         �U\r\n"  \
                                  "�U����װ��:reset                                      �U\r\n"  \
                                    "�U(��֧�ִ���)������̫��:ethreset                     �U\r\n"  \
                                      "�U�����豸�����̼�:updatefirmware                     �U\r\n"  \
                                        "�U���ö����ж�ʱ��(��λ:����,Ĭ��Ϊ10):setlinktime    �U\r\n"  \
                                          "�U����ͨ���жϻָ�(��λ:����,Ĭ��Ϊ10):setrecvtime    �U\r\n"  \
                                            "�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a\r\n" \
}; 


#define CHARSUMIS0 "Command fault\r\n"
#define MAX_NAME_SIZE 32

extern uint32_t Shell_State;
extern uint8_t DevNameStr[16];
extern u32 DeviceSerial[3];	
extern NetState comstate[4];
extern const DisignItem DisignTab[16];
static struct tcp_pcb *Termpcb;		

extern char isCom;  //������ʾ�ж�,�Ƿ��Ǵ���

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct name 
{
  int length;
  char bytes[MAX_NAME_SIZE];
};

/* Private function prototypes -----------------------------------------------*/
static err_t HelloWorld_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t HelloWorld_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static void HelloWorld_conn_err(void *arg, err_t err);

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Called when a data is received on the telnet connection
* @param  arg	the user argument
* @param  pcb	the tcp_pcb that has received the data
* @param  p	the packet buffer
* @param  err	the error value linked with the received data
* @retval error value
*/
static err_t HelloWorld_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  struct pbuf *q;
  struct name *name = (struct name *)arg;
  int done;
  char *c;
  //char *Outputstr;
  char Outputstr[1314];
  int i,charsum;
  
  memset(Outputstr,0,1314);
  //Outputstr=(char*)calloc(1314,1);//���������Զ�̬�����
  
  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {        
    /* We call this function to tell the LwIp that we have processed the data */
    /* This lets the stack advertise a larger window, so more data can be received*/
    tcp_recved(pcb, p->tot_len);
    
    /* Check the name if NULL, no data passed, return withh illegal argument error */
    if(!name) 
    {
      pbuf_free(p);
      return ERR_ARG;
    }
    
    done = 0;
    for(q=p; q != NULL; q = q->next) 
    {
      c = q->payload;
      for(i=0; i<q->len && !done; i++) 
      {
        done = ((c[i] == '\r') || (c[i] == '\n'));
        if(name->length < MAX_NAME_SIZE) 
        { 
          if(c[i]!='\b')
            name->bytes[name->length++] = c[i];
          else
            name->length--;
        }  
      }
    }
    
    if(done) 
    {
      
      comstate[TERM].state=1;
      if(name->bytes[name->length-2] != '\r' || name->bytes[name->length-1] != '\n') 
      {
        if((name->bytes[name->length-1] == '\r' || name->bytes[name->length-1] == '\n') && (name->length+1 <= MAX_NAME_SIZE)) 
        {
          name->length += 1;
        } 
        else if(name->length+2 <= MAX_NAME_SIZE) 
        {
          name->length += 2;
        } 
        else 
        {
          name->length = MAX_NAME_SIZE;
        }
        
        name->bytes[name->length-2] = '\r';
        name->bytes[name->length-1] = '\n';
      }
      //   tcp_write(pcb, HELLO, strlen(HELLO), 1);
      
      //tcp_write(pcb, name->bytes, name->length, TCP_WRITE_FLAG_COPY);
      if(NULL != Outputstr)
      {
        isCom = 0;
        switch(Shell_State)
        {
        case INIT:
          charsum=ShellCmdMatch(name->bytes,Outputstr,name->length-2); 
          break;
          
        case PASSWORD_ASSERT:
          charsum=ShellPwdMatch(name->bytes,Outputstr,name->length-2); 
          break;
          
        case ENTER_DATA:
          charsum=ShellEtrData(name->bytes,Outputstr,4);
          CmdnShellInit();
          break; 
          
        case ENTER_ADDR:
          charsum=ShellEtrAddr(name->bytes,Outputstr,4); 
          CmdnShellInit();
          break;
          
        case SET_DATE:
          charsum=ShellEtrDate(name->bytes,Outputstr,name->length);
          CmdnShellInit(); 
          break;
          
        case TI_mA:
          charsum=ShellEtrData(name->bytes,Outputstr,1);
          CmdnShellInit();
          break;
          
        case MODIFY_NAME:
          charsum=ShellEtrStr(name->bytes,Outputstr,name->length);
          CmdnShellInit();  
          break;
          
        case SET_PARA:
          charsum=ShellEtrPara(name->bytes,Outputstr,name->length);
          CmdnShellInit(); 
          break;          
          
        case ENTER_RESTOREADDR:
          if(!isCom)
          {
            charsum=ShellEtrRestoreIP(name->bytes, Outputstr, 4); 
            CmdnShellInit();  
          }
          else
            charsum = 0;
          break;
          
        case SET_ETH_TIME:
          charsum=ShellSetEthTime(name->bytes,Outputstr,name->length);
          CmdnShellInit(); 
          break;
//
//        case UPDATE_FIRMWARE:
//          charsum=ShellUpdateFirmware(name->bytes,Outputstr,name->length);
//          CmdnShellInit(); 
//          break;          
          
        default:
          break;
        }
	//  if(0==strncmp(name->bytes,"measure",strlen("measure"))) 
	//  {
	//  charsum=CmdShowMea(Outputstr);
        if(charsum)
	  tcp_write(pcb, Outputstr, charsum, TCP_WRITE_FLAG_COPY);
        //  }
        else
	  tcp_write(pcb, CHARSUMIS0, strlen(CHARSUMIS0), 1);
        
        name->length = 0;
        memset(Outputstr,0,1314);
      }
    }
    
    /* End of processing, we free the pbuf */
    //free(Outputstr);
    pbuf_free(p);
  }  
  else if (err == ERR_OK) 
  {
    /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
    /* We free the allocated memory and we close the connection */
    mem_free(name);
    //free(Outputstr);
    comstate[TERM].state=0;
    
    return tcp_close(pcb);
  }
  
  return ERR_OK;
}

/**
* @brief  This function when the Telnet connection is established
* @param  arg  user supplied argument 
* @param  pcb	 the tcp_pcb which accepted the connection
* @param  err	 error value
* @retval ERR_OK
*/
static err_t HelloWorld_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{  
  char IDString[192]; 
  /* Tell LwIP to associate this structure with this connection. */
  tcp_arg(pcb, mem_calloc(sizeof(struct name), 1));
  
  /* Configure LwIP to use our call back functions. */
  tcp_err(pcb, HelloWorld_conn_err);
  tcp_recv(pcb, HelloWorld_recv);
  
  /* Send out the first message */  
  tcp_write(pcb, VENDEURGREETING, strlen(VENDEURGREETING), 1); 
  memset(IDString,0,192);
  sprintf(IDString,"�X�T�T�T�T�T�T�T�T�T�T�TPOSTINFO�T�T�T�T�T�T�T�T�T�T�T�[\n\r�UChip Electronic Signature:%X-%X-%X �U\n\r�UFlashSize= %3dK byte   Device Name:%-16s �U\n\r", \
    DeviceSerial[0], DeviceSerial[1], DeviceSerial[2],*(__IO uint16_t*)(0x1FFFF7E0),DevNameStr); 
  
  tcp_write(pcb,IDString,174,1);
  tcp_write(pcb, USERMANUAL, strlen(USERMANUAL), 1); 
  CmdnShellInit();
  comstate[TERM].state=1;
  Termpcb=pcb;
  
  return ERR_OK;
}

/**
* @brief  Initialize the hello application  
* @param  None 
* @retval None 
*/
struct tcp_pcb * HelloWorld_init(void)
{
  struct tcp_pcb *pcb;	            		
  
  /* Create a new TCP control block  */
  pcb = tcp_new();	                		 	
  
  /* Assign to the new pcb a local IP address and a port number */
  /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
  tcp_bind(pcb, IP_ADDR_ANY, 23);       
  
  
  /* Set the connection to the LISTEN state */
  pcb = tcp_listen(pcb);				
  
  /* Specify the function to be called when a connection is established */	
  tcp_accept(pcb, HelloWorld_accept);   
  
  return (struct tcp_pcb *)pcb;										
}

/**
* @brief  This function is called when an error occurs on the connection 
* @param  arg
* @parm   err
* @retval None
*/
static void HelloWorld_conn_err(void *arg, err_t err)
{
  struct name *name;
  name = (struct name *)arg;
  
  mem_free(name);
}


/*******************����Ϊ�����ն˲��� *********/
void Uart_Write(const char *ch,char cnt)
{
  char i;
  uint32_t Timeout=0x3fff;
  
  for(i=0;i++;i<cnt)
  {
    USART_SendData(USART1, ch[i]); 
    
    /* Loop until the end of transmission */ 
    
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) 
    { 
      if(!Timeout--) break;
    } 
    Timeout=0x3fff;
  }
}


void UartProcess(char *RxBuffer, char RxCounter)
{
  char Outputstr[1314];
  uint16_t charsum = 0;
  
  if((RxBuffer[0] == 0x24/*$*/)&&(RxCounter == 1))
  {
    //Uart_Write( USERMANUAL,strlen(USERMANUAL)); 
    printf(VENDEURGREETING);
    printf(USERMANUAL);
    CmdnShellInit();
  }
  else
  {
    isCom = 1;
    switch(Shell_State)
    {
    case INIT:
      charsum=ShellCmdMatch(RxBuffer,Outputstr,RxCounter-2); 
      break;
      
    case PASSWORD_ASSERT:
      charsum=ShellPwdMatch(RxBuffer,Outputstr,RxCounter-2); 
      break;
      
    case ENTER_DATA:
      charsum=ShellEtrData(RxBuffer,Outputstr,4); 
      CmdnShellInit();
      break;
      
    case ENTER_ADDR:
      charsum=ShellEtrAddr(RxBuffer,Outputstr,4); 
      CmdnShellInit();
      break;
      
    case SET_DATE:
      charsum=ShellEtrDate(RxBuffer,Outputstr,RxCounter);
      CmdnShellInit(); 
      break;
      
    case TI_mA:
      charsum=ShellEtrData(RxBuffer,Outputstr,1);
      CmdnShellInit();
      break;
      
    case MODIFY_NAME:
      charsum=ShellEtrStr(RxBuffer,Outputstr,RxCounter);
      CmdnShellInit();  
      break;
      
    case SET_PARA:
      charsum=ShellEtrPara(RxBuffer,Outputstr,RxCounter);
      CmdnShellInit();
      
    case ENTER_RESTOREADDR:
      charsum=ShellEtrRestoreIP(RxBuffer, Outputstr, 4); 
      CmdnShellInit();
      
    case SET_ETH_TIME:
      charsum=ShellSetEthTime(RxBuffer,Outputstr,RxCounter);
      CmdnShellInit(); 
      break;
      
      //
      //    case UPDATE_FIRMWARE:
      //      charsum=ShellUpdateFirmware(RxBuffer, Outputstr, RxCounter); 
      //      CmdnShellInit();       
      
    default:
      break;
    }
    
    if(charsum)
      //Uart_Write(Outputstr, charsum);
      printf("%s", Outputstr);
    else
      //Uart_Write(CHARSUMIS0, strlen(CHARSUMIS0));
      printf(CHARSUMIS0);
    
    memset(Outputstr,0,512);
  }
}

/**********************************��ӡ��telnet*************************************/
void telprintf( const char *fmt, ...)
{
  va_list args;
  char p[256];    
  va_start(args, fmt);
  vsprintf(p, fmt, args);
  va_end(args);
  
  
  if(1==comstate[TERM].state)
    
    tcp_write(Termpcb,p, strlen(p), 1);
}

/*********************************����SOE��Ϣ��ӡ***************************************/
void Term_soe_trig(SOE_Struct *soe)
{
  struct tm t_tm	;
  
  t_tm=Time_ConvUnixToCalendar(soe->timetamp);
  telprintf("%4d��%2d��%2d��%2dʱ%2d��%2d��%4d���� %8s %4s\n\r", t_tm.tm_year,t_tm.tm_mon/*+1*/,t_tm.tm_mday,\
    t_tm.tm_hour,t_tm.tm_min,t_tm.tm_sec, soe->mscd,DisignTab[soe->diseq].Title,(soe->state?"�պ�":"��")); 
  
}
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


