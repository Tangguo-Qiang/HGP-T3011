/*
********************************************************************************
*文件名     : device_timer.c
*作用       : 软件定时器设备
*原理       : 基于系统节拍1000/S，即1mS一次虚拟出软件定时器
********************************************************************************
*版本     作者            日期            说明
*V0.1    Wangsw        2013/07/21       初始版本
********************************************************************************
*/



#include "system.h"


typedef struct 
{
    int32_t  Times;
    function RegisterFunction;
}TimerStruct;

#define TimerSum 8    //最大不能超过32

static uint32_t State = 0;
static uint32_t Mode;
static TimerStruct TimerBlock[TimerSum];



/*******************************************************************************
* 描述	    : 系统节拍1000/S,即1mS一次调用，实现虚拟定时器。
*******************************************************************************/
void TimerSystick1000Routine(void)
{
    int32_t i = 0;
    uint32_t stateBackup;
	
    
    if(State == 0x00) return;

    stateBackup = State;
    
    while(stateBackup)	
    {
        if((stateBackup & 0x01) == 1) 
        {
            if((--TimerBlock[i].Times) == 0) 
            {
                ResetBit(State, i);
                if(GetBit(Mode, i)) 
                    PostMessage(MessageTimer, (uint32_t)(TimerBlock[i].RegisterFunction));
                else 
                    TimerBlock[i].RegisterFunction(); 
            }
        }
		
        stateBackup = stateBackup >> 1;
        i++;
    }
}

/*******************************************************************************
* 描述	    : 软件定时器
* 输入参数  : id:定时器ID号
*           : handleMode: 定时器模式，
*           :             TimerSystickHandle系统节拍中处理，
*           :             TimerMessageHandle逻辑任务中处理。
*           : times:延时节拍数，以系统节拍为单位
*           : registerFunction: 注册回调执行函数，延时超时后，执行此函数。
* 返回参数  : bool类型
*******************************************************************************/
static void Start(int32_t id, TimerModeEnum mode, int32_t times, function registerFunction)
{
    TimerBlock[id].Times  = times;
    TimerBlock[id].RegisterFunction = registerFunction;
           
    if(mode) 
        SetBit(Mode, id);
    else 
        ResetBit(Mode, id);
    			
    SetBit(State, id); 
}

/*******************************************************************************
* 描述	    : 停止某一路的软件定时器
* 输入参数  : id为0、1、2...
* 返回值    : bool类型，true为关闭并没有回调产生，false输入参数错误或者已经关闭
*******************************************************************************/
static void Stop(int32_t id) 
{
    ResetBit(State, id);
}


void Init_Timer(void)
{
    System.Device.Timer.Start = Start;
    System.Device.Timer.Stop = Stop; 
}
