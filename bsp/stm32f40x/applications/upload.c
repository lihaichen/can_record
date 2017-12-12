#include <board.h>
#include <rtthread.h>
#include "common.h"

static void rt_upload_thread_entry(void* parameter)
{
	char *p = RT_NULL;
	rt_device_t device;
	int err;
	device = rt_device_find("uart2");
	if (device == RT_NULL)
	{
		rt_kprintf("not find uart2\n");
		return;
	}
	err = rt_device_open(device,RT_DEVICE_OFLAG_RDWR);
	if(err != RT_EOK)
	{
		rt_kprintf("open uart2 error\n");
		return;
	}
	
	while(1)
	{
		if(rt_mq_recv (global.uart_mq, &p,sizeof(char *),  RT_WAITING_FOREVER) != RT_EOK)
		{
			rt_kprintf("uart ==> recv mq error\n");
			rt_thread_delay(2);
			continue;
		}
		rt_device_write(device, 0, p, FRAME_SIZE);
		rt_mp_free(p);
	}
}

int rt_upload_init()
{
	 rt_thread_t tid;

    tid = rt_thread_create("upload",
        rt_upload_thread_entry, RT_NULL,
        1024, 10, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
