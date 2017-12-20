#include "system.h"

#define PWM_FREQ_80K	100
#define PWM_FREQ_20K	400
#define PWM_FREQ_12K	666
#define PWM_FREQ_10K		800
#define PWM_FREQ_8K		1000
#define PWM_FREQ_5K		1600
#define PWM_FREQ_1K		8000
#define PWM_PLUSE_PERIOD		PWM_FREQ_10K

#define MOTOFREQ_STEP	5		//5% per step

#define MOTO_STARTDUTY	6000


#define HV_PORT	GPIOB
#define HV_PIN	GPIO_Pin_7
#define HV_OFF  GPIO_ResetBits(HV_PORT,HV_PIN)
#define HV_ON  	GPIO_SetBits(HV_PORT,HV_PIN)

#define MOTOPOWER_PORT	GPIOB
#define MOTOPOWER_PIN		GPIO_Pin_5
#define MOTOPOWER_ON		GPIO_SetBits(MOTOPOWER_PORT,MOTOPOWER_PIN)
#define MOTOPOWER_OFF		GPIO_ResetBits(MOTOPOWER_PORT,MOTOPOWER_PIN)

#define XFMOTOVSP_PORT	GPIOB
#define XFMOTO1VSP_PIN	GPIO_Pin_8
#define XFMOTO1VSP_PINSOURCE	GPIO_PinSource8
#define PFMOTOVSP_PORT	GPIOB
#define PFMOTO2VSP_PIN	GPIO_Pin_9
#define PFMOTO2VSP_PINSOURCE	GPIO_PinSource9
#define XFMOTO1_ON		GPIO_SetBits(XFMOTOVSP_PORT,XFMOTO1VSP_PIN)
#define XFMOTO1_OFF		GPIO_ResetBits(XFMOTOVSP_PORT,XFMOTO1VSP_PIN)
#define PFMOTO2_ON		GPIO_SetBits(PFMOTOVSP_PORT,PFMOTO2VSP_PIN)
#define PFMOTO2_OFF		GPIO_ResetBits(PFMOTOVSP_PORT,PFMOTO2VSP_PIN)

#define XFMOTO1_PORT						GPIOC
#define XFMOTO1_FB_PIN					GPIO_Pin_13
#define XFMOTO1_FB_PORTSOURCE	 	EXTI_PortSourceGPIOC
#define XFMOTO1_FB_EXTISOURCE 	 	EXTI_PinSource13
#define XFMOTO1_FB_EXTI					EXTI_Line13

#define PFMOTO2_PORT						GPIOC
#define PFMOTO2_FB_PIN					GPIO_Pin_14
#define PFMOTO2_FB_PORTSOURCE	 	EXTI_PortSourceGPIOC
#define PFMOTO2_FB_EXTISOURCE 		EXTI_PinSource14
#define PFMOTO2_FB_EXTI					EXTI_Line14

static	TIM_OCInitTypeDef  TIM_OCInitStructure;
static	uint16_t XFmotoDutySet=0; //persent of full speed
static 	uint16_t PFmotoDutySet=0;
static	uint16_t XFmotoDuty=0; //persent of full speed
static 	uint16_t PFmotoDuty=0;
static	uint16_t XFmotoRpmSet=0; //rounds in one minutes
static 	uint16_t PFmotoRpmSet=0;
static	uint16_t XFmotoRpm=0; //rounds in one minutes
static 	uint16_t PFmotoRpm=0;
static	uint16_t XFmotoFBPulse=0; 
static 	uint16_t PFmotoFBPulse=0;
static  uint8_t Times100ms=0;
static  uint8_t MotoFault=0;
static SysStatus_TypeDef* pSysStatus;

static void TIM16_17_ModeConfig(void)
{
	uint16_t PrescalerValue;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;


  /* TIM16 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);

/* Compute the prescaler value */
  PrescalerValue = (uint16_t) (SystemCoreClock / 8000000) - 1;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = PWM_PLUSE_PERIOD-1;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM16, &TIM_TimeBaseStructure);
  TIM_TimeBaseInit(TIM17, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
  TIM_OCInitStructure.TIM_Pulse = PWM_PLUSE_PERIOD;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Set;

  TIM_OC1Init(TIM16, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM16, TIM_OCPreload_Enable);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OC1Init(TIM17, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM17, TIM_OCPreload_Enable);

 
  /* TIM3 enable counter */
  TIM_Cmd(TIM16, ENABLE);
  TIM_Cmd(TIM17, ENABLE);
  /* TIM16 Main Output Enable */
  TIM_CtrlPWMOutputs(TIM16, ENABLE);
  TIM_CtrlPWMOutputs(TIM17, ENABLE);

}

 
static void SetMotoFreq_MOTO2(uint16_t speed)
{
	uint32_t fpctrl;

	fpctrl = speed;
	fpctrl *= PWM_PLUSE_PERIOD;
	fpctrl /= 10000;
  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = PWM_PLUSE_PERIOD-(uint16_t)fpctrl;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
//  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
//  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  TIM_OC1Init(TIM17, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM17, TIM_OCPreload_Enable);

	 
}

static void SetMotoFreq_MOTO1(uint16_t speed)
{
	uint32_t fpctrl;

	fpctrl = speed;
	fpctrl *= PWM_PLUSE_PERIOD;
	fpctrl /= 10000;

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = PWM_PLUSE_PERIOD-(uint16_t)fpctrl;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
//  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
//  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  TIM_OC1Init(TIM16, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM16, TIM_OCPreload_Enable);
	 
}


static void XFMoto_DutySet(uint16_t motoDuty,uint16_t motoRpm)
{
	XFmotoDutySet = motoDuty;
	XFmotoRpmSet = motoRpm;
	pSysStatus->XFmotoDuty = XFmotoDuty;
	if(XFmotoDutySet>=MOTO_STARTDUTY)
	{
		MOTOPOWER_ON;
		HV_ON;
	}
}

static void PFMoto_DutySet(uint16_t motoDuty,uint16_t motoRpm)
{
	PFmotoDutySet = motoDuty;
	PFmotoRpmSet = motoRpm;
	pSysStatus->PFmotoDuty = PFmotoDuty;
}

static void AirMoto1_DutySet(void)
{
	uint16_t Moto1Duty=XFmotoDutySet;
	if(XFmotoDutySet)
	{
		if(XFmotoDutySet>(XFmotoDuty+AIRFLOWCTRL_RUNSTEP_PERSENT))
			Moto1Duty = XFmotoDuty+AIRFLOWCTRL_RUNSTEP_PERSENT;
		else if(XFmotoDuty >(XFmotoDutySet+AIRFLOWCTRL_RUNSTEP_PERSENT))
			Moto1Duty = XFmotoDuty-AIRFLOWCTRL_RUNSTEP_PERSENT;
				
		XFmotoDuty = Moto1Duty;		
	}
	else
	{
		if(XFmotoDuty>(AIRFLOWCTRL_RUNSTEP_PERSENT))
		{
			XFmotoDuty -= AIRFLOWCTRL_RUNSTEP_PERSENT;
		}
		else
		{
			XFmotoDuty = 0;
			HV_OFF;
		}
	}
		
	
	SetMotoFreq_MOTO1(XFmotoDuty);	//*(pInstr+1);
	
}

static void AirMoto2_DutySet(void)
{
	uint16_t Moto2Duty=PFmotoDutySet;
	
		if(PFmotoDutySet)
		{
				if(PFmotoDutySet>(PFmotoDuty+AIRFLOWCTRL_RUNSTEP_PERSENT))
					Moto2Duty = PFmotoDuty+AIRFLOWCTRL_RUNSTEP_PERSENT;
				else if(PFmotoDuty >(PFmotoDutySet+AIRFLOWCTRL_RUNSTEP_PERSENT))
					Moto2Duty = PFmotoDuty-AIRFLOWCTRL_RUNSTEP_PERSENT;
					
			PFmotoDuty = Moto2Duty;		
		}
		else
		{
			if(PFmotoDuty>AIRFLOWCTRL_RUNSTEP_PERSENT)
			{
				PFmotoDuty -= AIRFLOWCTRL_RUNSTEP_PERSENT;
			}
			else
			{
				PFmotoDuty = 0;
			}
			
		}

	
	SetMotoFreq_MOTO2(PFmotoDuty);	//*(pInstr+1);

	
}

void EXTI4_15_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
		PFmotoFBPulse++;
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
	if(EXTI_GetITStatus(EXTI_Line14) != RESET)
	{
		XFmotoFBPulse++;
		EXTI_ClearITPendingBit(EXTI_Line14);
	}
}

void MotoFanSystick10Routine(void)
{
	Times100ms++;
	if(Times100ms>9)
	{
		Times100ms=0;
		/* 12 pulses per 1 round for moto NidecMoto*/
		XFmotoRpm = XFmotoFBPulse*5;
		pSysStatus->XFmotoRpm = XFmotoRpm; 
		XFmotoFBPulse = 0;		
		PFmotoRpm = PFmotoFBPulse*5;
		pSysStatus->PFmotoRpm = PFmotoRpm; 
		PFmotoFBPulse = 0;
	}
	
	
	if(XFmotoRpmSet)
	{
		pSysStatus->XFmotoFault=1;
		if(XFmotoRpmSet>(XFmotoRpm+60))
		{
			XFmotoDuty += 20;
			if(XFmotoDuty>10000)
			{
				XFmotoDuty =10000;
				if(XFmotoRpm<300)
					pSysStatus->XFmotoFault=100;
			}
			SetMotoFreq_MOTO1(XFmotoDuty);
		}
		else if(XFmotoRpmSet>(XFmotoRpm+20))
		{
			XFmotoDuty += 1;
			if(XFmotoDuty>10000)
			{
				XFmotoDuty =10000;
				if(XFmotoRpm<300)
					pSysStatus->XFmotoFault=100;
			}
			SetMotoFreq_MOTO1(XFmotoDuty);
		}
		else if(XFmotoRpm>(XFmotoRpmSet+60))
		{
			if(XFmotoDuty>10)
			{
				XFmotoDuty -= 10;
			}
			else
				pSysStatus->XFmotoFault=100;
			SetMotoFreq_MOTO1(XFmotoDuty);
		}
		else if(XFmotoRpm>(XFmotoRpmSet+20))
		{
			if(XFmotoDuty>1)
			{
				XFmotoDuty -= 1;
			}
			else
				pSysStatus->XFmotoFault=100;
			SetMotoFreq_MOTO1(XFmotoDuty);
		}
	}
	else if(XFmotoDutySet != XFmotoDuty)
	{
		AirMoto1_DutySet();
		pSysStatus->XFmotoDuty = XFmotoDuty;
	}
	
	if(PFmotoRpmSet)
	{
		pSysStatus->PFmotoFault=1;

		if(PFmotoRpmSet>(PFmotoRpm+60))
		{
			PFmotoDuty += 20;
			if(PFmotoDuty>10000)
			{
				PFmotoDuty =10000;
				if(PFmotoRpm<300)
					pSysStatus->PFmotoFault=100;
			}
			SetMotoFreq_MOTO2(PFmotoDuty);
		}
		else if(PFmotoRpmSet>(PFmotoRpm+20))
		{
			PFmotoDuty += 1;
			if(PFmotoDuty>10000)
			{
				PFmotoDuty =10000;
				if(PFmotoRpm<300)
					pSysStatus->PFmotoFault=100;
			}
			SetMotoFreq_MOTO2(PFmotoDuty);
		}
		else if(PFmotoRpm>(PFmotoRpmSet+60))
		{
			if(PFmotoDuty>10)
			{
				PFmotoDuty -= 10;
			}
			else
				pSysStatus->PFmotoFault=100;
			SetMotoFreq_MOTO2(PFmotoDuty);
		}
		else if(PFmotoRpm>(PFmotoRpmSet+20))
		{
			if(PFmotoDuty>1)
			{
				PFmotoDuty -= 1;
			}
			else
				pSysStatus->PFmotoFault=100;
			SetMotoFreq_MOTO2(PFmotoDuty);
		}
	}
	else if(PFmotoDutySet != PFmotoDuty)
	{
		AirMoto2_DutySet();
		pSysStatus->PFmotoDuty = PFmotoDuty;
	}
	

}

static void MotoStatusMap(SysStatus_TypeDef* SysStatus)
{
	pSysStatus=SysStatus;
	
}

static PowerSetTypedef MotoPowerSet(PowerSetTypedef PowerSet)
{
	if(PowerSet == POWER_ON)
	{
		MOTOPOWER_ON;
		HV_ON;
		MotoFault=0;
		pSysStatus->XFmotoFault=1;
		pSysStatus->PFmotoFault=1;
		XFmotoDuty = MOTO_STARTDUTY;
		PFmotoDuty = MOTO_STARTDUTY;
		
		return POWER_ON;
	}
	else if((XFmotoDuty+PFmotoDuty)== 0)
	{
		if((pSysStatus->XFmotoRpm+pSysStatus->PFmotoRpm)<100)
		{
			MotoFault=0;
			HV_OFF;
			MOTOPOWER_OFF;
			return POWER_OFF;
		}
    else
		{
			MotoFault++;
			if(MotoFault>100)
			{
				MOTOPOWER_OFF;
				HV_OFF;
				MotoFault=0;
				return POWER_OFF;
			}
		}
	
	}
	return POWER_ON;
}

void Init_MotoFan(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
  /*Enable or disable the AHB peripheral clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOB, ENABLE);
	
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* HV  PB7*/
  GPIO_InitStruct.GPIO_Pin = HV_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(HV_PORT, &GPIO_InitStruct);
	HV_OFF;
	/* 15v-Ctrl  PB5*/
  GPIO_InitStruct.GPIO_Pin = MOTOPOWER_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(MOTOPOWER_PORT, &GPIO_InitStruct);
	MOTOPOWER_OFF;
	
  /* MOTO1--PB8,MOTO2-PB9*/
	  /** TIM16 GPIO Configuration  
  PB8   ------> TIM16_CH1
  PB9   ------> TIM17_CH1
  */
  /*Configure GPIO pin : PB */
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStruct.GPIO_Pin = XFMOTO1VSP_PIN;
  GPIO_Init(XFMOTOVSP_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = PFMOTO2VSP_PIN;
  GPIO_Init(PFMOTOVSP_PORT, &GPIO_InitStruct);

  /*Configure GPIO pin alternate function */
  GPIO_PinAFConfig(XFMOTOVSP_PORT, XFMOTO1VSP_PINSOURCE, GPIO_AF_2);
  /*Configure GPIO pin alternate function */
  GPIO_PinAFConfig(PFMOTOVSP_PORT, PFMOTO2VSP_PINSOURCE, GPIO_AF_2);

  /*Configure GPIO pin : PC13,PC14,MOTO Feedback pulse*/
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStruct.GPIO_Pin = XFMOTO1_FB_PIN;
  GPIO_Init(XFMOTO1_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = PFMOTO2_FB_PIN;
  GPIO_Init(PFMOTO2_PORT, &GPIO_InitStruct);
	GPIO_SetBits(XFMOTO1_PORT,XFMOTO1_FB_PIN);
	GPIO_SetBits(PFMOTO2_PORT,PFMOTO2_FB_PIN);

  /*Configure GPIO pin : PC13,PC14,MOTO Feedback pulse*/
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;//GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStruct.GPIO_Pin = XFMOTO1_FB_PIN;
  GPIO_Init(XFMOTO1_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = PFMOTO2_FB_PIN;
  GPIO_Init(PFMOTO2_PORT, &GPIO_InitStruct);

  /* Connect EXTI0 Line to PA0 pin */
  SYSCFG_EXTILineConfig(XFMOTO1_FB_PORTSOURCE, XFMOTO1_FB_EXTISOURCE);
  SYSCFG_EXTILineConfig(PFMOTO2_FB_PORTSOURCE, PFMOTO2_FB_EXTISOURCE);

  /* Configure EXTI15 line */
  EXTI_InitStructure.EXTI_Line = XFMOTO1_FB_EXTI;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_InitStructure.EXTI_Line = PFMOTO2_FB_EXTI;
  EXTI_Init(&EXTI_InitStructure);
  /* Enable and set EXTI0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority  = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	TIM16_17_ModeConfig();
	
	System.Device.MotoFan.XFMoto_DutySet=XFMoto_DutySet;
	System.Device.MotoFan.PFMoto_DutySet=PFMoto_DutySet;
	System.Device.MotoFan.MotoStatusMap=MotoStatusMap;
	System.Device.MotoFan.MotoPowerSet=MotoPowerSet;
}
