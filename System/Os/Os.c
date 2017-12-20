/*
********************************************************************************
*文件名   : os.c
*创建时间 : 2014.9.9
********************************************************************************
*/





#include "system.h"
#include "os.h"


typedef struct                              // 定义队列类型
{
    uint    *   Start;                      // 指向队列开始
    uint    *   End;                        // 指向队列结束
    uint    *   In;                         // 插入一个消息
    uint    *   Out;                        // 取出一个消息
    ushort      Entries;                    // 消息长度
} QueueStruct;

#define MenuTaskStackSum    400             // 菜单界面任务栈深度
#define QueueBufferSum      40              // 消息队列深度


static uint32_t Time1ms=0;
static QueueStruct MessageQueue;
static uint QueueBuffer[QueueBufferSum];                // 业务逻辑消息队列

static void SystemTickRoutine(void)
{
  if(Time1ms)
		Time1ms--;		
}

/*******************************************************************************
* 描述	    : OS启动
*******************************************************************************/
static void Start(void)
{
	System.Device.Systick.Register(Systick1000, SystemTickRoutine); //注册OS节拍函数
    EnableIrq();
//    while(1);
}
static void DelayMs(int times)
{
   Time1ms=times;
	while(Time1ms);
}

/*******************************************************************************
* 描述	    : 发送一个消息到消息队列中，处于等待的任务会自动运行
*******************************************************************************/
static uint8_t PostMessageQueue(uint message)
{	
    EnterCritical();
    // 菜单界面任务下，且无消息，用任务成员Message传递，不用消息队列

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
* 描述	    : 等待消息队列，当消息队列为空时，所在任务挂起
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

