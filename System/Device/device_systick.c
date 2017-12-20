/*
********************************************************************************
*�ļ���     : device_systick.c
*����       : ϵͳ����
*ԭ��       : 
********************************************************************************
*�汾     ����            ����            ˵��
*V1.1     tgq       2016/5/12       ��ʼ�汾
********************************************************************************
*/

#include "system.h"


//extern void YTSensorCtrlSystick10Routine(void);
extern void MotoFanSystick10Routine(void);

//extern void IRSystick1000Routine(void);

extern void TimerSystick1000Routine(void);

//extern void Usart3TransSystick1000Routine(void);
extern void Usart3RecvSystick1000Routine(void); 

extern void BeepSystick100Routine(void); 

extern void Usart2TransSystick1000Routine(void); 
extern void Usart2RecvSystick1000Routine(void); 

extern void SysTick_Handler(void);

//#define Systicks 10000     // ÿ���ֲ����жϵĴ�����Ҳ���ǽ�����
#define Systicks 1000     // ÿ���ֲ����жϵĴ�����Ҳ���ǽ�����

#define Systick10000Sum 2
#define Systick1000Sum  2
#define Systick100Sum   5
#define Systick10Sum   5

//static function Systick10000RegisterPointerBlock[Systick10000Sum] =
//{
//    Dummy, Dummy
//};
extern function PMSensorSystick1000Routine;
extern function CO2SensorSystick1000Routine;


static function Systick1000RegisterPointerBlock[Systick1000Sum] =
{
    Dummy, Dummy, 
};

static function Systick100RegisterPointerBlock[Systick100Sum] =
{
    Dummy, Dummy, Dummy, Dummy, Dummy, 
};

static function Systick10RegisterPointerBlock[Systick100Sum] =
{
    Dummy, Dummy, Dummy, Dummy, Dummy, 
};

/*******************************************************************************
* ����	    : ϵͳ����ע�ắ��������ע��ص���������ͬ�ٶȵ�ϵͳ���������С�
* �������  : systemTick ϵͳ�����ٶ����� registerFunction ע��ĺ���ָ��
* ���ز���  : bool���������������Ƿ�ɹ�
*******************************************************************************/
static uint8_t RegisterSystick(SystickEnum type, function registerFunction)
{
    static byte Systick100Counter = 0;
    static byte Systick1000Counter = 0;
//    static byte Systick10000Counter = 0;
    static byte Systick10Counter = 0;

    switch(type)
    {
        case Systick10:
            if (Systick10Counter == Systick10Sum) return(FALSE);
            Systick10RegisterPointerBlock[Systick10Counter++] = registerFunction;  
            return(TRUE);
        case Systick100:
            if (Systick100Counter == Systick100Sum) return(FALSE);
            Systick100RegisterPointerBlock[Systick100Counter++] = registerFunction;  
            return(TRUE);
        case Systick1000:
            if (Systick1000Counter == Systick1000Sum) return(FALSE);
            Systick1000RegisterPointerBlock[Systick1000Counter++] = registerFunction;   
            return(TRUE);
//        case Systick10000:
//            if (Systick10000Counter == Systick10000Sum) return(false);
//            Systick10000RegisterPointerBlock[Systick10000Counter++] = registerFunction;   
//            return(true);
        default:
            return(FALSE);
    }
}

/*******************************************************************************
* ����	    : ϵͳ�����ж���ڣ��ǳ���Ҫ��ÿ��10000�Σ���0.1mSһ��
*           : ϵͳ����ͨ��switchϸ��Ϊÿ��1000�Σ�ÿ��100�ε����н���
*           : ͬ��ֱ�ӵ��ã��߲����ע�����
*******************************************************************************/
void SysTick_Handler(void)
{
    byte div;
    static byte Counter = 0;
    if(++Counter == 100) Counter = 0;
	Systick1000RegisterPointerBlock[0]();
	Systick1000RegisterPointerBlock[1]();
	
	CO2SensorSystick1000Routine();
	
	Usart3RecvSystick1000Routine();
	Usart2TransSystick1000Routine();
	Usart2RecvSystick1000Routine();
	TimerSystick1000Routine();
   div = Counter / 10;
    switch(Counter % 10)
    {
        case 0: 
					Systick100RegisterPointerBlock[0](); 
				break;
        case 1: 
					Systick100RegisterPointerBlock[1](); 
				break;
        case 2: 
					Systick100RegisterPointerBlock[2](); 
				break;
        case 3: 
					Systick100RegisterPointerBlock[3](); 
				break;
        case 4: 
					Systick100RegisterPointerBlock[4]();      
				break;
        case 5: 
					BeepSystick100Routine();        
				break;
        case 6: 
         
				break;
        case 7:         break;
        case 8: 								break;
        case 9:
            switch(div)
            {
                case 0:  Systick10RegisterPointerBlock[0]();      break;
                case 1:  Systick10RegisterPointerBlock[1]();      break;
                case 2:  Systick10RegisterPointerBlock[2]();      break;
                case 3:  Systick10RegisterPointerBlock[3]();      break;
                case 4:  Systick10RegisterPointerBlock[4]();      break;
                case 5:  MotoFanSystick10Routine();      break;
//                case 6:  YTSensorCtrlSystick10Routine();     break;
                default:                              break;
            }
            break;
    }
	if(TimeOut1ms)
		TimeOut1ms--;

}


void InitSystick(void)
{
    SysTick_Config(SystemCoreClock / Systicks);             // ���ý����ٶ�
    System.Device.Systick.Register = RegisterSystick;       // �����ӿں���ָ��
}





