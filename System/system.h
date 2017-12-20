/*
********************************************************************************
*文件名   : system.h
*作用     : 系统库全局头文件
*创建时间 : 2013.7.21
********************************************************************************
*/


#ifndef __SYSTEM_H
#define __SYSTEM_H


//#include <string.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>


#include "sysdefs.h"
/*
1、类型定义兼容C#，尽可能用C#标准，常用的为int
2、char在C#中为16bit的unicode，keil C下为8bit
*/
//8bit
//typedef char                sbyte;	    /*char Keil4默认定义为byte，若要带符号，需要更改keil设置*/
typedef unsigned char       byte;       
//16bit
//typedef short             short
typedef unsigned short      ushort;
//32bit
//typedef int               int;
typedef unsigned int        uint;

//字符串
typedef char *              string;

typedef unsigned char       bool;

typedef void (*function)(void);
#define Function(address)   ((function)(address))()


typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;




#define TRUE                        1
#define FALSE                       0
#define null                        ((void *) 0)                    // 空指针
#define invalid                     0xFF                            // 无效值
#define MainClock                   72000000                        // 系统时钟频率
#define IdentifyNumberAddress       0x1FFFF7E8                      // 芯片ID号

#define SetBit(data, offset)        ((data) |= 1U << (offset))      // 置位
#define ResetBit(data, offset)      ((data) &= ~(1U << (offset)))   // 复位
#define GetBit(data, offset)        (((data) >> (offset)) & 0x01)   // 获取位

#define GetByte0FromUint(data)      ((byte *)(&(data)))[0]
#define GetByte1FromUint(data)      ((byte *)(&(data)))[1]
#define GetByte2FromUint(data)      ((byte *)(&(data)))[2]
#define GetByte3FromUint(data)      ((byte *)(&(data)))[3]

#define GetUshort0FromUint(data)    ((ushort *)(&(data)))[0]
#define GetUshort1FromUint(data)    ((ushort *)(&(data)))[1]

#define GetMessageType(data)        GetByte3FromUint(data)
#define GetMessageData(data)        (data & 0x00FFFFFF)

#define GetByteFrom(address)        *((byte *)(address)) 
#define GetUshortFrom(address)      *((ushort *)(address)) 
#define GetUintFrom(address)        *((uint *)(address)) 

#define Assert(express)  if (!(express)){printf("\nASSERT: " #express "\n");}   //(void)0


typedef enum
{
    MessageKey       		 = 0x01,
    MessageCommRecv      = 0x02,
    MessageCommTrans     = 0x03,
    MessageIR    				 = 0x04,
    MessageEngine        = 0x05,
    MessageParaUpdate    = 0x06,
    MessageUpdataDate    = 0x07,
    MessageFuncRecv      = 0x08,
    MessageFuncTrans     = 0x09,
    MessageTimer   
    /*请填充类型*/
}MessageEnum;

#include "device.h"  
#include "os.h"

extern byte TimeOut1ms;
extern uint32_t Empty;
extern void Dummy(void);
extern uint16_t CriticalNesting;
#define	EnterCritical()		__disable_irq(); CriticalNesting++;
#define ExitCritical()		if(--CriticalNesting == 0){__enable_irq();}

#define EnableIrq()         __enable_irq();
#define DisableIrq()        __disable_irq();



extern void DelayNull(int times);

extern void DelayUsNull(int times);

extern void DelayMsNull(int times);

extern byte HexToAscii(byte hex);

extern void PostMessage(MessageEnum message, uint data);

/*******************************************************************************
* 结构体名	: SystemStruct
* 描述	    : msOS中最重要的一个结构体，封装了System层的所有接口
********************************************************************************
*版本     作者            日期            说明
*V0.1    tgq        2013/09/11       初始版本
*******************************************************************************/
typedef struct
{
    void (*Init)(void);
    
    struct Device
    {     
      
        struct Adc1
        {
            void (*ADC1_DataGet)(int8_t* pvalue);
        }Adc1;
      
        struct Beep
        {
            void (*BeepOnShort)(uint8_t val);
            void (*BeepOnLong)(void);
        }Beep;

       
        struct StepMoto
        {
          uint8_t (* STPM_BypassMove)(BypassModeTypedef tocircle);
					void (*BypassStatusMap)(BypassCtrl_TypeDef* BypassMode);
        }StepMoto;
       
        struct MotoFan
        {
          void (* XFMoto_DutySet)(uint16_t XFmotoDuty,uint16_t XFmotoRpm);
          void (* PFMoto_DutySet)(uint16_t PFmotoDuty,uint16_t PFmotoRpm);
					void (* MotoStatusMap)(SysStatus_TypeDef* MotoStatus);
					PowerSetTypedef (*MotoPowerSet)(PowerSetTypedef PowerSet);
        }MotoFan;
        
        struct Usart2
        {
//            void (*Open)(void);
//            void (*Close)(void);
            TestStatus (*WriteByte)(byte data);
            TestStatus (*ReadByte)(byte* data); 

        }Usart2;				
        
        struct Usart3
        {
            void (*Open)(void);
            void (*Close)(void);
            TestStatus (*CommProc)(byte* data);
            TestStatus (*ReadString)(byte* data); 

        }Usart3;				
        				
        struct Usart1
        {
            void(*PMSensor_Start)(void);
            void(*PMSensor_Stop)(void);
#ifdef __Cubic_PM25
					void (*PMSensor_Req)(CubicSensorReqTypedef req);
#endif
            uint16_t (*PM25ug_Get)(void);

        }Usart1;				
				
        struct Pwm
        {
            void(*CO2Sensor_Start)(void);
            void(*CO2Sensor_Stop)(void);
            uint16_t (*CO2ppm_Get)(void);
        }Pwm;
				
				
        struct Rtc
        {
            void (*RTC_SetFlag)(byte reg);
        }Rtc;
        
        struct Timer
        {
            void (*Start)(int id, TimerModeEnum mode, int times, function registerFunction);
            void (*Stop)(int id); 
        }Timer;

        struct Iwdog
        {
            void (*IwdogStart)(uint delay1ms);
            void (*IwdogReload)(void);
        }Iwdog;

        struct Systick
        {
            uint8_t (*Register)(SystickEnum type, function registerFunction);
        }Systick;
        
    }Device;

    struct OS
    {
        uint8_t (* PostMessageQueue)(uint message);
        uint (* PendMessageQueue)(void);
        void (* Start)(void); 
        void (* DelayMs)(int times); 
    }OS;


}SystemStruct;



extern SystemStruct System;

#endif 
