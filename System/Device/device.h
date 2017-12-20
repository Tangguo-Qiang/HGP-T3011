/**
  ******************************************************************************
  * @file    Project/STM32F0xx_StdPeriph_Templates/main.h 
  * @author  MCD Application Team
  * @version V1.3.1
  * @date    17-January-2014
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEVICE_H
#define __DEVICE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"


/* Exported types ------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))
/* Exported constants --------------------------------------------------------*/



#define AIRFLOWCTRL_RUNSTEP_PERSENT	100   //1%


typedef enum 
{
    TimerSystick            = 0,
    TimerMessage            = 1	
}TimerModeEnum;	

typedef enum
{
    Systick10000,
    Systick1000,
    Systick100,
    Systick10
}SystickEnum;

typedef enum{
	POWER_ON =0x01,
	POWER_OFF= 0x02
}PowerSetTypedef;

typedef enum{
	SENSOR_ON =(uint8_t)0x01,
	SENSOR_READ = (uint8_t)0x02,
}SensorActTypedef;

typedef enum{
	RATE10TO06 =(uint8_t)0x00,
	RATE10TO08= (uint8_t)0x01,
	RATE10TO10= (uint8_t)0x02,
	RATE10TO12 = (uint8_t)0x03
}VentilateRateTypedef;

typedef enum{
	BYPASS_EMPTY = 0x00,
	BYPASS_OUT = 0x01,
	BYPASS_IN = 0x02
}BypassModeTypedef;

typedef enum{
	READ_DATA=(uint8_t)1,
	MEASURE_DYNAMIC=(uint8_t)2
}CubicSensorReqTypedef;

typedef struct  {
	BypassModeTypedef BypassMode;
	uint16_t BypassAngle;
}BypassCtrl_TypeDef;

typedef enum{
	HEATER_OFF = (byte)0x00,
	HEATER_HALF = (byte)0x01,
	HEATER_FULL = (byte)0x02,
	HEATER_COOLON = (byte)0x03,
	HEATER_COOLOFF = (byte)0x04
}AuxiliaryHeatTypedef;

//typedef enum{
//	COOLER_OFF = (byte)0x00,
//	COOLER_ON = (byte)0x01
//}AirConditionerTypedef;

typedef enum{
	SWITCH_AC_RS1=(uint8_t)0x01,
	SWITCH_HEAT1=(uint8_t)0x02,
	SWITCH_HEAT2=(uint8_t)0x04,
	SWITCH_ESP=(uint8_t)0x08,
	SWITCH_12V_RS1=(uint8_t)0x10,
	SWITCH_12V_RS2=(uint8_t)0x20
}SwitchTypedef;

typedef struct  {
	BypassCtrl_TypeDef BypassMode;
	AuxiliaryHeatTypedef AuxiliaryHeatSet;
//	AirConditionerTypedef AirConditionerSet;
	uint16_t XFmotoDuty;
	uint16_t PFmotoDuty; //persent of full speed
	uint16_t XFmotoRpm;
	uint16_t PFmotoRpm; //rounds per minute
	PowerSetTypedef Power;
}SysPara_TypeDef;

typedef struct  {
	PowerSetTypedef Power;
	BypassCtrl_TypeDef BypassStatus;
	AuxiliaryHeatTypedef AuxiliaryHeat;
//	AirConditionerTypedef AirConditioner;
	uint16_t XFmotoDuty;
	uint16_t PFmotoDuty; //persent of full speed
	uint16_t XFmotoRpm;
	uint16_t PFmotoRpm; //rounds per minute
	uint8_t XFmotoFault;
	uint8_t PFmotoFault;
}SysStatus_TypeDef;

typedef struct {
	int8_t TempIn;
	int8_t TempOut;
	int8_t RHIn;
	uint16_t PMug;
	uint16_t CO2ppm;
}SensorData_TypeDef;



extern void InitDevice(void);

#endif 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
