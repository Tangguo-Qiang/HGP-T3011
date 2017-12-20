/*
********************************************************************************
*�ļ���     : device_timer.c
*����       : �����ʱ���豸
*ԭ��       : ����ϵͳ����1000/S����1mSһ������������ʱ��
********************************************************************************
*�汾     ����            ����            ˵��
*V0.1    Wangsw        2013/07/21       ��ʼ�汾
********************************************************************************
*/



#include "system.h"


typedef struct 
{
    int32_t  Times;
    function RegisterFunction;
}TimerStruct;

#define TimerSum 8    //����ܳ���32

static uint32_t State = 0;
static uint32_t Mode;
static TimerStruct TimerBlock[TimerSum];



/*******************************************************************************
* ����	    : ϵͳ����1000/S,��1mSһ�ε��ã�ʵ�����ⶨʱ����
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
* ����	    : �����ʱ��
* �������  : id:��ʱ��ID��
*           : handleMode: ��ʱ��ģʽ��
*           :             TimerSystickHandleϵͳ�����д���
*           :             TimerMessageHandle�߼������д���
*           : times:��ʱ����������ϵͳ����Ϊ��λ
*           : registerFunction: ע��ص�ִ�к�������ʱ��ʱ��ִ�д˺�����
* ���ز���  : bool����
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
* ����	    : ֹͣĳһ·�������ʱ��
* �������  : idΪ0��1��2...
* ����ֵ    : bool���ͣ�trueΪ�رղ�û�лص�������false���������������Ѿ��ر�
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
