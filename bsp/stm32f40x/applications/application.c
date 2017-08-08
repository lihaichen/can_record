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
#include "sdcard.h"

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_init.h>
#include <dfs_elm.h>
#endif

#ifdef RT_USING_RTC
#include "stm32f4_rtc.h"
#endif

#include "common.h"

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
		global.status = INIT;
	  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);//使能写入PR和RLR
		IWDG_SetPrescaler(IWDG_Prescaler_256);  //写入PR预分频值
		IWDG_SetReload(0xFFF);  //写入RLR
		IWDG_Enable();//KR写入0xCCCC
		rt_thread_idle_sethook(rt_thread_idle_hook);
		
#ifdef RT_USING_RTC
		rt_hw_rtc_init();
#endif		
    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
		rt_hw_sdcard_init();
	/* initialize the device file system */
		dfs_init();
	/* initialize the elm chan FatFS file system*/
		elm_init();
    /* mount sd card fat partition 1 as root directory */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
    {
        rt_kprintf("File System initialzation failed!\n");
    }
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */
		mempool_init();
		rt_can1_init();
		rt_can2_init();
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
