#ifndef __PADCLIENT_H
#define __PADCLIENT_H


/* Includes ------------------------------------------------------------------*/

#include "stm32f10x.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "BaseBus_Protocol.h"
#include "shellcmd.h"
#include "soe.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/** @defgroup helloworld_Exported_Functions
  * @{
  */
struct udp_pcb * Pad_client_init(void);
void pad_test(void);


void Pad_udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t Pad_tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
void tcp_client_err(void *arg, err_t err);
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t Pad_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
void introduction(uint8_t *tx_buff);
void Pad_soe_trig(SOE_Struct *soe);
/**
  * @}
  */

#endif /* __PADCLIENT_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
