#ifndef __SHELLCMD_H
#define __SHELLCMD_H

#include "bsp.h"
#include "stm32f10x.h"
#include <stdbool.h>

/*语言选择*/
#define EN 1
#define CH 0

#define LINGUA CH

/*命令状态*/      
#define CMD_NULL 0x00
//以太网
#define CMD_DI 0x01
#define CMD_AI 0x02
#define CMD_TI 0x04
#define CMD_SET 0x08
#define CMD_COEF 0x10
#define CMD_DO 0x20
#define CMD_ADDR 0x40
#define CMD_DATE 0x80
#define CMD_NAME 0x81
#define CMD_CALB 0x82
#define CMD_PARA 0x84
#define CMD_NET 0x88
#define CMD_DCCAL 0xA0
#define CMD_PROT 0xC0
#define CMD_SOE 0xC1
#define CMD_REC 0xC2
//串口  tyh:20130312 添加串口查看\修改 IP
#define CMD_SHOWIP 0x11
#define CMD_RESTOREIP 0x12
#define CMD_REBOOT 0x13
#define CMD_RESET_ETH 0x14
#define CMD_UPDATE_FIRMWARE 0x15
//tyh:20130530 添加串修改以太网判断时间
#define CMD_ETH_LINK_TIME 0x16
#define CMD_ETH_RECV_TIME 0x17
//tyh:20130730 添加读取AI板复位次数
#define CMD_AI_RESET_COUNT 0x18


/*人机交互状态*/
#define INIT 0x0000
#define PASSWORD_ASSERT 0x0001
#define SYSPW_ASSERT 0x0002
#define SUB_MENU 0x0004
#define ENTER_DATA 0x0008
#define ENTER_ADDR 0x0010
#define SET_DATE 0x0020
#define TI_mA 0x0040
#define MODIFY_NAME 0x0080
#define SET_PARA  0x00100

#define ENTER_RESTOREADDR 0x0012
#define SET_ETH_TIME 0x0013
//#define UPDATE_FIRMWARE 0x0014


struct TagT_MESSAGE{
  uint8_t	m_type;		/* message type */
  uint8_t	m_flag;		/* message flag */
  uint32_t m_intdata[4];		/* message data */
  float m_fltdata;
  unsigned long m_pointer;
};
typedef struct TagT_MESSAGE T_MESSAGE; 


typedef struct Tag_ShellCmd
{
  char CmdStr[16];
  uint32_t(* CmdFunc)(char *msgstring,T_MESSAGE *pMsg);
  uint8_t flag;
}ShellCmd ;

typedef struct Tag_Measure_Item
{
  char Title[16];
  char Unit[5];
  uint16_t scale;
}MeasureItem;

typedef struct Tag_Disign_Item
{
  char Title[16];
  uint8_t Demo_flag;
}DisignItem;

typedef struct Tag_Coef_Item
{
  char Title[4];
}CoefItem;


uint32_t ShellEtrData(char *entry,char *b,uint8_t len);
uint32_t ShellEtrDate(char *entry,char *b,uint8_t len);
uint32_t ShellEtrAddr(char *entry,char *b,uint8_t len);
bool ShellCmdCmp(char *a,const char *b,uint8_t len);
uint32_t ShellCmdMatch( char *a, char *b,uint8_t len);
uint32_t ShellPwdMatch(char *a,char *b,uint8_t len);
uint32_t ShellEtrStr(char *entry,char *b,uint8_t len);
uint32_t ShellEtrPara(char *entry,char *b,uint8_t len);
uint32_t CmdQNA(char *outputstr,T_MESSAGE *message);
uint32_t CmdSetting(char *outputstr,T_MESSAGE *message);
uint32_t CmdShowData(char *outputstr,T_MESSAGE *message);
uint32_t CmdPwdAsserted(char *outputstr,T_MESSAGE *message);
uint32_t CmdRestart(char *outputstr,T_MESSAGE *message);
void CmdnShellInit(void);
uint32_t ShellEtrRestoreIP(char *entry, char *b,uint8_t len);
uint32_t CmdShowIP(char *outputstr,T_MESSAGE *message);
uint32_t ShellSetEthTime(char *entry, char *b,uint8_t len);

#endif

