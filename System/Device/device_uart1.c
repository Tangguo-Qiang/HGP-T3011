#include "system.h"

#define USART1_CLK                       RCC_APB2Periph_USART1

#define AIRQTY_PORT	GPIOA
#define AIRQTY_PIN	GPIO_Pin_10
#define AIRV_PORT	GPIOA
#define AIRV_PIN	GPIO_Pin_12
#define AIRV_OFF  GPIO_ResetBits(AIRV_PORT,AIRV_PIN)
#define AIRV_ON  GPIO_SetBits(AIRV_PORT,AIRV_PIN)

#define TXBUFFERLENGTH  8
#define RXBUFFERLENGTH	20

static ushort PM25ug=50;
static byte TxBuffer[TXBUFFERLENGTH]={0};
static byte RxBuffer[RXBUFFERLENGTH]={0};
static byte Index=0;
static byte SensorState=0;
static byte ReqNoAnswer=0;

/* calculate for Wuhan Cubic*/
static byte SumCalculate(byte* data)
{
	unsigned char temp;
	unsigned char i;
	unsigned char len = data[1] +2;
	temp = 0;
	for(i = 0;i < len; i++)
	{
			temp += data[i];
	}
	temp =(0xff-temp)+1;
	return temp;
}

static void InitBuffer(uint8_t* pbuffer, uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)
	{
		*pbuffer++ = 0;
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
					if(RxBuffer[Index]==SumCalculate(RxBuffer))
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
							}
							ReqNoAnswer=0;
							
						}
						else if(RxBuffer[2]== 0x06)
						{
							if(RxBuffer[3])
								SensorState =0;
							PM25ug =150;
						}
					}
					Index = 0;
				}
				if(Index>19)
					Index =0;
			}
    } 
}

static void SendReq(uint8_t *data)
{
	byte len = data[1]+3;
	byte i=0;
	
	USART_ITConfig  (USART1,USART_IT_RXNE,DISABLE );
	InitBuffer((uint8_t*)&RxBuffer, (uint16_t)RXBUFFERLENGTH);
	Index = 0;
	TimeOut1ms=5;
	for(i=0;i<len;i++)
	{
		while((USART1->ISR&0x40)==0)
		{
			if(!TimeOut1ms)
				break;
		}
    USART1->TDR = (ushort)data[i];//*pTxd++;
		TimeOut1ms=5;

	}
	USART_ITConfig  (USART1,USART_IT_RXNE,ENABLE );

}

static void PMSensor_Req(CubicSensorReqTypedef req)
{
	if(SensorState)
	{
		SensorState++;
		if(SensorState>9)
		{
			AIRV_OFF;
			SensorState =1;
			return;
		}
		else
		{
			AIRV_ON;
			USART_Cmd(USART1, ENABLE);
			req =MEASURE_DYNAMIC;
			PM25ug = 1000;
		}
	}

	switch(req)
	{
		case READ_DATA:
			TxBuffer[0]= 0x11;
		  TxBuffer[1]= 0x02;
		  TxBuffer[2]= 0x0B;
		  TxBuffer[3]= 0x01;
		  TxBuffer[4]= 0xE1;
//		  InitBuffer(RxBuffer,20);
		  SendReq(TxBuffer);
		ReqNoAnswer++;
	
			break;
		case MEASURE_DYNAMIC:
			TxBuffer[0]= 0x11;
		  TxBuffer[1]= 0x02;
		  TxBuffer[2]= 0x06;
		  TxBuffer[3]= 0x01;
		  TxBuffer[4]= 0xE6;
//		  InitBuffer(RxBuffer,20);
		  SendReq(TxBuffer);
						
			break;
		default:
			break;
	}
}

static void PMSensor_Start(void)
{
	AIRV_ON;
	USART_Cmd(USART1, ENABLE);
	SensorState = 1;
	ReqNoAnswer=0;
	PM25ug = 100;
//	PMSensor_Req(MEASURE_DYNAMIC);
}

static void PMSensor_Stop(void)
{
	AIRV_OFF;
	SensorState = 1;
	USART_Cmd(USART1, DISABLE);
	USART_ITConfig  (USART1,USART_IT_RXNE,DISABLE );
}

static uint16_t PM25ug_Get(void)
{
//	if(SensorState)
//	{
//		AIRV_ON;
//		PM25ug = 1000;
//		PMSensor_Req(MEASURE_DYNAMIC);
//	}
	if(ReqNoAnswer>10)
	{
		PM25ug=1000;
		if(ReqNoAnswer<12) //ReqNoAnswer=11
			PMSensor_Stop();
		else if(ReqNoAnswer>20)  //Request per 5s
			PMSensor_Start();
	}		
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
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);


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
	
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE );
	
	
	System.Device.Usart1.PM25ug_Get=PM25ug_Get;
	System.Device.Usart1.PMSensor_Req=PMSensor_Req;
	System.Device.Usart1.PMSensor_Start=PMSensor_Start;
	System.Device.Usart1.PMSensor_Stop=PMSensor_Stop;
}
