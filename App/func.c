#include "app.h"

#define FRAME_HEAD	0
#define FRAME_MAINORDER	1
#define FRAME_AIDORDER	2
#define FRAME_DATABEGIN	3
#define FRAME_DATALENGTH	4
#define FRAME_END	7

uint8_t FuncRxBuffer[FUNCRXDATASIZE]={0};
uint8_t FuncTxBuffer[FUNCTXDATASIZE]={0};






/**
* @brief  Compares two buffers.
* @param  pBuffer1, pBuffer2: buffers to be compared.
* @param  BufferLength: buffer's length
* @retval PASSED: pBuffer1 identical to pBuffer2
*         FAILED: pBuffer1 differs from pBuffer2
*/
TestStatus IfBufferEmpty(uint8_t* pBuffer, uint16_t BufferLength)
{
  while (BufferLength>0)
  {
    if (*pBuffer != 0x00)
    {
      return PASSED;
    }
    pBuffer++;
		
		BufferLength--;
  }
  
  return FAILED;
}



	
bool FuncTalk_Echo(void)
{
	bool flag=FALSE;
	byte i=0;
	
	System.Device.Usart3.ReadString(&FuncRxBuffer[0]);
	i =Data_Sum(FuncRxBuffer,FUNCRXDATASIZE-1);
	if(i == FuncRxBuffer[FRAME_END])
	{
		switch(FuncRxBuffer[FRAME_MAINORDER])
		{
			case FUNC_SWITCH_SET:
				App.FuncState.SwitchState = FuncRxBuffer[FRAME_DATABEGIN];
				i=App.FuncState.SwitchState&(SWITCH_HEAT1|SWITCH_HEAT2|SWITCH_AC_RS1);
			  switch(i)
				{
					case (SWITCH_HEAT1):
						App.SysStatus.AuxiliaryHeat= HEATER_HALF;
						break;
					case (SWITCH_HEAT1|SWITCH_HEAT2):
						App.SysStatus.AuxiliaryHeat= HEATER_FULL;
						break;
					case (SWITCH_AC_RS1):
						App.SysStatus.AuxiliaryHeat= HEATER_COOLON;
						break;
					case (0):
						App.SysStatus.AuxiliaryHeat= HEATER_OFF;
						break;
					default:
						
						break;
				}
		  		break;
			case FUNC_BYPASS2_SET:
				App.FuncState.BypassMode2.BypassMode  = (BypassModeTypedef)FuncRxBuffer[FRAME_AIDORDER];
				break;
			case FUNC_BYPASS3_SET:
				App.FuncState.BypassMode3.BypassMode  = (BypassModeTypedef)FuncRxBuffer[FRAME_AIDORDER];
				break;
		  case FUNC_BYPASS4_SET:
				App.FuncState.BypassMode4.BypassMode  = (BypassModeTypedef)FuncRxBuffer[FRAME_AIDORDER];
				break;
		  case FUNC_ACK:
//				if(FuncRxBuffer[FRAME_AIDORDER])
				break;

			}
	} //end of if(i == FuncRxBuffer[FRAME_END])
	return flag;
	
}


bool FuncTalk_Trans(byte order)
{
	bool flag;
//	ushort rpm;
	
	Fill_Buffer((uint8_t*)&FuncTxBuffer[FRAME_HEAD], (uint16_t)FUNCTXDATASIZE);	
	FuncTxBuffer[FRAME_HEAD]=0x46;
//	FuncTxBuffer[FRAME_END]= Data_Sum((uint8_t*)FuncTxBuffer,FUNCTXDATASIZE-1);
	switch(order)
	{
		case FUNC_ACK:
			FuncTxBuffer[FRAME_MAINORDER]=FUNC_ACK;
			break;
		case FUNC_SWITCH_SET:
			FuncTxBuffer[FRAME_MAINORDER]=FUNC_SWITCH_SET;
			FuncTxBuffer[FRAME_AIDORDER]= App.FuncPara.SwitchState;
			break;
		case FUNC_BYPASS2_SET:
			FuncTxBuffer[FRAME_MAINORDER]=CODE_BYPASS2_SET;
			FuncTxBuffer[FRAME_AIDORDER]=App.FuncPara.BypassMode2.BypassMode ;
			FuncTxBuffer[FRAME_DATABEGIN]=((ushort)(App.FuncPara.BypassMode2.BypassAngle)>>8);
			FuncTxBuffer[FRAME_DATABEGIN+1]=App.FuncPara.BypassMode2.BypassAngle&0xFF;
			break;
		case FUNC_BYPASS3_SET:
			FuncTxBuffer[FRAME_MAINORDER]=CODE_BYPASS3_SET;
			FuncTxBuffer[FRAME_AIDORDER]=App.FuncPara.BypassMode3.BypassMode ;
			FuncTxBuffer[FRAME_DATABEGIN]=((ushort)(App.FuncPara.BypassMode3.BypassAngle)>>8);
			FuncTxBuffer[FRAME_DATABEGIN+1]=App.FuncPara.BypassMode3.BypassAngle&0xFF;
			break;
		case FUNC_BYPASS4_SET:
			FuncTxBuffer[FRAME_MAINORDER]=CODE_BYPASS4_SET;
			FuncTxBuffer[FRAME_AIDORDER]=App.FuncPara.BypassMode4.BypassMode ;
			FuncTxBuffer[FRAME_DATABEGIN]=((ushort)(App.FuncPara.BypassMode4.BypassAngle)>>8);
			FuncTxBuffer[FRAME_DATABEGIN+1]=App.FuncPara.BypassMode4.BypassAngle&0xFF;
			break;
//		case VERSIONINFO:
//			FuncTxBuffer[FRAME_MAINORDER]=VERSIONINFO;
//			FuncTxBuffer[FRAME_AIDORDER]=0;
//			FuncTxBuffer[FRAME_DATABEGIN]=MAIN_VERSION;
//			FuncTxBuffer[FRAME_DATABEGIN+1]=SUB_VERSION;
//			break;
	}
	FuncTxBuffer[FRAME_END]= Data_Sum((uint8_t*)FuncTxBuffer,FUNCTXDATASIZE-1);
	
	flag =System.Device.Usart3.CommProc(&FuncTxBuffer[0]);
	
	return flag;
}

