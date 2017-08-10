#include <board.h>
#include <rtthread.h>
#include "common.h"
#include <dfs_posix.h>

static int save(const char *file_name,const void *buf, size_t count)
{
	int fd;
	fd = open(file_name,O_WRONLY | O_CREAT|O_APPEND,0);
	if(fd < 0)
	{
		rt_kprintf("save ==> open file[%s] error\n", file_name);
		return -1;
	}
	if(write(fd, buf,count) != count){
		rt_kprintf("save ==> write file[%s] error\n", file_name);
		close(fd);
	}
	return close(fd);
}

void rt_file_thread_entry(void* parameter)
{	
	// 消息队列
	static msg_t msg;
	static char buf[MEMPOLL_SIZE];
	while(1)
	{
		if(rt_mq_recv (global.save_mq, &msg,sizeof(msg_t),  RT_WAITING_FOREVER) != RT_EOK)
		{
			rt_kprintf("save ==> recv mq error\n");
			rt_thread_delay(RT_TICK_PER_SECOND * 10/1000);
			continue;
		}
		rt_kprintf("save ==> recv msg[%d]\n",msg.type);
		switch(msg.type)
		{
			case CAN1_SAVE:
				rt_memset(buf,0,MEMPOLL_SIZE);
				rt_memcpy(buf,msg.p,msg.value);
				rt_mp_free(msg.p);
				save("/test.csv",buf,msg.value);
				break;
			case CAN2_SAVE:
				break;
			default:
				break;
		}
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
}
int rt_file_init()
{ 
	rt_thread_t tid;
	tid = rt_thread_create("file",
        rt_file_thread_entry, RT_NULL,
        2048, 10, 20);  
	if (tid != RT_NULL)
		rt_thread_startup(tid);
	return 0;
}
