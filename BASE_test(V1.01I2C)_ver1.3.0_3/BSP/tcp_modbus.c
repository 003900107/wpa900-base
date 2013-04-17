/**
******************************************************************************

GOD is our refuge and strength,a very present help in trouble.


/* Includes ------------------------------------------------------------------*/
#include "tcp_modbus.h"
#include "shellcmd.h"
#include "modbus.h"
#include "pad_client.h" 
#include "bsp.h"
#include "Proj_Config.h"
#include <string.h>


extern  NetState comstate[4];

/* Private define ------------------------------------------------------------*/
#define MAX_BUFF_SIZE 16

/* Private typedef -----------------------------------------------------------*/
struct RxBuffer 
{
  int length;
  char bytes[MAX_BUFF_SIZE];
};

struct UDP_RBUF 
{
  int length;
  u8_t bytes[41];
};

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint32_t Shell_State;
static struct tcp_pcb *MODBUS_TcpPCB;
/* Private function prototypes -----------------------------------------------*/
static err_t TcpModbus_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t TcpModbus_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static void TcpModbus_conn_err(void *arg, err_t err);

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Called when a data is received on the telnet connection
* @param  arg	the user argument
* @param  pcb	the tcp_pcb that has received the data
* @param  p	the packet buffer
* @param  err	the error value linked with the received data
* @retval error value
*/
static err_t TcpModbus_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  struct pbuf *q;
  struct RxBuffer *RxBuffer = (struct RxBuffer *)arg;
  uint8_t i,done;
  char *c,Tx_Buf[256];
  uint32_t charsum;
  
  
  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {        
    /* We call this function to tell the LwIp that we have processed the data */
    /* This lets the stack advertise a larger window, so more data can be received*/
    tcp_recved(pcb, p->tot_len);
    
    /* Check the name if NULL, no data passed, return withh illegal argument error */
    if(!RxBuffer) 
    {
      pbuf_free(p);
      return ERR_ARG;
    }
    done = 0;
    for(q=p; q != NULL; q = q->next) 
    {
      c = q->payload;
      for(i=0; i<q->len && !done ; i++) 
      {
        if(RxBuffer->length < MAX_BUFF_SIZE) 
        {
          RxBuffer->bytes[RxBuffer->length++] = c[i];
          done=((c[0]==0x00)&&(c[1]==0x00)&&(c[2]==0x00)&&(c[3]==0x00)&&(c[4]==0x00)&&(c[5]==0x06)&&(TCPMODBUS_ADDR==c[6])&&(i==11));
        }
      }
    }
    if(done) 
    {
      comstate[NETMBUS].state=2;
      charsum=process_rx_data(&(RxBuffer->bytes[6]),Tx_Buf,RxBuffer->length);
      if(charsum) 
      {
        tcp_write(pcb, Tx_Buf, charsum, TCP_WRITE_FLAG_COPY);
      }
      memset(Tx_Buf,0,charsum);
      RxBuffer->length = 0;
      
    }
    
    /* End of processing, we free the pbuf */
    pbuf_free(p);
  }  
  else if (err == ERR_OK) 
  {
    /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
    /* We free the allocated memory and we close the connection */
    mem_free(RxBuffer);
    comstate[NETMBUS].state=0;
    
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
static err_t TcpModbus_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{     
  
  /* Tell LwIP to associate this structure with this connection. */
  tcp_arg(pcb, mem_calloc(sizeof(struct RxBuffer), 1));	
  
  /* Configure LwIP to use our call back functions. */
  tcp_err(pcb, TcpModbus_conn_err);
  tcp_recv(pcb, TcpModbus_recv);
  
  return ERR_OK;
}

/**
* @brief  Initialize the hello application  
* @param  None 
* @retval None 
*/
struct tcp_pcb * TcpModbus_init(void)
{
  struct tcp_pcb *pcb;	            		

  /* Create a new TCP control block  */
  pcb = tcp_new();	                		 	
  
  /* Assign to the new pcb a local IP address and a port number */
  /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
  tcp_bind(pcb, IP_ADDR_ANY, 5185);       
  
  /* Set the connection to the LISTEN state */
  pcb = tcp_listen(pcb);				
  
  /* Specify the function to be called when a connection is established */	
  tcp_accept(pcb, TcpModbus_accept); 
  
  return (struct tcp_pcb *)pcb;										
}


struct tcp_pcb *  main_TcpModbus_init(void)
{
  MODBUS_TcpPCB = TcpModbus_init();
  
  return (struct tcp_pcb *)MODBUS_TcpPCB; 
}
/**
* @brief  This function is called when an error occurs on the connection 
* @param  arg
* @parm   err
* @retval None 
*/
static void TcpModbus_conn_err(void *arg, err_t err)
{
  struct RxBuffer  *tempstruct;
  tempstruct = (struct RxBuffer  *)arg;
  
  mem_free(tempstruct);
}

/**
* @brief  This function is called when an error occurs on the connection 
* @param  arg
* @parm   err
* @retval None 
*/
struct udp_pcb * main_udp_timing_init(void)
{
   //TYH:创建UDP客户端, 用于接收对时报文  20130305
  struct udp_pcb *upcb;  
  
  /* Create a new UDP control block  */
  upcb = udp_new();   
  
  /* Bind the upcb to any IP address and the UDP_PORT port*/
  udp_bind(upcb, IP_ADDR_ANY, UDP_TIME_TICK_PORT);
  
  /* Set a receive callback for the upcb */
  udp_recv(upcb, udp_time_tick_callback, NULL);  
  
  return upcb;
}


void udp_time_tick_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  struct UDP_RBUF RxBuffer;
  struct tm t;
  
  
  u8_t *c;
  u16_t len;
  u8_t result = 1;
  
  memset(&RxBuffer,0,45) ;
  
  c = p->payload;
  len = p->len;
  
  if((0xFF != c[0]) && (0x01 != c[1]))
  {
    pbuf_free(p);
    return;
  }
  
  if(len != /*41*/9)
  {
    pbuf_free(p);
    return;
  }
  
  memcpy(RxBuffer.bytes, c, len);
  
  if(RxBuffer.bytes[8]<100)
    t.tm_year =RxBuffer.bytes[8]+2000;
  else
    result = 0;
  
  if((RxBuffer.bytes[7]>0) && (RxBuffer.bytes[7]<13))
    t.tm_mon  =RxBuffer.bytes[7];
  else
    result = 0;
  
  if((RxBuffer.bytes[6]>0) && (RxBuffer.bytes[6]<32))  
    t.tm_mday =RxBuffer.bytes[6]&0x1f;
  else
    result = 0;
  
  if(RxBuffer.bytes[5]<24)
    t.tm_hour =RxBuffer.bytes[5];
  else
    result = 0;
  
  if(RxBuffer.bytes[4]<60)  
    t.tm_min  =RxBuffer.bytes[4];
  else
    result = 0;
  
  t.tm_sec  =(RxBuffer.bytes[3]*256+RxBuffer.bytes[2])/1000;
  if(t.tm_sec > 59)
    result = 0;
  
  if(result == 0)
  {
    telprintf("udp Timing fault!\r\n");
  }
  else
  {
    telprintf("udp sent timetamp as:%4d-%02d-%02d %02d:%02d:%02d\r\n",t.tm_year,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
    Time_SetCalendarTime(t);
  }
  
  pbuf_free(p);
  
  return;
}


void udp_timing_close( struct udp_pcb *upcb)
{
  //关闭UDP对时连接
  udp_remove(upcb);
  
  return;
}


struct udp_pcb * udp_test_init(void)
{
   struct udp_pcb *upcb;
   struct pbuf *p;
   
   char buf_test[10] = {0x00, 0x01, 0x02, 0x03};
   u16_t len;   
   
   struct ip_addr ipaddr;
   IP4_ADDR(&ipaddr, 172, 20, 255, 255);      
   
   /* Create a new UDP control block  */
   upcb = udp_new();   
   
   /* Bind the upcb to any IP address and the UDP_PORT port*/
   udp_bind(upcb, IP_ADDR_ANY, 10000);   
   
   p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_RAM);
   
   len = sizeof(buf_test);
   memcpy(p->payload, &buf_test[0], len);
   p->tot_len = p->len = len;
   
   udp_sendto(upcb, p, &ipaddr, 8958);
   
   /* Free the p buffer */
   pbuf_free(p);
   udp_timing_close(upcb);
   
   return upcb;
}


/******************* (C) COPYRIGHT 2009 STMicroelectronics *********/



