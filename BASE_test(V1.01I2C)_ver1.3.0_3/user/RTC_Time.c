
/* Includes ------------------------------------------------------------------*/
//#include "stm32f10x_lib.h"
#include "RTC_Time.h"
#include "bsp.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : Time_ConvUnixToCalendar(time_t t)
* Description    : ת��UNIXʱ���Ϊ����ʱ��
* Input 		 : u32 t  ��ǰʱ���UNIXʱ���
* Output		 : None
* Return		 : struct tm
*******************************************************************************/
struct tm Time_ConvUnixToCalendar(time_t t)
{
  struct tm *t_tm;
  t_tm = localtime(&t);
  t_tm->tm_year += 1900;	//localtimeת�������tm_year�����ֵ����Ҫת�ɾ���ֵ
  t_tm->tm_mon += 1;	//TYH:20130312 ��ԭ�·�
  
  return *t_tm;
}

/*******************************************************************************
* Function Name  : Time_ConvCalendarToUnix(struct tm t)
* Description    : д��RTCʱ�ӵ�ǰʱ��
* Input 		 : struct tm t
* Output		 : None
* Return		 : time_t
*******************************************************************************/
time_t Time_ConvCalendarToUnix(struct tm t)
{
  t.tm_year -= 1900;   //�ⲿtm�ṹ��洢�����Ϊ2008��ʽ
  
  t.tm_mon  -= 1; 	  //tyh:20130308 Ϊ�˼���Unixʱ��, �·�"-1"
  
  //��time.h�ж������ݸ�ʽΪ1900�꿪ʼ�����
  //���ԣ�������ת��ʱҪ���ǵ�������ء�
  
  return mktime(&t);
}

/*******************************************************************************
* Function Name  : Time_GetUnixTime()
* Description    : ��RTCȡ��ǰʱ���Unixʱ���ֵ
* Input 		 : None
* Output		 : None
* Return		 : time_t t
*******************************************************************************/
time_t Time_GetUnixTime(void)
{
  return (time_t)RTC_GetCounter();
}

/*******************************************************************************
* Function Name  : Time_GetCalendarTime()
* Description    : ��RTCȡ��ǰʱ�������ʱ�䣨struct tm��
* Input 		 : None
* Output		 : None
* Return		 : time_t t
*******************************************************************************/
struct tm Time_GetCalendarTime(void)
{
  time_t t_t;
  struct tm t_tm;
  
  t_t = (time_t)RTC_GetCounter();
  t_tm = Time_ConvUnixToCalendar(t_t);
  return t_tm;
}

/*******************************************************************************
* Function Name  : Time_SetUnixTime()
* Description    : ��������Unixʱ���д��RTC
* Input 		 : time_t t
* Output		 : None
* Return		 : None
*******************************************************************************/
void Time_SetUnixTime(time_t t)
{
  RTC_WaitForLastTask();
  RTC_SetCounter((u32)t);
  RTC_WaitForLastTask();
  return;
}

/*******************************************************************************
* Function Name  : Time_SetCalendarTime()
* Description    : ��������Calendar��ʽʱ��ת����UNIXʱ���д��RTC
* Input 		 : struct tm t
* Output		 : None
* Return		 : None
*******************************************************************************/
void Time_SetCalendarTime(struct tm t)
{
  Time_SetUnixTime(Time_ConvCalendarToUnix(t));
  return;
}

/*�������*/
uint16_t Get_Msec(void)
{
  uint32_t tempmsec;
  uint16_t result;
  
  tempmsec=RTC_GetDivider();
  result=(32767-tempmsec)*1000/32767;
  
  return result;
}