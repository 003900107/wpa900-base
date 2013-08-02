
/*écrivain chinuxer 
神却拣选了世上愚钝的，叫有智慧的羞愧；又拣选了世上羸弱的，叫那强壮的羞愧。
——————哥林多前
pad 客户端程序      袁博*/
/* Includes ------------------------------------------------------------------*/
#include "Pad_client.h"
/* Private typedef -----------------------------------------------------------*/
#define UDP_SERVER_PORT      5185
#define TCP_PORT      54321
#define MAX_BUFF_SIZE 32
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static struct tcp_pcb *TcpPCB;

struct RxBuffer 
{
  int length;
  char bytes[MAX_BUFF_SIZE];
};



/* Private function prototypes -----------------------------------------------*/

extern float I2C_MeasureTab[MEANUM];
extern Setting SetCurrent;
extern uint8_t DevIPAddressTab[4];
extern NetState comstate[4];
extern const MeasureItem MeaPropTab[28];
extern DisignItem DisignTab[16];
extern DiStatus_Type DiStatus_DI[16];

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Initialize the client application.
* @param  None
* @retval None
*/
struct udp_pcb * Pad_client_init(void)
{
  struct udp_pcb *upcb;
  
  /* Create a new UDP control block  */
  upcb = udp_new();   
  
  /* Bind the upcb to any IP address and the UDP_PORT port*/
  udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_PORT);
  
  /* Set a receive callback for the upcb */
  udp_recv(upcb, Pad_udp_client_callback, NULL);
  
  return (struct udp_pcb *)upcb;
}

static void Pad_conn_err(void *arg, err_t err)
{
  struct RxBuffer  *tempstruct;
  tempstruct = (struct RxBuffer  *)arg;
  
  mem_free(tempstruct);
}



void Pad_udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  struct tcp_pcb *pcb;
  u16 timeout=0xF000;
  char hello[300];
  
  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, TCP_PORT);
  comstate[PAD].foreignIP=addr->addr;
  if(ERR_OK == tcp_connect(pcb, addr, TCP_PORT, Pad_tcp_client_connected)) comstate[PAD].state=1;
  
  tcp_arg(pcb, mem_calloc(sizeof(struct RxBuffer), 1));	
  tcp_err(pcb, Pad_conn_err);
  
  
  introduction(hello);
  while(tcp_write(pcb, hello, strlen(hello), TCP_WRITE_FLAG_COPY))
  {
    if(0==timeout--) 
      break;
  }
  tcp_recv(pcb,Pad_recv);
  pbuf_free(p);
}


err_t Pad_tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  TcpPCB = tpcb;
  return ERR_OK;
}

uint32_t Pad_process_rx_data( uint8_t *byReadBuf,uint8_t *tx_buff,uint32_t rx_size )
{
  char i,*p,result;
  Setting *setaddr;
  
  setaddr=&SetCurrent;
  
  p=tx_buff;
  
  if(byReadBuf[1]=='m'&&byReadBuf[2]=='e'&&byReadBuf[3]=='a')
  {
    result = AiQuerry();
    if(result)
    {
      sprintf(p,"@celiangshuju:");
      p+=strlen("@celiangshuju:");
      for(i=0;i<MEANUM;i++)
      {
        sprintf(p, "%s-%s-0-0-%5.1f-0:",MeaPropTab[i].Title,MeaPropTab[i].Unit,I2C_MeasureTab[i]);
        p+=(14+strlen(MeaPropTab[i].Title)+strlen(MeaPropTab[i].Unit));
        
      }
    }
    else
    {
      sprintf(p,"data illegal");
      p+=strlen("data illegal");
    }
  }
  else if(byReadBuf[1]=='d'&&byReadBuf[2]=='i'&&byReadBuf[3]=='c')
  {
    sprintf(p,"@xinhaoshuju:");
    p+=strlen("@xinhaoshuju:");
    for(i=0;i<16;i++)
    {
      sprintf(p, "%s-%1d-%1d-%1d:",DisignTab[i].Title,DiStatus_DI[i].Value,DiStatus_DI[i].Value,DisignTab[i].Demo_flag);
      p+=strlen(DisignTab[i].Title);
      p+=7;
    }
  }
  else if(byReadBuf[1]=='d'&&byReadBuf[2]=='o')
  {
    //tyh 20130802  此处需添加I2C出错后，通知其他从板
    if((byReadBuf[3]>0x30)&&(byReadBuf[3]<0x40))
      DoExecute(byReadBuf[3]-0x30);
  }
  else if(byReadBuf[1]=='c'&&byReadBuf[2]=='a'&&byReadBuf[3]=='l')
  {
    Calibration();
  }
  else if(byReadBuf[1]=='r'&&byReadBuf[2]=='s'&&byReadBuf[3]=='t')
  {
    NVIC_SystemReset();
  }
  else if(byReadBuf[1]=='t'&&byReadBuf[2]=='x'&&byReadBuf[3]=='d')
  {
    sprintf(p,"@txdk:");
    p+=strlen("@txdk:");
    sprintf(p, "%6d-%3d-%3d-%3d-%3d:",SetCurrent.Com1Para.Baud,SetCurrent.Com1Para.Parity,\
      SetCurrent.Com1Para.WordLength,SetCurrent.Com1Para.StopBits,SetCurrent.Com1Para.Protocol);
    p+=23;
    sprintf(p, "%6d-%3d-%3d-%3d-%3d",SetCurrent.Com2Para.Baud,SetCurrent.Com2Para.Parity,\
      SetCurrent.Com2Para.WordLength,SetCurrent.Com2Para.StopBits,SetCurrent.Com2Para.Protocol);
    p+=22;
  }
  else if(byReadBuf[0]=='#'&&byReadBuf[1]=='s'&&byReadBuf[2]=='e'&&byReadBuf[3]=='t')
  {
    SetCurrent.Com1Para.Baud= atoi(byReadBuf+5);
    p=strpbrk(byReadBuf+5,"-")+1;
    SetCurrent.Com1Para.Parity= atoi(p);
    p=strpbrk(p,"-")+1;	
    SetCurrent.Com1Para.WordLength=atoi(p);
    p=strpbrk(p,"-")+1;	
    SetCurrent.Com1Para.StopBits=atoi(p);
    p=strpbrk(p,"-")+1;	
    SetCurrent.Com1Para.Protocol=atoi(p);
    p=strpbrk(p,"-")+1;	
    SetCurrent.Com2Para.Baud=atoi(p);
    p=strpbrk(p,"-")+1;	
    SetCurrent.Com2Para.Parity= atoi(p);
    p=strpbrk(p,"-")+1;	
    SetCurrent.Com2Para.WordLength=atoi(p);
    p=strpbrk(p,"-")+1;	
    SetCurrent.Com2Para.StopBits=atoi(p);
    p=strpbrk(p,"-")+1;	
    SetCurrent.Com2Para.Protocol=atoi(p);
    
#if STORE_METHOD == FLASH_METHOD 
    if( DataBase_Write(STORAGE_ROMADDR,(u32 *)setaddr,sizeof(Setting)))
#endif
#if STORE_METHOD == BKP_METHOD
      if( DataBase_Write(STORAGE_ROMADDR,(u16 *)setaddr,sizeof(Setting)))
#endif
      {
        sprintf(tx_buff,"it's OK");
        p=tx_buff+strlen("it's OK");
      }
      else 
      {
        sprintf(tx_buff,"error");
        p=tx_buff+strlen("error");
      }
  }
  return(p-tx_buff);	
}

static err_t Pad_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  struct pbuf *q;
  struct RxBuffer *RxBuffer = (struct RxBuffer *)arg;
  uint8_t i,done;
  char *c,Tx_Buf[512];
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
          done=((('@'==c[0])&&(i==3))||('#'==c[0]));
        }
      }
    }
    if(done) 
    {
      charsum=Pad_process_rx_data(RxBuffer->bytes,Tx_Buf,RxBuffer->length);
      if(charsum) 
      {
        if(tcp_write(pcb, Tx_Buf, charsum, TCP_WRITE_FLAG_COPY)) comstate[PAD].state=0;
      }
      memset(Tx_Buf,0,charsum);
      RxBuffer->length = 0;
      
    }
    pbuf_free(p);
  }
  else if (err == ERR_OK) 
  {
    mem_free(RxBuffer);
    return tcp_close(pcb);
  }
  return ERR_OK;
}

struct tm verstime(time_t t_t)
{
  struct tm t_tm;
  
  t_tm = Time_ConvUnixToCalendar(t_t);
  
  return t_tm;
}

void introduction(uint8_t *tx_buff)
{
  struct tm ver_tm;
  
  ver_tm=verstime(SetCurrent.cpledtm);
  sprintf(tx_buff,"@ip:%d.%d.%d.%d:%s:%d.%d:%d-%d-%d:%dkVA:^_^:^o^:(=_=)",DevIPAddressTab[0],DevIPAddressTab[1],DevIPAddressTab[2],\
    DevIPAddressTab[3],SetCurrent.DevNameChar,SetCurrent.vers/10,SetCurrent.vers%10,ver_tm.tm_year,ver_tm.tm_mon,\
      ver_tm.tm_mday,SetCurrent.capa);
}

void Pad_soe_trig(SOE_Struct *soe)
{
  char str[32];
  struct tm t_tm;
  
  t_tm = Time_ConvUnixToCalendar(soe->timetamp);
  
  sprintf(str,"@alm:%d-%d-%d-%d-%d-%d",soe->diseq,soe->state,t_tm.tm_hour,t_tm.tm_min,t_tm.tm_sec,soe->mscd);
  if(tcp_write(TcpPCB, str, strlen(str), TCP_WRITE_FLAG_COPY)) comstate[PAD].state=0;
}

/******************* sac_wpa900 风电工程部 *****la fin de documant****/