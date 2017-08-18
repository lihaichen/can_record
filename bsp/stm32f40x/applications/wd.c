#include <board.h>
#include <rtthread.h>
#include "common.h"
#include <drivers/pin.h>

static void rt_wd_thread_entry(void* parameter)
{
	// 初始化状态LED
	rt_pin_mode(0,0);
	rt_pin_mode(1,0);
	rt_pin_mode(2,0);
	rt_pin_write(0,1);
	rt_pin_write(1,1);
	rt_pin_write(2,1);	
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//使能写入PR和RLR
	IWDG_SetPrescaler(IWDG_Prescaler_256);  //写入PR预分频值
	IWDG_SetReload(0xFFF);  //写入RLR
	IWDG_Enable();//KR写入0xCCCC
	while(1)
	{
		switch(global.status)
		{
			case RUN:
				rt_pin_write(1,0);
				rt_thread_delay(RT_TICK_PER_SECOND/2);
				rt_pin_write(1,1);
				rt_thread_delay(RT_TICK_PER_SECOND/2);
			break;
			default:
				rt_pin_write(1,0);
				rt_thread_delay(RT_TICK_PER_SECOND);
				break;
		}
		IWDG_ReloadCounter();
	}
}

int rt_wd_init()
{
	 rt_thread_t tid;

    tid = rt_thread_create("wd",
        rt_wd_thread_entry, RT_NULL,
        1024, RT_THREAD_PRIORITY_MAX-1, 10);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
