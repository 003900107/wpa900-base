#ifndef __PADCLIENT_H
#define __PADCLIENT_H

#include  "err.h"
#include "bsp.h"
#include "udp.h"
#include "tcp.h"
#include "soe.h"
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


/** @defgroup helloworld_Exported_Functions
  * @{
  */
u8 udp_timesync(struct udp_pcb *upcb, struct pbuf *p);
void sub_udp_timesync(char *buffer);
err_t lw103_tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
void lw103_client_init(void);
static void lw103_conn_err(void *arg, err_t err);
void lw103_udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
uint32_t lw103_process_rx_data( char *ReadBuf,char *WriteBuf,uint32_t rx_size );
static err_t lw103_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
struct tcp_pcb * TcpModbus_init(void);
void P103_soe_trig(SOE_Struct *soe);
void UPDATA_ASDU10(u8 quartre);
void Task_Pt103(void);
/**
  * @}
  */



#ifdef __cplusplus
}
#endif

#endif /* __PADCLIENT_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
