/*
********************************************************************************
*�ļ���   : os.c
*����ʱ�� : 2014.9.9
********************************************************************************
*/





#include "system.h"
#include "os.h"


typedef struct                              // �����������
{
    uint    *   Start;                      // ָ����п�ʼ
    uint    *   End;                        // ָ����н���
    uint    *   In;                         // ����һ����Ϣ
    uint    *   Out;                        // ȡ��һ����Ϣ
    ushort      Entries;                    // ��Ϣ����
} QueueStruct;

#define MenuTaskStackSum    400             // �˵���������ջ���
#define QueueBufferSum      40              // ��Ϣ�������


static uint32_t Time1ms=0;
static QueueStruct MessageQueue;
static uint QueueBuffer[QueueBufferSum];                // ҵ���߼���Ϣ����

static void SystemTickRoutine(void)
{
  if(Time1ms)
		Time1ms--;		
}

/*******************************************************************************
* ����	    : OS����
*******************************************************************************/
static void Start(void)
{
	System.Device.Systick.Register(Systick1000, SystemTickRoutine); //ע��OS���ĺ���
    EnableIrq();
//    while(1);
}
static void DelayMs(int times)
{
   Time1ms=times;
	while(Time1ms);
}

/*******************************************************************************
* ����	    : ����һ����Ϣ����Ϣ�����У����ڵȴ���������Զ�����
*******************************************************************************/
static uint8_t PostMessageQueue(uint message)
{	
    EnterCritical();
    // �˵����������£�������Ϣ���������ԱMessage���ݣ�������Ϣ����

    if (MessageQueue.Entries >= QueueBufferSum)
    {                                                              
        ExitCritical();
        return (FALSE);
    }

    MessageQueue.Entries++;
    
    *MessageQueue.In++ = message;
    if (MessageQueue.In > MessageQueue.End)
       	MessageQueue.In = MessageQueue.Start;

    ExitCritical();

    return (TRUE);
}

/*******************************************************************************
* ����	    : �ȴ���Ϣ���У�����Ϣ����Ϊ��ʱ�������������
*******************************************************************************/
static uint PendMessageQueue(void)
{
    uint message=0;
		
			EnterCritical();
    if (MessageQueue.Entries > 0)
    {                    
       	message = *MessageQueue.Out++;
       	MessageQueue.Entries--;
       	if (MessageQueue.Out > MessageQueue.End) 
            MessageQueue.Out = MessageQueue.Start;
		}   
       	ExitCritical();
       	return (message);
    
}



void InitOs(void)
{    
    MessageQueue.Start        = QueueBuffer;
    MessageQueue.End          = QueueBuffer + QueueBufferSum - 1;
    MessageQueue.In           = MessageQueue.Start;
    MessageQueue.Out          = MessageQueue.Start;
    MessageQueue.Entries      = 0;	
    
    
    System.OS.PendMessageQueue = PendMessageQueue;
    System.OS.PostMessageQueue = PostMessageQueue;
    System.OS.Start = Start;
    System.OS.DelayMs = DelayMs;
}

