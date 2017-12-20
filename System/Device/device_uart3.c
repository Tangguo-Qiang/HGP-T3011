#include "system.h"

#define USART3_CLK                       RCC_APB1Periph_USART3
#define USER_TIMEOUT				100

static uint8_t Txd3Buffer[8];
static uint8_t Rxd3Buffer[8];
//static uint8_t RecvStartFlag=0;
 uint8_t Recv3Timer=0;



/*******************************************************************************
* 描述	    : 串口3数据缓冲区接收一个字节
* 输入参数  : data 字节类型，发送参数
* 返回参数  : TestStatus 类型
*******************************************************************************/
static TestStatus Usart3ReadString(uint8_t* data) 
{
	byte i;
	
	if(Rxd3Buffer[0] != 0x41)
		return FAILED;
	for(i=0;i<8;i++)
	{
		*data++ = Rxd3Buffer[i];
	}
	for(i=0;i<8;i++)
	{
		Rxd3Buffer[i] = 0;
	}
	return PASSED;
}

/*******************************************************************************
* 描述	    : 系统节拍1000/S,即1mS一次调用此服务程序接收缓冲区数据
*******************************************************************************/
void Usart3RecvSystick1000Routine(void) 
{
  if(Recv3Timer)
	{
		Recv3Timer--;
	}
		
}

static TestStatus Usart3CommProc(byte* data)
{
	byte i=0;
	
	for(i=0;i<8;i++)
	{
		Txd3Buffer[i]=*data++;
	}

	USART_Cmd(USART3, ENABLE);

  /* Clear the TC bit in the SR register by writing 0 to it */
  USART_ClearFlag(USART3, USART_FLAG_TC);
    /* Wait the USART DMA Rx transfer complete or time out */
   for(i=0;i<8;i++)
	{
   Recv3Timer = USER_TIMEOUT;
		USART_SendData(USART3, Txd3Buffer[i]);
		while((USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)&&(Recv3Timer != 0x00))
		{}
	}
	if(!Recv3Timer)
	{
		USART_Cmd(USART3, DISABLE);
		return FAILED;
	}
	
  if(Txd3Buffer[1])
	{
		Recv3Timer = USER_TIMEOUT;
		i=0;
		/* Clear the TC bit in the SR register by writing 0 to it */
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
		while((Recv3Timer)&&(i<8))
		{
			if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE))
			{
				Rxd3Buffer[i] = (uint8_t)USART_ReceiveData(USART3);
				while(USART_GetFlagStatus(USART3, USART_FLAG_RXNE));
				if(Rxd3Buffer[0]==0x41)
					i++;
			}
		}
		if(i>7)
			PostMessage(MessageFuncRecv, Rxd3Buffer[1]);
	}
	
	USART_Cmd(USART3, DISABLE);

	return PASSED;

}



/**
* @brief  Configures the USART Peripheral.
* @param  None
* @retval None
*/
void Init_USART3(void)
{
	USART_InitTypeDef USART_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStruct;
//	NVIC_InitTypeDef NVIC_InitStructure;

	  /** USART3 GPIO Configuration  
  PB10   ------> USART3_TX
  PB11   ------> USART3_RX
  */

  /*Enable or disable the AHB peripheral clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /*Configure GPIO pin : PB */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);


  /*Configure GPIO pin alternate function */
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_4);

  /*Configure GPIO pin alternate function */
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_4);

	RCC_APB1PeriphClockCmd(USART3_CLK,ENABLE);
/* Enable the DMA periph */
//  RCC_AHBPeriphClockCmd(DMA1_CLK, ENABLE);
  
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
  USART_Init(USART3, &USART_InitStructure);

  /* Configure the NVIC Preemption Priority Bits */  
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = USART3_8_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
	

//	USART_ITConfig  (USART3,USART_IT_RXNE,ENABLE );
//	USART_ITConfig  (USART3,USART_IT_ERR,ENABLE );
	
	System.Device.Usart3.CommProc=Usart3CommProc;
	System.Device.Usart3.ReadString=Usart3ReadString;

}

