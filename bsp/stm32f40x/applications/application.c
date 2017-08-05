/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2014-04-27     Bernard      make code cleanup. 
 */

#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include "stm32f4xx_eth.h"
#endif

#ifdef RT_USING_GDB
#include <gdb_stub.h>
#endif

/*
*使用idle线程进行喂狗
*2s喂一次
*/
static void rt_thread_idle_hook(void)
{
	static rt_tick_t time = 0;
	if((rt_tick_get() - time) > 2*RT_TICK_PER_SECOND)
	{
		IWDG_ReloadCounter();
		time = rt_tick_get();
	}
}

void rt_init_thread_entry(void* parameter)
{
    /* GDB STUB */
#ifdef RT_USING_GDB
    gdb_set_device("uart6");
    gdb_start();
#endif

    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
        extern void lwip_sys_init(void);

        /* register ethernetif device */
        eth_system_device_init();

        rt_hw_stm32_eth_init();

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");
    }
#endif
		
		rt_kprintf("RCC_FLAG_BORRST:%d\r\n",RCC_GetFlagStatus(RCC_FLAG_BORRST));
		rt_kprintf("RCC_FLAG_PINRST:%d\r\n",RCC_GetFlagStatus(RCC_FLAG_PINRST));
		rt_kprintf("RCC_FLAG_PORRST:%d\r\n",RCC_GetFlagStatus(RCC_FLAG_PORRST));
		rt_kprintf("RCC_FLAG_SFTRST:%d\r\n",RCC_GetFlagStatus(RCC_FLAG_SFTRST));
		rt_kprintf("RCC_FLAG_IWDGRST:%d\r\n",RCC_GetFlagStatus(RCC_FLAG_IWDGRST));
		rt_kprintf("RCC_FLAG_WWDGRST:%d\r\n",RCC_GetFlagStatus(RCC_FLAG_WWDGRST));
		rt_kprintf("RCC_FLAG_LPWRRST:%d\r\n",RCC_GetFlagStatus(RCC_FLAG_LPWRRST));
		RCC_ClearFlag();
	
	  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//使能写入PR和RLR
		IWDG_SetPrescaler(IWDG_Prescaler_256);  //写入PR预分频值
		IWDG_SetReload(0xFFF);  //写入RLR
		IWDG_Enable();//KR写入0xCCCC
		rt_thread_idle_sethook(rt_thread_idle_hook);
}

int rt_application_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

/*@}*/
