#include "app.h"
#include "comm.h"

#define FRAME_HEAD	0
#define FRAME_MAINORDER	1
#define FRAME_AIDORDER	2
#define FRAME_DATABEGIN	3
#define FRAME_DATALENGTH	4
#define FRAME_END	7

static uint8_t CommRxBuffer[COMMRXDATASIZE]={0};
static uint8_t CommTxBuffer[COMMTXDATASIZE]={0};

const byte ResetOrder[COMMRXDATASIZE]={0x58,0x28,0x52,0x45,0x53,0x45,0x54,0x29};
//static uint8_t CommSync=0;


void DataReady_Temper(void)
{
	int8_t* pTemper;
	
	CommTxBuffer[FRAME_HEAD]=0x46;
	CommTxBuffer[FRAME_MAINORDER]=CODE_TEMPER_RH;
	CommTxBuffer[FRAME_AIDORDER]= 0x00;
	Fill_Buffer(&CommTxBuffer[FRAME_DATABEGIN], (uint16_t) FRAME_DATALENGTH);

	
	pTemper = (int8_t*)&CommTxBuffer[FRAME_DATABEGIN];
	*pTemper = App.SensorData.TempIn;
	*(pTemper+1) = App.SensorData.TempOut;
	*(pTemper+2) = App.SensorData.RHIn;
	CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);
	
}

void DataReady_Moto1(void)
{
	uint8_t* pTemper;
	
	CommTxBuffer[FRAME_HEAD]=0x46;
	CommTxBuffer[FRAME_MAINORDER]=CODE_XFMOTODUTY;
//	Fill_Buffer(&CommTxBuffer[FRAME_AIDORDER], (uint16_t) FRAME_DATALENGTH);

	App.SysCtrlPara.XFmotoDuty = CommRxBuffer[FRAME_DATABEGIN];
	App.SysCtrlPara.XFmotoDuty <<= 8;
	App.SysCtrlPara.XFmotoDuty |= CommRxBuffer[FRAME_DATABEGIN+1];
	App.SysCtrlPara.XFmotoRpm = CommRxBuffer[FRAME_DATABEGIN+2];
	App.SysCtrlPara.XFmotoRpm <<= 8;
	App.SysCtrlPara.XFmotoRpm |= CommRxBuffer[FRAME_DATABEGIN+3];
	
	CommTxBuffer[FRAME_AIDORDER]=App.SysStatus.XFmotoFault;
	pTemper = &CommTxBuffer[FRAME_DATABEGIN];
	*pTemper++ = (App.SysStatus.XFmotoDuty>>8);
	*pTemper++ = (App.SysStatus.XFmotoDuty&0xff);
	*pTemper++ = App.SysStatus.XFmotoRpm>>8;
	*pTemper = App.SysStatus.XFmotoRpm&0xff;
	
	CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);
}

void DataReady_Moto2(void)
{
	uint8_t* pTemper;
	
	CommTxBuffer[FRAME_HEAD]=0x46;
	CommTxBuffer[FRAME_MAINORDER]=CODE_PFMOTODUTY;
//	Fill_Buffer(&CommTxBuffer[FRAME_AIDORDER], (uint16_t) FRAME_DATALENGTH);

	App.SysCtrlPara.PFmotoDuty = CommRxBuffer[FRAME_DATABEGIN];
	App.SysCtrlPara.PFmotoDuty <<= 8;
	App.SysCtrlPara.PFmotoDuty |= CommRxBuffer[FRAME_DATABEGIN+1];
	App.SysCtrlPara.PFmotoRpm = CommRxBuffer[FRAME_DATABEGIN+2];
	App.SysCtrlPara.PFmotoRpm <<= 8;
	App.SysCtrlPara.PFmotoRpm |= CommRxBuffer[FRAME_DATABEGIN+3];
	
	CommTxBuffer[FRAME_AIDORDER]=App.SysStatus.PFmotoFault;
	pTemper = &CommTxBuffer[FRAME_DATABEGIN];
	*pTemper++ = (App.SysStatus.PFmotoDuty>>8);
	*pTemper++ = (App.SysStatus.PFmotoDuty&0xff);
	*pTemper++ = App.SysStatus.PFmotoRpm>>8;
	*pTemper = App.SysStatus.PFmotoRpm&0xff;
	
	CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);
}

void DataReady_CircleSet(void)
{
	
//	Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
	CommTxBuffer[FRAME_HEAD]=0x46;
	CommTxBuffer[FRAME_MAINORDER]=CODE_CIRCLEMODE;
	CommTxBuffer[FRAME_AIDORDER]=App.SysStatus.BypassStatus.BypassMode;
	
	App.SysCtrlPara.BypassMode.BypassMode = (BypassModeTypedef)CommRxBuffer[FRAME_AIDORDER];
	App.SysCtrlPara.BypassMode.BypassAngle = CommRxBuffer[FRAME_DATABEGIN];
	App.SysCtrlPara.BypassMode.BypassAngle <<= 8;
	App.SysCtrlPara.BypassMode.BypassAngle |= CommRxBuffer[FRAME_DATABEGIN+1];
	CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);
	
}


#ifdef WINSEN_HCHO
void DataReady_HCHO(void)
{
	uint16_t val;
	
	val=System.Device.Usart1.HCHOppb_Get();
	
//	Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
	CommTxBuffer[FRAME_HEAD]=0x46;
	CommTxBuffer[FRAME_MAINORDER]=CODE_HCHO_READ;
	CommTxBuffer[FRAME_DATABEGIN]= (uint8_t)(val&0xff);
	CommTxBuffer[FRAME_DATABEGIN+1]= (uint8_t)(val>>8);
	
	CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);
	
}
#else
void DataReady_PM(void)
{
	uint16_t val;
	
	val=System.Device.Usart1.PM25ug_Get();
	
//	Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
	CommTxBuffer[FRAME_HEAD]=0x46;
	CommTxBuffer[FRAME_MAINORDER]=CODE_IAQ_READ;
	CommTxBuffer[FRAME_DATABEGIN]= (uint8_t)(val&0xff);
	CommTxBuffer[FRAME_DATABEGIN+1]= (uint8_t)(val>>8);
	
	CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);
	
}
#endif

void DataReady_CO2(void)
{
	uint16_t val;
	
	val=System.Device.Pwm.CO2ppm_Get();
	
//	Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
	CommTxBuffer[FRAME_HEAD]=0x46;
	CommTxBuffer[FRAME_MAINORDER]=CODE_CO2_READ;
		CommTxBuffer[FRAME_DATABEGIN]= (uint8_t)(val&0xff);
		CommTxBuffer[FRAME_DATABEGIN+1]= (uint8_t)(val>>8);
	
	CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);
	
}



void CommTalk_Echo(byte length)
{
	uint8_t sum=0;
	uint8_t order;
	TestStatus flag;
	
	if(length<COMMRXDATASIZE)
	{
		for(sum=0;sum<length;sum++)
		   System.Device.Usart2.ReadByte(&CommRxBuffer[sum]);
		flag = FAILED;
	}
	else
	{
		while(length)
	  {
			do
			{
				flag = System.Device.Usart2.ReadByte(&CommRxBuffer[0]);
				length--;
				if(CommRxBuffer[0]==0x58)
					break;
			}while(length);
			if(length>6) //>=7
			{
				for(sum=1;sum<COMMRXDATASIZE;sum++)
				{
					flag =System.Device.Usart2.ReadByte(&CommRxBuffer[sum]);
					length--;
				}

				if(flag==PASSED)
				{
					sum=Data_Sum(CommRxBuffer,COMMRXDATASIZE-1);

					if(sum == CommRxBuffer[FRAME_END])
					{
						Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
						order = CommRxBuffer[FRAME_MAINORDER];
						switch(order)
						{
							case CODE_TEMPER_RH:
								DataReady_Temper();
								break;
							case CODE_XFMOTODUTY:
								DataReady_Moto1();
								if(App.SysStatus.Power != POWER_ON)
								{
									App.SysCtrlPara.Power = POWER_ON;
									PostMessage(MessageParaUpdate, CODE_POWER_SET);
								}
							if(App.SysCtrlPara.XFmotoDuty != App.SysStatus.XFmotoDuty )
								PostMessage(MessageParaUpdate, CODE_XFMOTODUTY);					
								break;
							case CODE_PFMOTODUTY:
								DataReady_Moto2();
							if(App.SysCtrlPara.PFmotoDuty != App.SysStatus.PFmotoDuty )
								PostMessage(MessageParaUpdate, CODE_PFMOTODUTY);					
								break;
							case CODE_IAQ_READ:
#ifndef WINSEN_HCHO
								DataReady_PM();
#endif							
								break;
							case CODE_HCHO_READ:
#ifdef WINSEN_HCHO
								DataReady_HCHO();
#endif
  							break;
							case CODE_CO2_READ:
								DataReady_CO2();
								break;
							case CODE_CIRCLEMODE:
								DataReady_CircleSet();
								PostMessage(MessageParaUpdate, CODE_CIRCLEMODE);					
								break;
							case CODE_HEATER_SET:
//								Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
								CommTxBuffer[FRAME_HEAD]=0x46;
								CommTxBuffer[FRAME_MAINORDER]=CODE_HEATER_SET;
								CommTxBuffer[FRAME_AIDORDER]=App.SysStatus.AuxiliaryHeat;//App.FuncState.SwitchState;
//								CommTxBuffer[FRAME_DATABEGIN]=App.SysStatus.AirConditioner;//App.FuncState.SwitchState;
								
								App.SysCtrlPara.AuxiliaryHeatSet = (AuxiliaryHeatTypedef)CommRxBuffer[FRAME_AIDORDER];
								CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);							
								PostMessage(MessageParaUpdate, CODE_HEATER_SET);					
								break;
//							case CODE_COOLER_SET:
////								Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
//								CommTxBuffer[FRAME_HEAD]=0x46;
//								CommTxBuffer[FRAME_MAINORDER]=CODE_COOLER_SET;
//								CommTxBuffer[FRAME_AIDORDER]=App.SysStatus.AirConditioner;//App.FuncState.SwitchState;
//								CommTxBuffer[FRAME_DATABEGIN]=App.SysStatus.AuxiliaryHeat;//App.FuncState.SwitchState;
//								
//								App.SysCtrlPara.AirConditionerSet = (AirConditionerTypedef)CommRxBuffer[FRAME_AIDORDER];
//								CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);							
//								PostMessage(MessageParaUpdate, CODE_COOLER_SET);					
//								break;
							case CODE_BYPASS2_SET:
//								Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
								CommTxBuffer[FRAME_HEAD]=0x46;
								CommTxBuffer[FRAME_MAINORDER]=CODE_BYPASS2_SET;
								CommTxBuffer[FRAME_AIDORDER]=App.FuncState.BypassMode2.BypassMode;
								
								App.FuncPara.BypassMode2.BypassMode = (BypassModeTypedef)CommRxBuffer[FRAME_AIDORDER];
								App.FuncPara.BypassMode2.BypassAngle = CommRxBuffer[FRAME_DATABEGIN];
								App.FuncPara.BypassMode2.BypassAngle <<= 8;
								App.FuncPara.BypassMode2.BypassAngle |= CommRxBuffer[FRAME_DATABEGIN+1];
								CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);							
								PostMessage(MessageParaUpdate, CODE_BYPASS2_SET);					
								break;
							case CODE_BYPASS3_SET:
//								Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
								CommTxBuffer[FRAME_HEAD]=0x46;
								CommTxBuffer[FRAME_MAINORDER]=CODE_BYPASS3_SET;
								CommTxBuffer[FRAME_AIDORDER]=App.FuncState.BypassMode3.BypassMode;
								
								App.FuncPara.BypassMode3.BypassMode = (BypassModeTypedef)CommRxBuffer[FRAME_AIDORDER];
								App.FuncPara.BypassMode3.BypassAngle = CommRxBuffer[FRAME_DATABEGIN];
								App.FuncPara.BypassMode3.BypassAngle <<= 8;
								App.FuncPara.BypassMode3.BypassAngle |= CommRxBuffer[FRAME_DATABEGIN+1];
								CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);							
								PostMessage(MessageParaUpdate, CODE_BYPASS3_SET);					
								break;
							case CODE_BYPASS4_SET:
//								Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);	
								CommTxBuffer[FRAME_HEAD]=0x46;
								CommTxBuffer[FRAME_MAINORDER]=CODE_BYPASS4_SET;
								CommTxBuffer[FRAME_AIDORDER]=App.FuncState.BypassMode4.BypassMode;
								
								App.FuncPara.BypassMode4.BypassMode = (BypassModeTypedef)CommRxBuffer[FRAME_AIDORDER];
								App.FuncPara.BypassMode4.BypassAngle = CommRxBuffer[FRAME_DATABEGIN];
								App.FuncPara.BypassMode4.BypassAngle <<= 8;
								App.FuncPara.BypassMode4.BypassAngle |= CommRxBuffer[FRAME_DATABEGIN+1];
								CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);							
								PostMessage(MessageParaUpdate, CODE_BYPASS4_SET);					
								break;
							case CODE_BEEP_SETON:
								if(CommRxBuffer[FRAME_AIDORDER]<2)
									System.Device.Beep.BeepOnShort(1);
								else if(CommRxBuffer[FRAME_AIDORDER]<0xFF)
									System.Device.Beep.BeepOnShort(CommRxBuffer[FRAME_AIDORDER]);
								else
									System.Device.Beep.BeepOnLong();
									
								break;
							case CODE_POWER_SET:
//								Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);
								App.SysCtrlPara.Power =(PowerSetTypedef)CommRxBuffer[FRAME_AIDORDER];
								CommTxBuffer[FRAME_HEAD]=0x46;
								CommTxBuffer[FRAME_MAINORDER]=CODE_POWER_SET;
								CommTxBuffer[FRAME_AIDORDER]= App.SysStatus.Power;
								CommTxBuffer[FRAME_DATABEGIN]= MAIN_VERSION;
								CommTxBuffer[FRAME_DATABEGIN+1]= SUB_VERSION;
								CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);
//							if(App.SysCtrlPara.Power != App.SysStatus.Power)
							{
//								CommSync=0;
								PostMessage(MessageParaUpdate, CODE_POWER_SET);
							}
//							else
//							{
//								CommSync++;
//								if(CommSync>20)
//								{
//									if(App.SysStatus.Power==POWER_OFF)
//										NVIC_SystemReset();
//									CommSync = 20;
//								}
//							}
				
								break;
							case CODE_VERSION:
								Fill_Buffer((uint8_t*)&CommTxBuffer[FRAME_HEAD], (uint16_t)COMMTXDATASIZE);
								CommTxBuffer[FRAME_HEAD]=0x46;
								CommTxBuffer[FRAME_MAINORDER]=CODE_VERSION;
								CommTxBuffer[FRAME_AIDORDER]= App.SysStatus.Power;
								CommTxBuffer[FRAME_DATABEGIN]= MAIN_VERSION;
								CommTxBuffer[FRAME_DATABEGIN+1]= SUB_VERSION;
							  CommTxBuffer[FRAME_END]= Data_Sum(CommTxBuffer,COMMTXDATASIZE-1);
				
								break;
						}
						if(CommTxBuffer[FRAME_HEAD])
						{
							for(sum=0;sum<COMMTXDATASIZE;sum++)
							{
								flag =System.Device.Usart2.WriteByte(CommTxBuffer[sum]);
								if(flag == FAILED)
									return;
							}
						}
					}
					else	if((App.SysStatus.Power==POWER_OFF)&&(CommRxBuffer[FRAME_END]==0x29))
					{
							for(sum=0;sum<8;sum++)
							{
								if(CommRxBuffer[sum] != ResetOrder[sum])
									break;
							}
							if(sum==8)
							{
								System.Device.Rtc.RTC_SetFlag(2);
								NVIC_SystemReset();
							}
					}

				}
			}
			else //length<7
			{
				do
				{
					 flag=System.Device.Usart2.ReadByte(&CommRxBuffer[0]);
				}while(flag==PASSED);
				length=0;
				flag = FAILED;
			}	
		}//while(length);
	}
		

	
}
