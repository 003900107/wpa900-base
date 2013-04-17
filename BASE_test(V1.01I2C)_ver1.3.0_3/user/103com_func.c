/*花开时相识，花盛时相知，花败时相爱，花落时相守*/
/*我所忍受的是何等的逼迫，但从这一切苦难中，神都把我救出来了        */
/*103规约公共函数 

上传一个带时标的事件ASDU10
字节	报文内容	说明
1	类型标识（TYP）	<10>
2	可变结构限定词（VSQ）	81H
3	传送原因（COT）	<1>自发(突发)
4	公共地址	
5	功能类型（FUN）	GEN通用分类功能
6	信息序号（INF）	<244>读单个条目的值或属性
7	返回信息标识符（RII）	无关数据
8	通用分类数据集数目（NGD）	<1>以下n组描述
9	通用分类标识序号（GIN）	组号=08H(动作元件)
10		条目号=01H（过流速断出口）
11	描述类别（KOD）	<1>实际值
12	通用分类数据描述（GDD）	数据类型<19>带相对时间的时标报文
13		数据宽度<10>10个字节
14		数目及后续状态<1>
15	通用分类标识数据（GID）
带相对时间的时标报文
	见B.6.5
	<1>=开或<2>=合
16，17		
相对时间（RET）

18，19		
故障序号（FAN）

20，21		Milliseconds<0~59999ms>二进制
22		Minutes<0~59min>二进制00xx-xxxx
23		Hours<0~23h>二进制000x-xxxx
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
  
  memset(&t, 0, sizeof(t));//tyh:20130308 增加初始化为'0', 避免时间计算出错
  
  t.tm_year =ReadBuf[6]+2000;
  t.tm_mon =ReadBuf[5]/*-1*/;//tyh:20130308 去除"-1", 交由 Time_SetCalendarTime 统一处理
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
				if(0xF9==ReadBuf[5])//假返校
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
	