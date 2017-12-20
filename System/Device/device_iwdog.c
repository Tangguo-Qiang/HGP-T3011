#include "system.h"

#define LSI_TIM_MEASURE

static  __IO uint32_t LsiFreq = 40000;
static  __IO uint16_t CaptureNumber;
static  __IO uint16_t IC1ReadValue1 = 0, IC1ReadValue2 = 0;
static  __IO uint16_t CaptureNumber = 0;
static  __IO uint32_t Capture = 0;

#ifdef LSI_TIM_MEASURE
/**
  * @brief  Configures TIM14 to measure the LSI oscillator frequency.
  * @param  None
  * @retval None
  */
static void TIM14_ConfigForLSI(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  
  /* Enable peripheral clocks ------------------------------------------------*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to the RTC */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset RTC Domain */
  RCC_BackupResetCmd(ENABLE);
  RCC_BackupResetCmd(DISABLE);
  
  /*!< LSI Enable */
  RCC_LSICmd(ENABLE);
  
  /*!< Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {}
  
  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
   
  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();
  
  /* Enable TIM14 clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);  
  
  /* Enable the TIM14 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Configure TIM14 prescaler */
  TIM_PrescalerConfig(TIM14, 0, TIM_PSCReloadMode_Immediate);
  
  /* Connect internally the TM14_CH1 Input Capture to the LSI clock output */
  TIM_RemapConfig(TIM14, TIM14_RTC_CLK);
  
  /* TIM14 configuration: Input Capture mode ---------------------
     The LSI oscillator is connected to TIM14 CH1
     The Rising edge is used as active edge,
     The TIM14 CCR1 is used to compute the frequency value 
  ------------------------------------------------------------ */
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV8;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(TIM14, &TIM_ICInitStructure);
  
  /* TIM14 Counter Enable */
  TIM_Cmd(TIM14, ENABLE);

  /* Reset the flags */
  TIM14->SR = 0;
    
  /* Enable the CC1 Interrupt Request */  
  TIM_ITConfig(TIM14, TIM_IT_CC1, ENABLE);  
}
#endif /* LSI_TIM_MEASURE */

#ifdef LSI_TIM_MEASURE
/**
  * @brief  This function handles TIM14 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM14_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM14, TIM_IT_CC1) != RESET)
  {    
    if(CaptureNumber == 0)
    {
      /* Get the Input Capture value */
      IC1ReadValue1 = TIM_GetCapture1(TIM14);
    }
    else if(CaptureNumber == 1)
    {
      /* Get the Input Capture value */
      IC1ReadValue2 = TIM_GetCapture1(TIM14); 
      
      /* Capture computation */
      if (IC1ReadValue2 > IC1ReadValue1)
      {
        Capture = (IC1ReadValue2 - IC1ReadValue1); 
      }
      else
      {
        Capture = ((0xFFFF - IC1ReadValue1) + IC1ReadValue2); 
      }
      /* Frequency computation */ 
      LsiFreq = (uint32_t) SystemCoreClock / Capture;
      LsiFreq *= 8;
    }
    
    CaptureNumber++;
    
    /* Clear TIM14 Capture compare interrupt pending bit */
    TIM_ClearITPendingBit(TIM14, TIM_IT_CC1);
  }
}
#endif /* LSI_TIM_MEASURE */

static void IwdogStart(uint delay1ms)
{
	uint val=32000;
	
  /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_32);

  /* For Example:
	 Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4) 
                          = LsiFreq/128
	So, delay1ms IWDG TimeOut: LsiFreq/(32000/delay1ms)	
   */
	val /= delay1ms;
  IWDG_SetReload(LsiFreq/val);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

static void IwdogReload(void)
{
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
}

void Init_Iwdog(void)
{
#ifdef LSI_TIM_MEASURE
  /* TIM Configuration -------------------------------------------------------*/
  TIM14_ConfigForLSI();
EnableIrq()  
  /* Wait until the TIM14 get 2 LSI edges */
  while(CaptureNumber != 2)
  {
  }

  /* Disable TIM14 CC1 Interrupt Request */
  TIM_ITConfig(TIM14, TIM_IT_CC1, DISABLE);
#endif /* LSI_TIM_MEASURE */
  
DisableIrq();	
	
	System.Device.Iwdog.IwdogStart =IwdogStart;
	System.Device.Iwdog.IwdogReload = IwdogReload ;
	
}

