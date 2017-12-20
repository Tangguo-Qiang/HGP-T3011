#include "system.h"

#define USART1_CLK                       RCC_APB2Periph_USART1

#define SENSORSLEEP_PORT	GPIOA
#define SENSORSLEEP_PIN	GPIO_Pin_9
#define AIRV_PORT	GPIOA
#define AIRV_PIN	GPIO_Pin_12
#define AIRV_OFF  GPIO_ResetBits(AIRV_PORT,AIRV_PIN)
#define AIRV_ON  GPIO_SetBits(AIRV_PORT,AIRV_PIN)

#define SENSOR_ON       GPIO_SetBits(SENSORSLEEP_PORT,SENSORSLEEP_PIN)
#define SENSOR_SLEEP  	GPIO_ResetBits(SENSORSLEEP_PORT,SENSORSLEEP_PIN)

#define TXBUFFERLENGTH  8
#define RXBUFFERLENGTH	32

static ushort PM25ug=50;
//static byte TxBuffer[TXBUFFERLENGTH]={0};
static byte RxBuffer[RXBUFFERLENGTH]={0};
static byte Index=0;
static byte SensorState=0;
static byte SensorFaultFlag=0;
static ushort TestVal=0;
static ushort timer100ms=0;

/* calculate for Wuhan Cubic*/
static ushort SumCalculate(void)
{
	ushort temp=0;
	unsigned char i;
	for(i = 0;i < 30; i++)
	{
			temp += RxBuffer[i];
	}
	return temp;
}

static void InitBuffer(void)
{
	uint8_t i;
	for(i=0;i<32;i++)
	{
		RxBuffer[i] = 0;
	}
}
/*******************************************************************************
* 描述	    : 串口中断处理函数
*******************************************************************************/
void USART1_IRQHandler(void) 
{    
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
    {
			RxBuffer[Index] = (uint8_t)USART_ReceiveData(USART1);
			if(Index<2)
			{
				if(RxBuffer[0]==0x42)
					Index++;
				else if(RxBuffer[1]==0x4d)
					Index++;
				else
					Index = 0;
		  }
			else
			{
				Index++;
				if(Index==32)
				{
					TestVal = ((RxBuffer[30]<<8)|RxBuffer[31]);
					if(TestVal==SumCalculate())
					{
						PM25ug =((RxBuffer[6]<<8)|RxBuffer[7]);
						SensorFaultFlag=0;
					}
					Index = 0;
					InitBuffer();
				}
			}
    } 
}

void YTSensorCtrlSystick10Routine(void)
{	
	if(SensorState)
	{
		timer100ms++;
		if(timer100ms==400) //30seconds
		{
			SENSOR_SLEEP;
		}
		else if(timer100ms >700)
		{
			timer100ms=0;
			SENSOR_ON;
		}
		
	}
}


static void PMSensor_Start(void)
{
	AIRV_ON;
	USART_Cmd(USART1, ENABLE);
	SENSOR_ON;
	timer100ms=0;
	SensorState = 1;
	SensorFaultFlag=0;
	PM25ug = 100;
	USART_ITConfig  (USART1,USART_IT_RXNE,ENABLE );
}

static void PMSensor_Stop(void)
{
	AIRV_OFF;
	SENSOR_SLEEP;
	SensorState = 0;
	USART_Cmd(USART1, DISABLE);
	USART_ITConfig  (USART1,USART_IT_RXNE,DISABLE );
}

static uint16_t PM25ug_Get(void)
{
	if(SensorFaultFlag>10)  //req per5s, no data in 50s for fault
	{
		if(SensorFaultFlag<12) //ReqNoAnswer=11
			PMSensor_Stop();
		else if(SensorFaultFlag>20)  //Request per 5s
			PMSensor_Start();
		PM25ug=1000;
	}	
  SensorFaultFlag++;	
	return PM25ug;
}

void Uart1_Init(void)
{
USART_InitTypeDef USART_InitStructure;  
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

  /*Enable or disable the AHB peripheral clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  GPIO_InitStruct.GPIO_Pin = AIRV_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(AIRV_PORT, &GPIO_InitStruct);
	AIRV_OFF;
   
	  /** USART2 GPIO Configuration  
  PA9   ------> USART2_TX
  PA10   ------> USART2_RX
  */

  /*Configure GPIO pin : PA */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
	SENSOR_SLEEP;

//  /*Configure GPIO pin alternate function */
//  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);

  /*Configure GPIO pin alternate function */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

  /* USART configuration -----------------------------------------------------*/
	RCC_APB2PeriphClockCmd(USART1_CLK,ENABLE);
  
  /* USARTx configuration ----------------------------------------------------*/
  /* USARTx configured as follow:
  - BaudRate = 9600 baud  
  - Word Length = 8 Bits
  - one Stop Bit
  - No parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  /* When using Parity the word length must be configured to 9 bits */
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   
  USART_InitStructure.USART_Mode = USART_Mode_Rx;
  USART_Init(USART1, &USART_InitStructure);
	
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE );
	
	
	System.Device.Usart1.PM25ug_Get=PM25ug_Get;
	System.Device.Usart1.PMSensor_Start=PMSensor_Start;
	System.Device.Usart1.PMSensor_Stop=PMSensor_Stop;
}
