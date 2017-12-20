/*
********************************************************************************
*文件名     : device_beep.c
*作用       : 系统节拍
*原理       : 
********************************************************************************
*版本     作者            日期            说明
*V1.1     tgq       2016/5/12       初始版本
********************************************************************************
*/

#include "system.h"

#define BEEP_PORT	GPIOB
#define BEEP_PIN	GPIO_Pin_6
#define BEEP_OFF  GPIO_ResetBits(BEEP_PORT,BEEP_PIN)
#define BEEP_ON  	GPIO_SetBits(BEEP_PORT,BEEP_PIN)

static uint8_t BeepTim10ms=0;
static uint8_t SilentTim10ms=0;
static uint8_t Times=0;

static void BeepOnShort(uint8_t val)
{
	if(!val)
	{
		BEEP_OFF;
		return;
	}
	
	Times = val;
	BEEP_ON;
	Times--;
	BeepTim10ms=3;
}

static void BeepOnLong(void)
{
	BEEP_ON;
	Times=0;
	BeepTim10ms=50;
}

void BeepSystick100Routine(void)
{
	if(BeepTim10ms)
	{
		BeepTim10ms--;
		if(!BeepTim10ms)
		{
			BEEP_OFF;
			if(Times)
				SilentTim10ms=10;
		}
	}
	
	if(SilentTim10ms)
	{
		SilentTim10ms--;
		if(!SilentTim10ms)
			BeepOnShort(Times);
	}
}


void Init_Beep(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

  /*Enable or disable the AHB peripheral clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	/* BEEP  PA15*/
  GPIO_InitStruct.GPIO_Pin = BEEP_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(BEEP_PORT, &GPIO_InitStruct);
	BEEP_OFF;
	
	System.Device.Beep.BeepOnLong =BeepOnLong;
	System.Device.Beep.BeepOnShort =BeepOnShort;

}
