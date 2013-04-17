/*
	��crivain=chinuxer 
��͸����ǵģ����ǵ��ӵ��ģ����Ǹ�ǿ���ʰ������ص���
													--��Ħ̫��
____________________________________________________________________________________
*UDP���ĸ�ʽ
�ֽ�0	FFH����ʾ��վ
�ֽ�1	01H��ʾ������ʱ��Ϣ�� 00H��������ʱ��Ϣ
�ֽ�2��8 ��ʱ��Ϣ	�ֽ�2	���루0��59999�����ֽڣ�
�ֽ�3	���루0��59999�����ֽڣ�
�ֽ�4	�֣�0��59��
�ֽ�5	Сʱ��0��23��
�ֽ�6	���ڣ���5λ��1��31�������ڣ���3λ��1��7��0��ʾ������Ч��
�ֽ�7	�£�1��12��
�ֽ�8	�꣨0��99�����������ͣ�
------------�ֽ�9��24	����վ���ɸ�ֵ��һ�������վ����/�汾�ŵ�ASCII��------------
�ֽ�9   ��վ��𼰶�ʱ��Դ
�ֽ�10  ��1��ʾװ������tcp���Ӹõ�ַ
�ֽ�11~12  ��ʱ�����ms��
�ֽ�13~24 ����
�ֽ�25��40	��0
____________________________________________________________________________________
����վ������վ�ı��ģ����Ʒ���
06H	ʱ��ͬ�� ASDU6	
07H	�ܲ�ѯ ASDU7	
0AH	ͨ�÷������� ASDU10	
14H	һ������ ASDU20	
15H	ͨ�÷������� ASDU21	
18H	�Ŷ����ݴ�������� ASDU24	
19H	�Ŷ����ݴ�����Ͽ� ASDU25	

����վ������վ�ı��ģ����ӷ���
05H	��ʶ ASDU5	
0AH	ͨ�÷������� ASDU10	
17H	����¼���Ŷ��� ASDU23	
1AH	�Ŷ����ݴ���׼������ ASDU26	
1BH	����¼��ͨ������׼������ ASDU27	
1CH	����־��״̬��λ����׼������ ASDU28	
1DH	���ʹ���־��״̬��λ ASDU29	
1EH	�����Ŷ�ֵ ASDU30	
1FH	���ͽ��� ASDU31	


______________________________________________________________________

______________________________________________________________________

______________________________________________________________________
������������̫��103��Լ�豸�˴�����ֲ
Code transplanted,client-side lightweight Protocol103 on ethernet
																		  Ԭ��
*/
/* Includes ------------------------------------------------------------------*/



#include "lwp103client.h"
#include "103com_func.h"
#include "bsp.h"
#include "com_manager.h"
#include "Realtimedb.h"


/* Private define ------------------------------------------------------------*/
#define ASDU_ADDR 0x01

#define UDP_PORT  1032
#define TCP_PORT      1048
#define MAX_BUFF_SIZE 16
#define PT103_DEFAITE_CONNEXION 7  //means polling loop,for protocol103 updating to host,the number 7 maybe OK

/* Private typedef -----------------------------------------------------------*/
struct RxBuffer 
{
  int length;
  char bytes[MAX_BUFF_SIZE];
};
struct UDP_RBUF 
{
  int length;
  char bytes[41];
};
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static struct tcp_pcb *TcpPCB;
__IO uint8_t PseyewinIpTab[4];
__IO uint8_t	Pt103_asdu10_cnt;
__IO uint8_t TCP103_BUSYHOLDING_FLAG=0x00;
u8 RII;
/* Global variables ----------------------------------------------------------*/
extern float I2C_MeasureTab[MEANUM];
extern Setting SetCurrent;
extern uint8_t DevIPAddressTab[4];
extern NetState comstate[4];
extern const MeasureItem MeaPropTab[28];
extern u16 CAN_MeasureTab[4];
extern DisignItem DisignTab[16];
extern DiStatus_Type DiStatus_DI[16];
/* Private function prototypes -----------------------------------------------*/




/* Private functions ---------------------------------------------------------*/
err_t	pt103_tcp_write(struct tcp_pcb *pcb, const void *data, u16_t len, u8_t apiflags)
{	 	
	err_t Rstatus=1;
	if(BinSemPend(TCP103_BUSYHOLDING_FLAG))
		 {
			 Rstatus=tcp_write(pcb, data, len, apiflags);
			 BinSemPost(TCP103_BUSYHOLDING_FLAG);
		 }
	return Rstatus;	
}
/**
  * @brief  Initialize the client application.
  * @param  None
  * @retval None
  */
void lw103_client_init(void)
{
   struct udp_pcb *upcb;
                                     
   /* Create a new UDP control block  */
   upcb = udp_new();   
    
   /* Bind the upcb to any IP address and the UDP_PORT port*/
   udp_bind(upcb, IP_ADDR_ANY, UDP_PORT);
   
   /* Set a receive callback for the upcb */
   udp_recv(upcb, lw103_udp_client_callback, NULL);

  
}

static void lw103_conn_err(void *arg, err_t err)
{
  struct RxBuffer  *tempstruct;
  tempstruct = (struct RxBuffer  *)arg;

  mem_free(tempstruct);
}

void sub_udp_timesync(char *buffer)
{
	struct tm t;
        
  memset(&t, 0, sizeof(t));//tyh:20130308 ���ӳ�ʼ��Ϊ'0', ����ʱ��������        
        
	t.tm_year =buffer[6]+2000;
  t.tm_mon =buffer[5]/*-1*/;//tyh:20130308 ȥ��"-1", ���� Time_SetCalendarTime ͳһ����
  t.tm_mday =buffer[4]&0x1f;
  t.tm_hour =buffer[3];
  t.tm_min =buffer[2];
  t.tm_sec =buffer[1]*256+buffer[0];
  t.tm_sec/=1000;
  telprintf("udp sent timetamp as:%4d-%02d-%02d %02d:%02d:%02d\r\n",t.tm_year,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
  Time_SetCalendarTime(t);	
}

u8 udp_timesync(struct udp_pcb *upcb, struct pbuf *p)
{
	 struct pbuf *q;
	 u8 done=0;
	 u8 i,*c;
	struct UDP_RBUF RxBuffer;

	memset(&RxBuffer,0,45) ;

	for(q=p; q != NULL; q = q->next) 
    {
          c = q->payload;
          for(i=0; i<q->len && !done ; i++) 
	   {
            if(RxBuffer.length < 41) 
	    {
               RxBuffer.bytes[RxBuffer.length++] = c[i];
               done=((0xFF==c[0])&&(0x01==c[1])&&(i==40));
      }
     }
    }
    if(done)
	{
	 sub_udp_timesync(&(RxBuffer.bytes[2]));

		if(0x01== RxBuffer.bytes[10])
		return 0;
		else
		return 1;
	}
	else
	{
		return 0;
	}
}

void lw103_udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  struct tcp_pcb *pcb;
  u8 PseyewinIpTab[4];

 
 	if((udp_timesync(upcb,p))&&( comstate[P103].state==0))
  {
  	comstate[P103].foreignIP=addr->addr;
   //udp from eyewin
 	PseyewinIpTab[0] = (uint8_t)((uint32_t)(addr->addr) >> 24);  
  	PseyewinIpTab[1] = (uint8_t)((uint32_t)(addr->addr) >> 16);
  	PseyewinIpTab[2] = (uint8_t)((uint32_t)(addr->addr) >> 8);
  	PseyewinIpTab[3] = (uint8_t)((uint32_t)(addr->addr));
    
	 if((0xFF!=PseyewinIpTab[1])&&(0xFF!=PseyewinIpTab[0]))
    {			
   		telprintf( "udp detected from eyewin��%d.%d.%d.%d\n\r", \
  	PseyewinIpTab[3], PseyewinIpTab[2], PseyewinIpTab[1], PseyewinIpTab[0]);
			
  	pcb = tcp_new();
  	tcp_bind(pcb, IP_ADDR_ANY, TCP_PORT);
  	if(ERR_OK == tcp_connect(pcb, addr, TCP_PORT, lw103_tcp_client_connected)) comstate[P103].state=1;
  	tcp_arg(pcb, mem_calloc(sizeof(struct RxBuffer), 1));	
  	tcp_err(pcb, lw103_conn_err);
  	tcp_recv(pcb,lw103_recv);
  	}
  }
  	pbuf_free(p);
}


err_t lw103_tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  TcpPCB = tpcb;
  telprintf("103 protocol client linkup PCB_Pointer=0X%X\n\r",TcpPCB);
  return ERR_OK;
}

uint32_t lw103_process_rx_data( char *ReadBuf,char *WriteBuf,uint32_t rx_size )
{
	u8 checksum;

	switch(ReadBuf[0])
	{
		#ifdef ASDU6EN
		case ASDU6:
			checksum=ANSWER_ASDU6(ReadBuf+6,WriteBuf);
			break;
		#endif
		case ASDU7:
			checksum=ANSWER_ASDU7(ReadBuf,WriteBuf);
			break;
		case ASDU20:
			checksum=ANSWER_ASDU20(ReadBuf,WriteBuf);
			break;
		case ASDU21:
			checksum=ANSWER_ASDU21(ReadBuf,WriteBuf);
			break;
		case ASDU10:
			checksum=ANSWER_ASDU10(ReadBuf,WriteBuf);
			break;
		default:
			break;

	}
	comstate[P103].state=2;
return checksum;	
}

static err_t lw103_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  struct pbuf *q;
  struct RxBuffer *RxBuffer = (struct RxBuffer *)arg;
  uint8_t i,done;
  char *c,Tx_Buf[128];
  uint16_t charsum=0;
 

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
          for(i=0; i<q->len; i++) 
	  {
             if(RxBuffer->length < MAX_BUFF_SIZE) 
	    	{
               RxBuffer->bytes[RxBuffer->length++] = c[i];
			   done=((0x06==RxBuffer->bytes[0])\
			   		||(0x07==RxBuffer->bytes[0])\
			   		||(0x0A==RxBuffer->bytes[0])\
			   		||(0x14==RxBuffer->bytes[0])\
			  		||(0x15==RxBuffer->bytes[0]));
            }
      }
    }
    if(done) 
    {
	     charsum=lw103_process_rx_data(RxBuffer->bytes,Tx_Buf,RxBuffer->length);	 

	  if(charsum) 
	  {
	  	if(pt103_tcp_write(pcb, Tx_Buf, charsum, TCP_WRITE_FLAG_COPY)) comstate[P103].state=0;
	  }
	  memset(Tx_Buf,0,charsum);
    RxBuffer->length = 0;
    
    }
	pbuf_free(p);
   }
   else if (err == ERR_OK) 
  {
    mem_free(RxBuffer);
	comstate[P103].state=0;
	telprintf("103 Protocol disconnect! ��rz\r\n");
    return tcp_close(pcb);
  }
  return ERR_OK;
}

void P103_soe_trig(SOE_Struct *soe)
{
	struct tm t_tm;
  u16 mscd;
  char Buf[19];
  char *WriteBuf=Buf;
	t_tm = Time_ConvUnixToCalendar(soe->timetamp);
	
	    	*WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF4;
			*WriteBuf++=RII+1;
			*WriteBuf++=0x01;
			*WriteBuf++=0x08;	
			*WriteBuf++=soe->diseq;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0x12;
			*WriteBuf++=0x06;
			*WriteBuf++=0x01;
			*WriteBuf++=soe->state+1;
			
			mscd=soe->mscd+t_tm.tm_sec*1000;//�ĸ���λ��Ķ�����ʱ��
			memcpy(WriteBuf,&mscd,2);
			WriteBuf++;
			WriteBuf++;
			*WriteBuf++=t_tm.tm_min;
			*WriteBuf=t_tm.tm_hour;
	if(pt103_tcp_write(TcpPCB, Buf,19, TCP_WRITE_FLAG_COPY)) 
	{
		comstate[P103].offline_cnt++;
	}
	else
	{
		tcp_output(TcpPCB);
	}
	

  			WriteBuf=Buf;
  			*WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF4;
			*WriteBuf++=RII+1;
			*WriteBuf++=0x01;
			*WriteBuf++=0x08;	
			*WriteBuf++=soe->diseq;
			*WriteBuf++=0x01;	
			*WriteBuf++=0x09;
			*WriteBuf++=0x01;
			*WriteBuf++=0x01;
			*WriteBuf++=soe->state+1;	
			if(pt103_tcp_write(TcpPCB, Buf,15, TCP_WRITE_FLAG_COPY))
			{
				comstate[P103].offline_cnt++;
			}
			else
			{
				tcp_output(TcpPCB);
			}
}

void UPDATA_ASDU10(u8 quartre)
{
	u8 i;
	char Buf[68];
	char *WriteBuf=Buf;
	u16 value;
	
	if(comstate[P103].offline_cnt>PT103_DEFAITE_CONNEXION) 
		{
			comstate[P103].state=0;
			comstate[P103].offline_cnt=0;
		}
	switch(quartre)
	{
		case 0:
		{
 		    *WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x02;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF4;
			*WriteBuf++=RII+1;
			*WriteBuf++=0x08;
			for(i=1;i<9;i++)
			{
			*WriteBuf++=0x08;
			*WriteBuf++=i;
			*WriteBuf++=0x01;	
			*WriteBuf++=0x09;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0x01;	
			*WriteBuf++=DiStatus_DI[i-1].Value+1;
		    }
			if(pt103_tcp_write(TcpPCB, Buf,64, TCP_WRITE_FLAG_COPY))
			comstate[P103].offline_cnt++;
			
		 }
		 break;
		 case 1:
		{
 		    *WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x02;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF4;
			*WriteBuf++=RII+1;
			*WriteBuf++=0x08;
			for(i=9;i<17;i++)
			{
			*WriteBuf++=0x08;
			*WriteBuf++=i;
			*WriteBuf++=0x01;	
			*WriteBuf++=0x09;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0x01;	
			*WriteBuf++=DiStatus_DI[i-1].Value+1;
		    }
			if(pt103_tcp_write(TcpPCB, Buf,64, TCP_WRITE_FLAG_COPY))
			comstate[P103].offline_cnt++;
			
		 }
		 break;
		 case 2:
		 {
		  	 WriteBuf=Buf;  
			*WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x02;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF4;
			*WriteBuf++=RII+1;
			*WriteBuf++=0x06;
			for(i=1;i<7;i++)
			{
			*WriteBuf++=0x07;
			*WriteBuf++=i;
			*WriteBuf++=0x01;	
			*WriteBuf++=0x06;	
			*WriteBuf++=0x04;	
			*WriteBuf++=0x01;	
			memcpy(WriteBuf,&(I2C_MeasureTab[i-1]),4);
		    WriteBuf+=4;
		 	}
			if(pt103_tcp_write(TcpPCB, Buf,68, TCP_WRITE_FLAG_COPY))
			 comstate[P103].offline_cnt++;	
		 }
		 break;
		case 3:
		{	
			WriteBuf=Buf;  
			*WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x02;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF4;
			*WriteBuf++=RII+1;
			*WriteBuf++=0x06;
			for(i=7;i<13;i++)
			{
			*WriteBuf++=0x07;
			*WriteBuf++=i;
			*WriteBuf++=0x01;	
			*WriteBuf++=0x06;	
			*WriteBuf++=0x04;	
			*WriteBuf++=0x01;
			memcpy(WriteBuf,&(I2C_MeasureTab[i-1]),4);
		    WriteBuf+=4;
		 	}
			if(pt103_tcp_write(TcpPCB, Buf,68, TCP_WRITE_FLAG_COPY)) 
			comstate[P103].offline_cnt++;
		 }
		 break;
		 case 4:
		{	
			WriteBuf=Buf;  
			*WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x02;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF4;
			*WriteBuf++=RII+1;
			*WriteBuf++=0x06;
			for(i=13;i<19;i++)
			{
			*WriteBuf++=0x07;
			*WriteBuf++=i;
			*WriteBuf++=0x01;	
			*WriteBuf++=0x06;	
			*WriteBuf++=0x04;	
			*WriteBuf++=0x01;
			memcpy(WriteBuf,&(I2C_MeasureTab[i-1]),4);
		    WriteBuf+=4;
		 	}
			if(pt103_tcp_write(TcpPCB, Buf,68, TCP_WRITE_FLAG_COPY)) 
			comstate[P103].offline_cnt++;
		 }
		 break;
		  case 5:
		{	
			WriteBuf=Buf;  
			*WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x02;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF4;
			*WriteBuf++=RII+1;
			*WriteBuf++=0x06;
			for(i=19;i<25;i++)
			{
			*WriteBuf++=0x07;
			*WriteBuf++=i;
			*WriteBuf++=0x01;	
			*WriteBuf++=0x06;	
			*WriteBuf++=0x04;	
			*WriteBuf++=0x01;
			memcpy(WriteBuf,&(I2C_MeasureTab[i-1]),4);
		    WriteBuf+=4;
		 	}
			if(pt103_tcp_write(TcpPCB, Buf,68, TCP_WRITE_FLAG_COPY)) 
			comstate[P103].offline_cnt++;
		 }
		 break;
		 case 6:
		 {
		 	WriteBuf=Buf;  
			*WriteBuf++=ASDU10;																		  
			*WriteBuf++=0x81;	
			*WriteBuf++=0x02;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF4;
			*WriteBuf++=RII+1;
			*WriteBuf++=0x05;
			for(i=1;i<5;i++)
			{
			*WriteBuf++=0x07;
			*WriteBuf++=24+i;
			*WriteBuf++=0x01;	
			*WriteBuf++=0x0C;	
			*WriteBuf++=0x02;	
			*WriteBuf++=0x01;
			value=Modbus_Meas_Prescale(CAN_MeasureTab[i-1],MeaPropTab[23+i].scale);	
			memcpy(WriteBuf,&value,2);
		    WriteBuf+=2;
		 	}
			if(pt103_tcp_write(TcpPCB, Buf,48, TCP_WRITE_FLAG_COPY)) 
			comstate[P103].offline_cnt++;
			}
			break;
			default:
			break;
		}
		tcp_output(TcpPCB);
}

void Task_Pt103(void)
{
	if(comstate[P103].state>0)
	{
		UPDATA_ASDU10(Pt103_asdu10_cnt++%PT103_DEFAITE_CONNEXION);
	}
}

/******************* sac_wpa900 ��繤�̲� *****la fin de document****/