/*����ʱ��ʶ����ʢʱ��֪������ʱ�మ������ʱ����*/
/*�������ܵ��Ǻεȵı��ȣ�������һ�п����У��񶼰��Ҿȳ�����        */
/*103��Լ�������� 

�ϴ�һ����ʱ����¼�ASDU10
�ֽ�	��������	˵��
1	���ͱ�ʶ��TYP��	<10>
2	�ɱ�ṹ�޶��ʣ�VSQ��	81H
3	����ԭ��COT��	<1>�Է�(ͻ��)
4	������ַ	
5	�������ͣ�FUN��	GENͨ�÷��๦��
6	��Ϣ��ţ�INF��	<244>��������Ŀ��ֵ������
7	������Ϣ��ʶ����RII��	�޹�����
8	ͨ�÷������ݼ���Ŀ��NGD��	<1>����n������
9	ͨ�÷����ʶ��ţ�GIN��	���=08H(����Ԫ��)
10		��Ŀ��=01H�������ٶϳ��ڣ�
11	�������KOD��	<1>ʵ��ֵ
12	ͨ�÷�������������GDD��	��������<19>�����ʱ���ʱ�걨��
13		���ݿ��<10>10���ֽ�
14		��Ŀ������״̬<1>
15	ͨ�÷����ʶ���ݣ�GID��
�����ʱ���ʱ�걨��
	��B.6.5
	<1>=����<2>=��
16��17		
���ʱ�䣨RET��

18��19		
������ţ�FAN��

20��21		Milliseconds<0~59999ms>������
22		Minutes<0~59min>������00xx-xxxx
23		Hours<0~23h>������000x-xxxx
*/



#include "103com_func.h"



extern Setting SetCurrent;
extern DiStatus_Type DiStatus_DI[16];
extern float I2C_MeasureTab[MEANUM];

u8 ANSWER_ASDU20(char *ReadBuf,char *WriteBuf)
{
	if(5==ReadBuf[2])
		{
			*WriteBuf++=ASDU5;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x04;	
			*WriteBuf++=ReadBuf[3];	
			*WriteBuf++=0xFE;
			*WriteBuf++=0x03;
			*WriteBuf++=0x02;
			*WriteBuf++='W';	
			*WriteBuf++='P';	
			*WriteBuf++='A';	
			*WriteBuf++='9';	
			*WriteBuf++='0';
			*WriteBuf++='0';
			*WriteBuf++=' ';
			*WriteBuf++=' ';
			*WriteBuf++=0x30+SetCurrent.vers/10;
			*WriteBuf++='.';
			*WriteBuf++=0x30+SetCurrent.vers%10;
			*WriteBuf  =0x30;
			return 19;
		}	
			return 0;
}

u8 ANSWER_ASDU21(char *ReadBuf,char *WriteBuf)
{
	if(0x81==ReadBuf[1])
		{
			*WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x2a;	
			*WriteBuf++=ReadBuf[3];	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF1;
			*WriteBuf++=ReadBuf[6];
			*WriteBuf++=0x00;	
		
			return 8;
		}	
			return 0;
}

#ifdef ASDU6EN
u8 ANSWER_ASDU6(char *ReadBuf,char *WriteBuf)
{
  struct tm t;
  
  memset(&t, 0, sizeof(t));//tyh:20130308 ���ӳ�ʼ��Ϊ'0', ����ʱ��������
  
  t.tm_year =ReadBuf[6]+2000;
  t.tm_mon =ReadBuf[5]/*-1*/;//tyh:20130308 ȥ��"-1", ���� Time_SetCalendarTime ͳһ����
  t.tm_mday =ReadBuf[4]&0x1f;
  t.tm_hour =ReadBuf[3];
  t.tm_min =ReadBuf[2];
  t.tm_sec =ReadBuf[1]*256+ReadBuf[0];
  t.tm_sec/=1000;
  Time_SetCalendarTime(t);
}
#endif
u8 ANSWER_ASDU7(char *ReadBuf,char *WriteBuf)
{
u8 i;
	if((0x09==ReadBuf[2])&&(0x01==ReadBuf[3])&&(254==ReadBuf[4])&&(0==ReadBuf[5]))
		{
			*WriteBuf++=ASDU10;	
			*WriteBuf++=0x81;	
			*WriteBuf++=0x09;	
			*WriteBuf++=ReadBuf[3];	
			*WriteBuf++=0xFE;
			*WriteBuf++=0xF1;
			*WriteBuf++=ReadBuf[6];
			*WriteBuf++=0x10;
			for(i=1;i<17;i++)
			{
			*WriteBuf++=0x08;
			*WriteBuf++=i;
			*WriteBuf++=0x01;	
			*WriteBuf++=0x09;	
			*WriteBuf++=0x01;	
			*WriteBuf++=0x01;	
			*WriteBuf++=DiStatus_DI[i-1].Value+1;
		  }
		  
		return 120;
		}	
			return 0;
}

u8 ANSWER_ASDU10(char *ReadBuf,char *WriteBuf)
{
	if((0x81==ReadBuf[1])&&(0x28==ReadBuf[2])&&(0x01==ReadBuf[3]))
		{
				if(0xF9==ReadBuf[5])//�ٷ�У
					{
						*WriteBuf++=ASDU10;
						*WriteBuf++=0x81;
						*WriteBuf++=0x2C;
						*WriteBuf++=0x01;
						*WriteBuf++=0xFE;
						*WriteBuf++=0xF9;
						*WriteBuf++=ReadBuf[6];
						*WriteBuf++=ReadBuf[7];
						*WriteBuf++=ReadBuf[8];
						*WriteBuf++=ReadBuf[9];
						*WriteBuf++=ReadBuf[10];
						*WriteBuf++=ReadBuf[11];
						*WriteBuf++=ReadBuf[12];
						*WriteBuf++=ReadBuf[13];
						*WriteBuf++=ReadBuf[14];
						return 15;
					}
				else if(0xFA==ReadBuf[5])	
					{
					if(0x0B==ReadBuf[8])
							{
								if(0x02==ReadBuf[14])
							  DoExecute((unsigned char)(ReadBuf[9]*2));
							  if(0x01==ReadBuf[14])
							  DoExecute((unsigned char)(ReadBuf[9]*2-1));
							  
						*WriteBuf++=ASDU10;
						*WriteBuf++=0x81;
						*WriteBuf++=0x2C;
						*WriteBuf++=0x01;
						*WriteBuf++=0xFE;
						*WriteBuf++=0xFA;
						*WriteBuf++=ReadBuf[6];
						*WriteBuf++=ReadBuf[7];
						*WriteBuf++=ReadBuf[8];
						*WriteBuf++=ReadBuf[9];
						*WriteBuf++=ReadBuf[10];
						*WriteBuf++=ReadBuf[11];
						*WriteBuf++=ReadBuf[12];
						*WriteBuf++=ReadBuf[13];
						*WriteBuf++=ReadBuf[14];
						return 15;
							}		
						
					}
		}
		else return 0;
	
}
	