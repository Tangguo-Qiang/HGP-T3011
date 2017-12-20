/*
********************************************************************************
*文件名   : system.c
*作用     : 系统应用文件
*创建时间 : 2013.7.21
********************************************************************************
*/

#include "system.h"

uint Empty;
void Dummy(void) {};
	
byte GifTimer100ms=0;
byte TimeOut1ms=0;

ushort CriticalNesting = 0;


static const byte HextoAscii[16] = 
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

#define DelayUsTick (MainClock/9000000)

#define DelayMsTick (MainClock/9000)

#define APPLICATION_ADDRESS     (uint32_t)0x08003000

 __IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));

void IAPInit()
{
  uint32_t i = 0;
	
/* Relocate by software the vector table to the internal SRAM at 0x20000000 ***/  

  /* Copy the vector table from the Flash (mapped at the base of the application
     load address 0x08003000) to the base address of the SRAM at 0x20000000. */
  for(i = 0; i < 48; i++)
  {
    VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
  }

  /* Enable the SYSCFG peripheral clock*/
//  RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE); 
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  /* Remap SRAM at 0x00000000 */
  SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
}

void DelayNull(int times)
{
	while(times--);
}

void DelayUsNull(int times)
{
	while(times--)
		DelayNull(DelayUsTick);
}

void DelayMsNull(int times)
{
	while(times--)
		DelayNull(DelayMsTick);
}


byte HexToAscii(byte hex)
{
	Assert(hex < 16);

    if (hex < 16) 
        return HextoAscii[hex];
    else 
         return 0x30;
}



/*******************************************************************************
* 函数名	: PostMessage
* 描述	    : 向LogicTask发送消息
* 输入参数  : messageType: 消息类型
*             data: 消息值
* 返回参数  : 无
*******************************************************************************/
void PostMessage(MessageEnum message, uint data)
{
    GetByte3FromUint(data) = message;
    System.OS.PostMessageQueue(data);	
}


extern void InitDevice(void);

static void Init(void)
{
	IAPInit();
	
	DisableIrq();
    
    
    InitDevice();
	InitOs();
}

SystemStruct System = 
{
	Init,
};


