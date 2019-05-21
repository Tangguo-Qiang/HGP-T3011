#include "system.h"

#ifndef WINSEN_HCHO

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

typedef enum
{
    SENSOR_EMPTY       		= (byte)0x00,
    SENSOR_YUNTONG      	= (byte)0x01,
    SENSOR_CUBIC       		= (byte)0x02,
    SENSOR_WEISHENG       = (byte)0x03,
    SENSOR_OTHER   
    
}SensorTypeEnum;

static ushort PM25ug=50;
static byte TxBuffer[8]={0};
static byte RxBuffer[RXBUFFERLENGTH]={0};
static byte DataLength=0;
static byte Index=0;
//static byte SensorState=0;
static byte SensorFaultFlag=0;
static ushort TestVal=0;

byte SensorType=SENSOR_EMPTY;

/* calculate for YunTong&Honeyware*/
static ushort SumCalculate(void)
{
	ushort temp=0;
	unsigned char i;
	for(i = 0;i <(DataLength-2); i++)
	{
			temp += RxBuffer[i];
	}
	return temp;
}
/* calculate for Wuhan Cubic*/
static byte SumCalculatecCubic(void)
{
	unsigned char temp;
	unsigned char i;
	unsigned char len = RxBuffer[1] +2;
	temp = 0;
	for(i = 0;i < len; i++)
	{
			temp += RxBuffer[i];
	}
	temp =(0xff-temp)+1;
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
			if (SensorType!=SENSOR_CUBIC)
			{
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
					if(Index==4)
						DataLength = 4+RxBuffer[3];
					if(Index==DataLength)
					{
						TestVal = ((RxBuffer[DataLength-2]<<8)|RxBuffer[DataLength-1]);
						if(TestVal==SumCalculate())
						{
							PM25ug =((RxBuffer[6]<<8)|RxBuffer[7]);
							SensorType = SENSOR_YUNTONG;
							SensorFaultFlag=0;
						}
						Index = 0;
						InitBuffer();
					}
					
				}
			}
			else
			{
				if(Index ==0)
				{
					if(RxBuffer[0]==0x16)
						Index++;
				}
				else
				{
					Index++;
					if(Index==(RxBuffer[1]+3))
					{
						Index--;
						if(RxBuffer[Index]==SumCalculatecCubic())
						{
							if(RxBuffer[2]== 0x0B)
							{
								PM25ug =RxBuffer[3];
								PM25ug <<=8;
								PM25ug |=RxBuffer[4];
								if(PM25ug)
									PM25ug =1000;
								else
								{
									PM25ug =RxBuffer[5];
									PM25ug <<=8;
									PM25ug |=RxBuffer[6];
									SensorType = SENSOR_CUBIC;
									SensorFaultFlag=0;
								}
								
							}

						}
						Index = 0;
					}
					if(Index>19)
						Index =0;
				}				
			}
    } 
		
		if(USART_GetFlagStatus(USART1, USART_FLAG_ORE)!= RESET)
		{
			USART_ReceiveData(USART1);
			USART_ClearFlag(USART1, USART_FLAG_ORE);
		}
		if(USART_GetFlagStatus(USART1, USART_FLAG_NE)!= RESET)
		{
			USART_ClearFlag(USART1, USART_FLAG_NE);
		}
		if(USART_GetFlagStatus(USART1, USART_FLAG_FE)!= RESET)
		{
			USART_ClearFlag(USART1, USART_FLAG_FE);
		}
		if(USART_GetFlagStatus(USART1, USART_FLAG_PE)!= RESET)
		{
			USART_ClearFlag(USART1, USART_FLAG_PE);
		}
		
}


static void PMCubic_ReadReq(void)
{
	byte i=0;
	
	USART_ITConfig  (USART1,USART_IT_RXNE,DISABLE );
	InitBuffer();
	Index = 0;
/* CubicSensorReq GetData*/
	TxBuffer[0]= 0x11;
	TxBuffer[1]= 0x02;
	TxBuffer[2]= 0x0B;
	TxBuffer[3]= 0x01;
	TxBuffer[4]= 0xE1;

	TimeOut1ms=5;
	for(i=0;i<5;i++)
	{
		while((USART1->ISR&0x40)==0)
		{
			if(!TimeOut1ms)
				break;
		}
    USART1->TDR = (ushort)TxBuffer[i];//*pTxd++;
		TimeOut1ms=5;

	}
	USART_ITConfig  (USART1,USART_IT_RXNE,ENABLE );	
}

static void PMSensor_Start(void)
{
	AIRV_ON;
	SENSOR_ON;
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig  (USART1,USART_IT_RXNE,ENABLE );
	SensorFaultFlag=0;
	PM25ug = 0;
}

static void PMSensor_Stop(void)
{
	AIRV_OFF;
	USART_Cmd(USART1, DISABLE);
	USART_ITConfig  (USART1,USART_IT_RXNE,DISABLE );
}

static uint16_t PM25ug_Get(void)
{
	if(SensorFaultFlag>5)  //req per5s, no data in 50s for fault
	{
		if(!SensorType)
		{
			SensorType =SENSOR_CUBIC;
		}
		PM25ug=1000;
		SensorFaultFlag=5;
	}	
  SensorFaultFlag++;	
	if(SensorType==SENSOR_CUBIC)
		PMCubic_ReadReq();
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
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
	SENSOR_ON;

  /*Configure GPIO pin alternate function */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);

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
   
  USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
	
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
//	USART_Cmd(USART1, ENABLE);
//	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE );
	
	
	System.Device.Usart1.PM25ug_Get=PM25ug_Get;
	System.Device.Usart1.PMSensor_Start=PMSensor_Start;
	System.Device.Usart1.PMSensor_Stop=PMSensor_Stop;
}
#endif
