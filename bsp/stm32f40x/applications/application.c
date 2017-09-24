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
#include "i2c.h"
#include "ds1340.h"
#include "us.h"

void rt_init_thread_entry(void* parameter)
{
		extern void lwip_sys_init(void);
		global.status = INIT;
		i2c_init();
#ifdef RT_USING_RTC
		DS1340Init("rtc","i2c");
		// rt_hw_rtc_init();
#endif		
		rt_wd_init();
		us_timer_init();
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
			global.power_time = time(RT_NULL);
			global.run_time = 0;
			rt_kprintf("rtc=%ld-%ld\n",global.power_time, global.run_time);
			mempool_init();
			messagequeue_init();
			rt_can1_init();
			rt_can2_init();
			rt_file_init();
			rt_upload_init();
			global.status = RUN;
    }
    else
    {
      rt_kprintf("File System initialzation failed!\n");
			global.status = SD_ERROR;
    }
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */
#if defined(RT_USING_LWIP)		
		lwip_sys_init();
		eth_system_device_init();
		rt_hw_stm32_eth_init();
		rt_export_init();
#endif			
		read_filter_id();
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
