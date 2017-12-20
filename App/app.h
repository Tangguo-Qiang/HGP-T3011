#ifndef __APP_H
#define __APP_H

#include "system.h"



/*******************************************************************************
* 描述	    : msOS中最重要的一个结构体，封装了App层的所有数据及窗口控件
*******************************************************************************/
typedef struct  {
	BypassCtrl_TypeDef BypassMode2;
	BypassCtrl_TypeDef BypassMode3;
	BypassCtrl_TypeDef BypassMode4;
	uint8_t SwitchState;
}FuncPara_TypeDef;


typedef struct
{
	SensorData_TypeDef		SensorData;

	SysPara_TypeDef		SysCtrlPara; 
		
	SysStatus_TypeDef SysStatus;
	
	FuncPara_TypeDef  FuncPara;
		
	FuncPara_TypeDef  FuncState;
}AppStruct;

#include "comm.h"
#include "logic.h"
#include "func.h"


extern AppStruct App;

void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLength);
uint8_t Data_Sum(uint8_t* pSrc,uint8_t len);
#endif
