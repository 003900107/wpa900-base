/*
	écrivain=chinuxer 
神赐给我们的，不是胆怯的心，乃是刚强，仁爱，谨守的心
													--提摩太后
____________________________________________________________________________________
*UDP报文格式
字节0	FFH，表示主站
字节1	01H表示包含对时信息， 00H不包含对时信息
字节2～8 对时信息	字节2	毫秒（0～59999，低字节）
字节3	毫秒（0～59999，高字节）
字节4	分（0～59）
字节5	小时（0～23）
字节6	日期（低5位，1～31）及星期（高3位，1～7，0表示星期无效）
字节7	月（1～12）
字节8	年（0～99，不包括世纪）
------------字节9～24	由主站自由赋值，一般包括主站名称/版本号的ASCII码------------
字节9   主站类别及对时来源
字节10  置1表示装置无需tcp连接该地址
字节11~12  对时间隔（ms）
字节13~24 备用
字节25～40	填0
____________________________________________________________________________________
由主站发往子站的报文（控制方向）
06H	时间同步 ASDU6	
07H	总查询 ASDU7	
0AH	通用分类数据 ASDU10	
14H	一般命令 ASDU20	
15H	通用分类命令 ASDU21	
18H	扰动数据传输的命令 ASDU24	
19H	扰动数据传输的认可 ASDU25	

由子站发往主站的报文（监视方向）
05H	标识 ASDU5	
0AH	通用分类数据 ASDU10	
17H	被记录的扰动表 ASDU23	
1AH	扰动数据传输准备就绪 ASDU26	
1BH	被记录的通道传输准备就绪 ASDU27	
1CH	带标志的状态变位传输准备就绪 ASDU28	
1DH	传送带标志的状态变位 ASDU29	
1EH	传送扰动值 ASDU30	
1FH	传送结束 ASDU31	


______________________________________________________________________

______________________________________________________________________

______________________________________________________________________
轻量级南自以太网103规约设备端代码移植
Code transplanted,client-side lightweight Protocol103 on ethernet
																		  袁博
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
        
  memset(&t, 0, sizeof(t));//tyh:20130308 增加初始化为'0', 避免时间计算出错        
        
	t.tm_year =buffer[6]+2000;
  t.tm_mon =buffer[5]/*-1*/;//tyh:20130308 去除"-1", 交由 Time_SetCalendarTime 统一处理
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
   		telprintf( "udp detected from eyewin：%d.%d.%d.%d\n\r", \
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
	telprintf("103 Protocol disconnect! 囧rz\r\n");
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
			
			mscd=soe->mscd+t_tm.tm_sec*1000;//四个八位组的二进制时间
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

/******************* sac_wpa900 风电工程部 *****la fin de document****/