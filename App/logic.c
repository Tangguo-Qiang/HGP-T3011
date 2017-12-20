/*
********************************************************************************
*文件名     : logic.c
*作用       : 业务逻辑处理文件
*原理       : 处理各类消息，如按键、传感器等，标准化一些按键的处理模式
********************************************************************************
*版本     作者            日期            说明
*V1.0    tgq        2016/05/12       初始版本
********************************************************************************
*/


#include "system.h"
#include "app.h"

typedef enum{
	BypassReady=(byte)0,
	BypassPreStart=(byte)1,
	BypassPreFinish=(byte)2,
	BypassMoving=(byte)3,
}BypassStatus;

#define PWMFORBYPASS    ((uint16_t)8000)
#define RPMFORBYPASS    ((uint16_t)500)

static BypassStatus PrepBypass=BypassReady;
//static uint16_t XFMoteBuf=0;
//static uint16_t PFMoteBuf=0;

static void SystemPowerOn(void)
{
	
	System.Device.Usart1.PMSensor_Start();
	System.Device.Pwm.CO2Sensor_Start();
	
	App.SysStatus.Power= System.Device.MotoFan.MotoPowerSet(POWER_ON);
//	App.SysCtrlPara.XFmotoDuty = MOTO_STARTDUTY;
//	App.SysCtrlPara.PFmotoDuty = MOTO_STARTDUTY;
	System.Device.MotoFan.XFMoto_DutySet(App.SysCtrlPara.XFmotoDuty,App.SysCtrlPara.XFmotoRpm);
	System.Device.MotoFan.PFMoto_DutySet(App.SysCtrlPara.PFmotoDuty,App.SysCtrlPara.PFmotoRpm);
	App.SysStatus.Power= System.Device.MotoFan.MotoPowerSet(POWER_ON);
#ifdef __Cubic_PM25
	System.Device.Usart1.PMSensor_Req(READ_DATA);
#endif
}

static void SystemPowerOff(void)
{
	System.Device.Pwm.CO2Sensor_Stop();
	System.Device.Usart1.PMSensor_Stop();
	App.SysCtrlPara.XFmotoDuty =0;
	System.Device.MotoFan.XFMoto_DutySet(App.SysCtrlPara.XFmotoDuty,0);
	App.SysCtrlPara.PFmotoDuty =0;
	System.Device.MotoFan.PFMoto_DutySet(App.SysCtrlPara.PFmotoDuty,0);
	App.SysCtrlPara.AuxiliaryHeatSet =HEATER_OFF;
	App.FuncPara.SwitchState=0;
	FuncTalk_Trans(FUNC_SWITCH_SET);
	App.SysStatus.Power= System.Device.MotoFan.MotoPowerSet(POWER_OFF);
}

static void CtrlParaUpdate(byte data)
{
	switch(data)
	{
		case CODE_XFMOTODUTY:
			
			if(PrepBypass)
			{
				if(App.SysStatus.XFmotoRpm>(RPMFORBYPASS))
					System.Device.MotoFan.XFMoto_DutySet(PWMFORBYPASS,(RPMFORBYPASS));
				if((RPMFORBYPASS+30)>App.SysStatus.XFmotoRpm)
				{
					PrepBypass = BypassPreFinish;
					PostMessage(MessageParaUpdate, CODE_CIRCLEMODE);
				}
				else
					PrepBypass = BypassPreStart;
					
			}
			else //if(App.SysCtrlPara.XFmotoDuty != App.SysStatus.XFmotoDuty )
			{
					System.Device.MotoFan.XFMoto_DutySet(App.SysCtrlPara.XFmotoDuty,App.SysCtrlPara.XFmotoRpm);
			}
				
			break;
		case CODE_PFMOTODUTY:			
			if(PrepBypass)
			{
				if(App.SysStatus.PFmotoRpm >RPMFORBYPASS)
					System.Device.MotoFan.PFMoto_DutySet(PWMFORBYPASS,(RPMFORBYPASS));
			}
			else //if(App.SysCtrlPara.PFmotoDuty != App.SysStatus.PFmotoDuty )
			{
					System.Device.MotoFan.PFMoto_DutySet(App.SysCtrlPara.PFmotoDuty,App.SysCtrlPara.PFmotoRpm);
			}
			break;
		case CODE_CIRCLEMODE:
//						||(App.SysCtrlPara.BypassMode.BypassAngle != App.SysStatus.BypassStatus.BypassAngle))
			if(App.SysCtrlPara.BypassMode.BypassMode != App.SysStatus.BypassStatus.BypassMode)
			{
				switch(PrepBypass)
				{
					case BypassReady:
						PrepBypass =BypassPreStart;
							PostMessage(MessageParaUpdate, CODE_XFMOTODUTY);
							PostMessage(MessageParaUpdate, CODE_PFMOTODUTY);
					break;
					case BypassPreStart:
							PostMessage(MessageParaUpdate, CODE_XFMOTODUTY);
							PostMessage(MessageParaUpdate, CODE_PFMOTODUTY);
						break;
					case BypassPreFinish:
						App.SysStatus.BypassStatus.BypassAngle= App.SysCtrlPara.BypassMode.BypassAngle;
						if(FALSE==System.Device.StepMoto.STPM_BypassMove(App.SysCtrlPara.BypassMode.BypassMode))
							PostMessage(MessageParaUpdate, CODE_CIRCLEMODE);
						else
							PrepBypass = BypassMoving;
						break;
					case BypassMoving:
						break;
					default:
						break;
				}
			}
			else if(PrepBypass) // Process for Bypass is not finish yet
				PrepBypass = BypassMoving;
			break;
		case CODE_HEATER_SET:
			
		  switch(App.SysCtrlPara.AuxiliaryHeatSet)
			{
				case HEATER_OFF:
//					App.FuncPara.SwitchState &= ~(SWITCH_HEAT1|SWITCH_HEAT2|SWITCH_AC_RS1);
						App.FuncPara.SwitchState = 0;
					break;
				case HEATER_HALF:
					App.FuncPara.SwitchState &= ~(SWITCH_HEAT1|SWITCH_HEAT2|SWITCH_AC_RS1);
					App.FuncPara.SwitchState |= SWITCH_HEAT1;
					break;
				case HEATER_FULL:
					App.FuncPara.SwitchState &= ~SWITCH_AC_RS1;
					App.FuncPara.SwitchState |= (SWITCH_HEAT1|SWITCH_HEAT2);
					break;
				case HEATER_COOLON:
					App.FuncPara.SwitchState &= ~(SWITCH_HEAT1|SWITCH_HEAT2);
					App.FuncPara.SwitchState |=SWITCH_AC_RS1;
					break;
				default:					
					App.FuncPara.SwitchState &= ~(SWITCH_HEAT1|SWITCH_HEAT2|SWITCH_AC_RS1);
						App.FuncPara.SwitchState = 0;
					break;
			}
			FuncTalk_Trans(FUNC_SWITCH_SET);
			break;
//		case CODE_COOLER_SET:
//			
//		  switch(App.SysCtrlPara.AirConditionerSet)
//			{
//				case COOLER_ON:
//					App.FuncPara.SwitchState |= SWITCH_AC_RS1;
//					App.FuncPara.SwitchState &= ~(SWITCH_HEAT1|SWITCH_HEAT2);
//					break;
//				case COOLER_OFF:
//					App.FuncPara.SwitchState &= ~SWITCH_AC_RS1;
//					break;
//			}
//			FuncTalk_Trans(FUNC_SWITCH_SET);
//			break;
		case CODE_BYPASS2_SET:
			FuncTalk_Trans(FUNC_BYPASS2_SET);
			break;
		case CODE_BYPASS3_SET:
			FuncTalk_Trans(FUNC_BYPASS3_SET);
			break;
		case CODE_BYPASS4_SET:
			FuncTalk_Trans(FUNC_BYPASS4_SET);
			break;
		case CODE_POWER_SET:
			if(App.SysCtrlPara.Power ==POWER_ON)
				SystemPowerOn();
			else
			{
				SystemPowerOff();
//				PrepBypass =0;
			}
			break;
	}
}

static void AppSystick10(void)
{
	static byte i=0;
	static byte second=0;
	
	if(++i>9)
	{
		i=0;
		second++;
	
		if(App.SysCtrlPara.Power ==POWER_OFF)
		{
			second = 0;
			if(PrepBypass)
			{
				if(PrepBypass<BypassPreFinish)
				{
					if(RPMFORBYPASS>=App.SysStatus.XFmotoRpm)
					{
						PrepBypass = BypassPreFinish;
						PostMessage(MessageParaUpdate, CODE_CIRCLEMODE);
					}
				}
				else if(PrepBypass >BypassPreFinish)
				{
					if(App.SysCtrlPara.BypassMode.BypassMode == App.SysStatus.BypassStatus.BypassMode)
						PrepBypass =BypassReady;
					else
					{
						PrepBypass = BypassPreFinish;
						PostMessage(MessageParaUpdate, CODE_CIRCLEMODE);
					}
				}
			}
			if(App.SysStatus.Power !=POWER_OFF)
				App.SysStatus.Power= System.Device.MotoFan.MotoPowerSet(POWER_OFF);
			return;
		}		
		switch(second)
		{
			case 1:
				PostMessage(MessageUpdataDate, CODE_TEMPER_RH);
				break;
			case 2:
				if(PrepBypass)
				{
					if(PrepBypass<BypassPreFinish) //BypassStart
					{
						if((RPMFORBYPASS+50)>App.SysStatus.XFmotoRpm)
						{
							PrepBypass = BypassPreFinish;
							PostMessage(MessageParaUpdate, CODE_CIRCLEMODE);
						}
						else
						{
							PostMessage(MessageParaUpdate, CODE_XFMOTODUTY);
							PostMessage(MessageParaUpdate, CODE_PFMOTODUTY);
							
						}
							
					}
					else if(PrepBypass >BypassPreFinish) //BypassMoving
					{
						if(App.SysCtrlPara.BypassMode.BypassMode == App.SysStatus.BypassStatus.BypassMode)
						{
							PrepBypass =BypassReady;
							PostMessage(MessageParaUpdate, CODE_XFMOTODUTY);
							PostMessage(MessageParaUpdate, CODE_PFMOTODUTY);
						}
						else
						{
							PrepBypass=BypassPreFinish;
							PostMessage(MessageParaUpdate, CODE_CIRCLEMODE);
						}
					}
				}
				break;
			case 3:
				PostMessage(MessageUpdataDate, CODE_IAQ_READ);
				break;
			case 4:
				break;
			case 5:
				second =0;
				break;
			default:
				break;
			
		}
		
		if(App.FuncPara.SwitchState)
			PostMessage(MessageFuncTrans, FUNC_ACK);
	}
	
}

static void InitLogic(void)
{
	System.Device.MotoFan.MotoStatusMap(&App.SysStatus );
	System.Device.StepMoto.BypassStatusMap(&(App.SysStatus.BypassStatus));
  System.Device.Systick.Register(Systick10, AppSystick10);
}

void LogicTask(void)
{
    uint message;
    uint data; 
    InitLogic();

//    PostMessage(MessageParaUpdate, CODE_CIRCLEMODE);
    PostMessage(MessageParaUpdate, CODE_POWER_SET);

//  System.Device.Iwdog.IwdogStart(1000);
	//逻辑业务任务获取消息，分配处理
    while(1)//(TRUE)
    {     
        message = System.OS.PendMessageQueue();
        
        data = GetMessageData(message);
        switch(GetMessageType(message))
        {
            case MessageCommRecv:
                CommTalk_Echo(data);
                break;
            case MessageParaUpdate:
                CtrlParaUpdate(data);
                break;
            case MessageFuncRecv:
                FuncTalk_Echo();
                break;
            case MessageFuncTrans:
                FuncTalk_Trans(data);
                break;
            case MessageUpdataDate:
							switch(data)
							{
								case CODE_TEMPER_RH:
									System.Device.Adc1.ADC1_DataGet((int8_t*)&App.SensorData.TempIn);
								break;
								case CODE_IAQ_READ:
#ifdef __Cubic_PM25
									System.Device.Usart1.PMSensor_Req(READ_DATA);
#endif
									break;
								default:
									break;
							}
                break;
            default:
                break;
        }
//				System.Device.Iwdog.IwdogReload();
				
				System.OS.DelayMs(10);
    }
}
