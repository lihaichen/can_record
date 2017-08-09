#include <board.h>
#include <rtthread.h>
#include "common.h"

#define CAN_FILTER_CHANNEL	0

void rt_can1_thread_entry(void* parameter)
{
	static msg_t msg;
	static CanRxMsg *can_msg;
	rt_kprintf("can1 thread start...\n");
	can_init(CAN1,1000000);
	can_filter_init(CAN_FILTER_CHANNEL, ENABLE);
	while(1)
	{		
		if(rt_mq_recv (global.can1_mq, &msg,sizeof(msg_t),  RT_WAITING_FOREVER) != RT_EOK)
		{
			rt_kprintf("can1 ==> recv mq error\n");
			rt_thread_delay(RT_TICK_PER_SECOND * 10/1000);
			continue;
		}
		rt_kprintf("can1 ==> recv msg[%d]\n",msg.type);
		switch(msg.type)
		{
			case CAN1_STOP:
				can_filter_init(CAN_FILTER_CHANNEL, DISABLE);
				break;
			case CAN1_START:
				can_filter_init(CAN_FILTER_CHANNEL,ENABLE);
				break;
			case CAN1_RECV:
				can_msg = msg.p;
				rt_kprintf("id[0x%x] data[0x%x]\r\n", can_msg->StdId, can_msg->Data[0]);
				break;
			default:
				break;
		}
		
	}
}
int rt_can1_init()
{
	 rt_thread_t tid;

    tid = rt_thread_create("can1",
        rt_can1_thread_entry, RT_NULL,
        2048, 5, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
