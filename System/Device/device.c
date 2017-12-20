/*
********************************************************************************
*�ļ���       : device.c
*����         : �豸�ӿ�
*ԭ��         : ��
********************************************************************************
*�汾     ����            ����            ˵��
*V0.1    Wangsw        2013/07/21       ��ʼ�汾
********************************************************************************
*/



#include "system.h"

byte TimeOut10ms=0;

extern void Init_USART3(void);
extern void Init_USART2(void);

extern void Init_PwmCount(void);

extern void Init_StepMoto(void) ;

extern void InitSystick(void);

extern void Init_ADC1(void);

extern void Init_MotoFan(void);

extern void Init_Beep(void);

extern void Uart1_Init(void);

extern void Init_Timer(void);

extern void Init_Iwdog(void);

/*******************************************************************************
* ����	    : ��ʼ���豸
*******************************************************************************/
static void RTC_SetFlag(byte reg)
{
   	  /*������ʱ��鱸�ݼĴ���BKP_DR1��������ݲ���0xA5A5*/
//	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to RTC */
  PWR_BackupAccessCmd(ENABLE);
	
	switch(reg)
	{
		case 2:
			 RTC_WriteBackupRegister(RTC_BKP_DR1, 0xb5b5);
			break;
		case 3:
			 RTC_WriteBackupRegister(RTC_BKP_DR2, 0xc5c5);
			break;
		default:
			break;
	}

}

void InitDevice(void)
{    

    Init_USART2();	
	
		Init_ADC1();	

    Init_PwmCount();

    Init_StepMoto();
	
	  Init_Timer();
	
  	Init_Beep();
	
	  Uart1_Init();
	
		Init_MotoFan();
	
	  Init_USART3();
	
//		Init_Iwdog();
	
    InitSystick();
		System.Device.Rtc.RTC_SetFlag=RTC_SetFlag;
//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     // ��ռ���ȼ�|����ռ���ȼ�,0Ϊ��
                                                        // bit3 bit2 | bit1 bit0 
}


