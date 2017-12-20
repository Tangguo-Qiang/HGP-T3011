#include "system.h"

#define STEPMOTO_GPIOPORT  (RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB)

#define MOTO3_PORT1		GPIOB
#define MOTO3_STPM1_PIN	GPIO_Pin_4
#define MOTO3_PORT2		GPIOB
#define MOTO3_STPM2_PIN	GPIO_Pin_3
#define MOTO3_PORT3		GPIOA
#define MOTO3_STPM3_PIN	GPIO_Pin_15
#define MOTO3_PORT4		GPIOA
#define MOTO3_STPM4_PIN	GPIO_Pin_8


//#define BYPASS_ANGLE		30

/* counter_clockwise direction */
uint8_t const gatemotostep[8]={0x08,0x0c,0x04,0x06,0x02,0x03,0x01,0x09};  

static uint16_t totalsteps=0;
static uint16_t movesteps=0;
static uint8_t nextmove=0;
static uint8_t direct =0;
static uint8_t STPMmoving;
static BypassCtrl_TypeDef* pBypassMode;

static void TIM15_ModeConfig(void) 
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	uint16_t PrescalerValue;


  /* TIM9 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);

/* Compute the prescaler value */
  PrescalerValue = (uint16_t) (SystemCoreClock / 1000000) - 1;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 3000-1; //3ms
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);


}

static void STPM_OneStep(void)
{
	if(direct==BYPASS_IN)
	{
		if(gatemotostep[nextmove]&0x01)
			GPIO_SetBits(MOTO3_PORT1,MOTO3_STPM1_PIN);
		else
			GPIO_ResetBits(MOTO3_PORT1,MOTO3_STPM1_PIN);
		if(gatemotostep[nextmove]&0x02)
			GPIO_SetBits(MOTO3_PORT2,MOTO3_STPM2_PIN);
		else
			GPIO_ResetBits(MOTO3_PORT2,MOTO3_STPM2_PIN);
		if(gatemotostep[nextmove]&0x04)
			GPIO_SetBits(MOTO3_PORT3,MOTO3_STPM3_PIN);
		else
			GPIO_ResetBits(MOTO3_PORT3,MOTO3_STPM3_PIN);
		if(gatemotostep[nextmove]&0x08)
			GPIO_SetBits(MOTO3_PORT4,MOTO3_STPM4_PIN);
		else
			GPIO_ResetBits(MOTO3_PORT4,MOTO3_STPM4_PIN);					
				
		nextmove++;
		if(nextmove>7)
			nextmove=0;
	}
	else
	{
		if(gatemotostep[nextmove]&0x01)
			GPIO_SetBits(MOTO3_PORT1,MOTO3_STPM1_PIN);
		else
			GPIO_ResetBits(MOTO3_PORT1,MOTO3_STPM1_PIN);
		if(gatemotostep[nextmove]&0x02)
			GPIO_SetBits(MOTO3_PORT2,MOTO3_STPM2_PIN);
		else
			GPIO_ResetBits(MOTO3_PORT2,MOTO3_STPM2_PIN);
		if(gatemotostep[nextmove]&0x04)
			GPIO_SetBits(MOTO3_PORT3,MOTO3_STPM3_PIN);
		else
			GPIO_ResetBits(MOTO3_PORT3,MOTO3_STPM3_PIN);
		if(gatemotostep[nextmove]&0x08)
			GPIO_SetBits(MOTO3_PORT4,MOTO3_STPM4_PIN);
		else
			GPIO_ResetBits(MOTO3_PORT4,MOTO3_STPM4_PIN);					
				
		if(nextmove==0)
			nextmove=7;
		else
			nextmove--;
	}
	movesteps++;
	if(movesteps>totalsteps)
	{
		pBypassMode->BypassMode = (BypassModeTypedef)direct;
		TIM_Cmd(TIM15, DISABLE);
		TIM_ITConfig(TIM15, TIM_IT_Update, DISABLE);
		STPMmoving	=0;	
	
		GPIO_ResetBits(MOTO3_PORT1,MOTO3_STPM1_PIN);
		GPIO_ResetBits(MOTO3_PORT2,MOTO3_STPM2_PIN);
		GPIO_ResetBits(MOTO3_PORT3,MOTO3_STPM3_PIN);
		GPIO_ResetBits(MOTO3_PORT4,MOTO3_STPM4_PIN);
	}

	
}


void TIM15_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM15, TIM_IT_Update) != RESET)  //per 1ms
  {
    STPM_OneStep();		
    TIM_ClearITPendingBit(TIM15, TIM_IT_Update );
  }
}


static byte STPM_BypassMove(BypassModeTypedef tocircle)
{
	uint16_t temp;
	
	if(STPMmoving)
		return FALSE;
	
	temp = pBypassMode->BypassAngle*10;
//	temp = BYPASS_ANGLE*10;
	temp /= 75;
	totalsteps = (uint8_t)temp +1;
	totalsteps *= 85;
	movesteps = 0;
	if(direct==BYPASS_IN)
		nextmove =0;
	else
		nextmove = 7;
	direct = (uint8_t)tocircle;
	
	STPMmoving =3;
	pBypassMode->BypassMode= BYPASS_EMPTY;
	
  TIM15_ModeConfig();
	TIM_Cmd(TIM15, ENABLE);
  TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);
	
	return TRUE;
}

static void BypassStatusMap(BypassCtrl_TypeDef* BypassMode)
{
	pBypassMode=BypassMode;
}

void Init_StepMoto(void) 
{
  GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

  /*Enable or disable the AHB peripheral clock */
  RCC_AHBPeriphClockCmd(STEPMOTO_GPIOPORT, ENABLE);

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;

  GPIO_InitStruct.GPIO_Pin = MOTO3_STPM1_PIN;
  GPIO_Init(MOTO3_PORT1, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = MOTO3_STPM2_PIN;
  GPIO_Init(MOTO3_PORT2, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = MOTO3_STPM3_PIN;
  GPIO_Init(MOTO3_PORT3, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = MOTO3_STPM4_PIN;
  GPIO_Init(MOTO3_PORT4, &GPIO_InitStruct);
	
  /* --------------------------NVIC Configuration -------------------------------*/
  /* Enable the TIM2 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM15_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);
	
	System.Device.StepMoto.STPM_BypassMove = STPM_BypassMove;
	System.Device.StepMoto.BypassStatusMap = BypassStatusMap;
}
