/*
********************************************************************************
*�ļ���     : app.c
*����       : Ӧ�ó������
*ԭ��       : Ӧ�ò���ڣ�Ҳ��ϵͳ��ڣ���Ҫ��ʼ����������������
********************************************************************************
*�汾     ����            ����            ˵��
*V1.0    tgq        2016/05/12       ��ʼ�汾
********************************************************************************
*/


#include "system.h"
#include "app.h"

AppStruct App;

/*******************************************************************************
* ����	    : ��ʼ�����ݿ�
*******************************************************************************/
static void InitData(void)
{
	App.SensorData.TempIn= 0;
	App.SensorData.TempOut =0;
	App.SensorData.RHIn=0;
	App.SensorData.CO2ppm=0;
	App.SensorData.PMug=0;
	
	App.SysCtrlPara.Power = POWER_OFF;
//	App.SysCtrlPara.Power = POWER_ON;
	App.SysCtrlPara.BypassMode.BypassMode = BYPASS_EMPTY;
	App.SysCtrlPara.BypassMode.BypassAngle = 180;
	App.SysCtrlPara.XFmotoDuty =0;
	App.SysCtrlPara.PFmotoDuty =0;
	App.SysCtrlPara.XFmotoRpm=0;
	App.SysCtrlPara.PFmotoRpm=0;
	
	App.SysStatus.Power=POWER_OFF;
	App.SysStatus.BypassStatus.BypassMode =BYPASS_EMPTY;
	App.SysStatus.BypassStatus.BypassAngle = 180;
	App.SysStatus.XFmotoDuty = 0;
	App.SysStatus.PFmotoDuty = 0;
	App.SysStatus.XFmotoRpm=0;
	App.SysStatus.PFmotoRpm=0;
	App.SysStatus.XFmotoFault=1;
	App.SysStatus.PFmotoFault=1;
	
	App.FuncPara.SwitchState =0;
	App.FuncPara.BypassMode2.BypassMode= BYPASS_IN;
	App.FuncPara.BypassMode2.BypassAngle = 180;
	App.FuncPara.BypassMode3.BypassMode= BYPASS_IN;
	App.FuncPara.BypassMode3.BypassAngle = 180;
	App.FuncPara.BypassMode4.BypassMode= BYPASS_IN;
	App.FuncPara.BypassMode4.BypassAngle = 180;
}

void Fill_Buffer(uint8_t *pBuffer, uint16_t BufferLength)
{
  
  /* Put in global buffer same values */
  while(BufferLength>0 )
  {
    *pBuffer++ = 0x00;
		BufferLength--;
  }
}

uint8_t Data_Sum(uint8_t* pSrc,uint8_t len)
{
	uint8_t i,sum=0;
	
	for(i=0;i<len; i++)
	{
		sum +=*pSrc++;
	}
	return sum;
}


/*******************************************************************************
* ����	    : C�������Main����
*******************************************************************************/
int main(void) 
{      
  System.Init();                          //��ʼ��ϵͳ��
  InitData();
    
  System.OS.Start();
	
	System.Device.Beep.BeepOnShort(1);	
	LogicTask();
}

