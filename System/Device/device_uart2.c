#include "system.h"

#define USART2_CLK                       RCC_APB1Periph_USART2


typedef struct 
{
    uint8_t    * Start;            //指向队列开始 
    uint8_t    * End;              //指向队列结束 
    uint8_t    * In;               //插入一个消息  
    uint8_t    * Out;              //取出一个消息  
    uint16_t    Entries;          //消息长度      
}QueueStruct;


#define Txd1BufferSum 255
#define Rxd1BufferSum 255
static QueueStruct Txd1Queue;
static QueueStruct Rxd1Queue;
static uint8_t Txd1Buffer[Txd1BufferSum];
static uint8_t Rxd1Buffer[Rxd1BufferSum];
//static uint8_t RecvStartFlag=0;
 uint8_t Recv2Timer=0;


/* Private functions ---------------------------------------------------------*/

/**
* @brief  Main program
* @param  None
* @retval None
*/

static void InitQueue(void)
{
    Txd1Queue.Start = Txd1Buffer;
    Txd1Queue.End = Txd1Buffer + Txd1BufferSum - 1;
    Txd1Queue.In = Txd1Queue.Start;
    Txd1Queue.Out = Txd1Queue.Start;
    Txd1Queue.Entries = 0;

    Rxd1Queue.Start = Rxd1Buffer;
    Rxd1Queue.End = Rxd1Buffer + Rxd1BufferSum - 1;
    Rxd1Queue.In = Rxd1Queue.Start;
    Rxd1Queue.Out = Rxd1Queue.Start;
    Rxd1Queue.Entries = 0;
	
}

/*******************************************************************************
* 描述	    : 串口1数据缓冲区接收一个字节
* 输入参数  : data 字节类型，发送参数
* 返回参数  : TestStatus 类型
*******************************************************************************/
static TestStatus ReadByteFromUsart2(uint8_t* data) 
{
    if (Rxd1Queue.Entries ==0) return (FAILED);
    
    Rxd1Queue.Entries--;
    
    *data =*Rxd1Queue.Out++ ;	
    
    if (Rxd1Queue.Out > Rxd1Queue.End) 
        Rxd1Queue.Out = Rxd1Queue.Start;
    
    return (PASSED);
}

/*******************************************************************************
* 描述	    : 串口1数据缓冲区发送一个字节
* 输入参数  : data 字节类型，发送参数
* 返回参数  : TestStatus 类型
*******************************************************************************/
static TestStatus WriteByteToUsart2(uint8_t data) 
{
    if (Txd1Queue.Entries >= Txd1BufferSum) return (FAILED);
    
    Txd1Queue.Entries++;
    
    *Txd1Queue.In++ = data;	
    
    if (Txd1Queue.In > Txd1Queue.End) 
        Txd1Queue.In = Txd1Queue.Start;
    
    return (PASSED);
}



/*******************************************************************************
* 描述	    : Keil编译器支持printf函数，需要调用此函数，注意这个需要MicorLib支持
* 输入参数  : ch 实为uint8_t类型，实际发送的数据
*           : f 设备，不需要处理
int fputc(int ch, FILE *f)
{
    if (ch == '\n')                         //换行符
        WriteByteToUsart1('\r');            //增加回车符

    WriteByteToUsart1((uint8_t)ch);

    return ch;
}
*******************************************************************************/

/*******************************************************************************
* 描述	    : 系统节拍1000/S,即1mS一次调用此服务程序发送缓冲区数据
*******************************************************************************/
void Usart2TransSystick1000Routine(void) 
{
	if (Txd1Queue.Entries == 0) return;

    if ((USART2->ISR&0X40)==0) return;

    USART2->TDR = *Txd1Queue.Out++;
    Txd1Queue.Entries--;
    
    if (Txd1Queue.Out > Txd1Queue.End) 
        Txd1Queue.Out = Txd1Queue.Start;
}


/*******************************************************************************
* 描述	    : 系统节拍1000/S,即1mS一次调用此服务程序接收缓冲区数据
*******************************************************************************/
void Usart2RecvSystick1000Routine(void) 
{
  if(Recv2Timer)
	{
		Recv2Timer--;
		if(!Recv2Timer)
			PostMessage(MessageCommRecv, Rxd1Queue.Entries);
	}
		
}

/*******************************************************************************
* 描述	    : 串口中断处理函数
*******************************************************************************/
void USART2_IRQHandler(void) 
{
    
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET) 
    {
			if (Rxd1Queue.Entries < Rxd1BufferSum)
			{
				*Rxd1Queue.In++ = (uint8_t)USART_ReceiveData(USART2);				
				if (Rxd1Queue.In > Rxd1Queue.End) 
						Rxd1Queue.In = Rxd1Queue.Start;
				
				Rxd1Queue.Entries++;
				if (Rxd1Queue.Entries >= Rxd1BufferSum)
				{
					PostMessage(MessageCommRecv, Rxd1Queue.Entries);
					Recv2Timer =0;
				}
				Recv2Timer =15;
			}
			else
			{
				USART_ReceiveData(USART2);
			}
    }
		if(USART_GetFlagStatus(USART2, USART_FLAG_ORE)!= RESET)
		{
			USART_ReceiveData(USART2);
			USART_ClearFlag(USART2, USART_FLAG_ORE);
		}
		if(USART_GetFlagStatus(USART2, USART_FLAG_NE)!= RESET)
		{
			USART_ClearFlag(USART2, USART_FLAG_NE);
		}
		if(USART_GetFlagStatus(USART2, USART_FLAG_FE)!= RESET)
		{
			USART_ClearFlag(USART2, USART_FLAG_FE);
		}
		if(USART_GetFlagStatus(USART2, USART_FLAG_PE)!= RESET)
		{
			USART_ClearFlag(USART2, USART_FLAG_PE);
		}
} 



/**
* @brief  Configures the USART Peripheral.
* @param  None
* @retval None
*/
void Init_USART2(void)
{
	USART_InitTypeDef USART_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

	  /** USART2 GPIO Configuration  
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */

  /*Enable or disable the AHB peripheral clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /*Configure GPIO pin : PA */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);


  /*Configure GPIO pin alternate function */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);

  /*Configure GPIO pin alternate function */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

	RCC_APB1PeriphClockCmd(USART2_CLK,ENABLE);
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
  USART_Init(USART2, &USART_InitStructure);

  /* Configure the NVIC Preemption Priority Bits */  
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	InitQueue();

	USART_Cmd(USART2, ENABLE);
	USART_ITConfig  (USART2,USART_IT_RXNE,ENABLE );
//	USART_ITConfig  (USART2,USART_IT_ERR,ENABLE );
	
	System.Device.Usart2.WriteByte=WriteByteToUsart2;
	System.Device.Usart2.ReadByte=ReadByteFromUsart2;

}

