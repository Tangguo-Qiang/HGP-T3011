#include "system.h"


#define CO2_PORT	GPIOB
#define CO2_PIN	GPIO_Pin_1
#define CO2V_PORT	GPIOA
#define CO2V_PIN	GPIO_Pin_11
#define CO2V_OFF  GPIO_ResetBits(CO2V_PORT,CO2V_PIN)
#define CO2V_ON  GPIO_SetBits(CO2V_PORT,CO2V_PIN)

//#define CO2_COUNTCYCLE_1MS   1000

static uint16_t CO2timer1ms=0;
static uint16_t CO2Count=0;
static uint16_t CO2ppm=0;
//static byte SensorType=0;
//static byte SensorState=1;
static byte FaultFlag=0;

function PMSensorSystick1000Routine=Dummy;
function CO2SensorSystick1000Routine=Dummy;

static void CO2SensorHighCountPer1ms(void)
{
	CO2timer1ms++;
  if(GPIO_ReadInputDataBit(CO2_PORT,CO2_PIN)==SET)
		CO2Count++;	
	
		if(CO2timer1ms>1003)  //1004
		{
			CO2ppm	= (CO2Count-2)*5;
			CO2timer1ms =0;
			CO2Count =0;
		}
}

static void CO2Sensor_Start(void)
{
	CO2V_ON;
	CO2timer1ms = 0;
	CO2Count =0;
	CO2ppm=0;
	FaultFlag=0;
	CO2SensorSystick1000Routine=CO2SensorHighCountPer1ms;
}

static void CO2Sensor_Stop(void)
{
	CO2V_OFF;
	CO2ppm=0;
	CO2SensorSystick1000Routine=Dummy;
}
static uint16_t CO2ppm_Get(void)
{
	if(CO2ppm<300)
	{
		FaultFlag++;
		if(FaultFlag>10)
		{
			if(FaultFlag<12)
				CO2Sensor_Stop();
			else if(FaultFlag>20)
				CO2Sensor_Start();
		}
	}
	else
		FaultFlag=0;
		
	return CO2ppm;
}



void Init_PwmCount(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);
	
	/* CO2-PA5*/
  GPIO_InitStruct.GPIO_Pin = CO2_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(CO2_PORT, &GPIO_InitStruct);
	
	
  GPIO_InitStruct.GPIO_Pin = CO2V_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(CO2V_PORT, &GPIO_InitStruct);
	CO2V_OFF;
	
	
	System.Device.Pwm.CO2Sensor_Start=CO2Sensor_Start;
	System.Device.Pwm.CO2Sensor_Stop=CO2Sensor_Stop;	
	System.Device.Pwm.CO2ppm_Get=CO2ppm_Get;

}
