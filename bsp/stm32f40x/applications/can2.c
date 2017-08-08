#include <board.h>
#include <rtthread.h>
#include "common.h"
void rt_can2_thread_entry(void* parameter)
{
	static char i = 0;
	rt_kprintf("can1 thread start...\n");
	can_init(CAN2,1000000);
	can_filter_init(14);
	can_nvic_config(CAN2,ENABLE);
	while(1)
	{
		if(rt_mp_alloc(&global.mempool,0)){
			i++;
		}
		
		// can_send_test(CAN2,i);
		// i = (i+1)%255;
		rt_kprintf("can2 send [%d]\n", i);
		rt_thread_delay(RT_TICK_PER_SECOND);
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
