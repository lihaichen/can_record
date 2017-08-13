#include <board.h>
#include <rtthread.h>
#include "common.h"

#define CAN_FILTER_CHANNEL	14

void rt_can2_thread_entry(void* parameter)
{
	rt_kprintf("can1 thread start...\n");
	can_init(CAN2,CAN_DEFAULT_BPS);
	can_filter_init(CAN_FILTER_CHANNEL,ENABLE);
	while(1)
	{
	}
}
int rt_can2_init()
{
	 rt_thread_t tid;

    tid = rt_thread_create("can2",
        rt_can2_thread_entry, RT_NULL,
        2048, 5, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
