																			   /**
  ******************************************************************************
  * @file    helloworld.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   This file contains all the functions prototypes for the helloworld.c 
  *          file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TCPMODBUS_H
#define __TCPMODBUS_H

#include "lwip/tcp.h"
#include "lwip/udp.h"

#define UDP_TIME_TICK_PORT 8958 //tyh:20130305 避免UDP风暴,特殊处理端口号,仅用于对时

#ifdef __cplusplus
 extern "C" {
#endif

   
struct tcp_pcb * main_TcpModbus_init(void);
struct tcp_pcb * TcpModbus_init(void);
struct udp_pcb * main_udp_timing_init(void);

//TYH:创建UDP客户端, 用于接收对时报文  20130305
void udp_time_tick_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
void udp_timing_close(struct udp_pcb *upcb);

struct udp_pcb * udp_test_init(void);


#ifdef __cplusplus
}
#endif

#endif /* __HELLOWERLOD_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

