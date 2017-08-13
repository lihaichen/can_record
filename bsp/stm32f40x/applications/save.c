#include <board.h>
#include <rtthread.h>
#include <dfs_posix.h>
#include <time.h> 
#include "common.h"
#include <drivers/pin.h>

static long get_file_size(const char *path)  
{  
    long filesize = -1;      
    struct stat stat_buf;  
    if(stat(path, &stat_buf) < 0){  
        return filesize;  
    }else{  
        filesize = stat_buf.st_size;  
    }  
    return filesize;  
}  

static int save(const char *file_name,const void *buf, size_t count)
{
	int fd;
	fd = open(file_name,O_WRONLY | O_CREAT|O_APPEND,0);
	if(fd < 0)
	{
		rt_kprintf("save ==> open file[%s] error\n", file_name);
		return -1;
	}
	rt_pin_write(0,0);
	if(write(fd, buf,count) != count){
		rt_kprintf("save ==> write file[%s] error\n", file_name);
		close(fd);
	}
	return close(fd);
}

static void new_file(char *buf,const char *pre)
{
	time_t timep;  
  struct tm *tm_p; 
	time(&timep);
	tm_p =localtime(&timep);
	sprintf(buf,"%s_%04d%02d%02d-%02d%02d%02d.csv",pre,
					tm_p->tm_year + 1900,(1+tm_p->tm_mon), tm_p->tm_mday,
					tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec);
}

void rt_file_thread_entry(void* parameter)
{	
	// 消息队列
	static msg_t msg;
	static char buf[MEMPOLL_SIZE];
	static char file_name[2][32];
	rt_memset(file_name,0,sizeof(file_name));
	new_file(file_name[0],"/CH1");
	new_file(file_name[1],"/CH2");
	rt_pin_mode(0,0);
	while(1)
	{
		if(rt_mq_recv (global.save_mq, &msg,sizeof(msg_t),  RT_WAITING_FOREVER) != RT_EOK)
		{
			rt_kprintf("save ==> recv mq error\n");
			rt_thread_delay(2);
			continue;
		}
		// rt_kprintf("s>%d-%d-%d\n",msg.type,msg.value,msg.reserve);
		switch(msg.type)
		{
			case CAN1_SAVE:
				rt_memset(buf,0,MEMPOLL_SIZE);
				rt_memcpy(buf,msg.p,msg.value);
				rt_mp_free(msg.p);
				save(file_name[0],buf,msg.value);
				if(get_file_size(file_name[0]) >= FILE_MAX_SIZE)
				{
					new_file(file_name[0],"/CH1");
				}
				rt_pin_write(0,1);
				break;
			case CAN2_SAVE:
				rt_memset(buf,0,MEMPOLL_SIZE);
				rt_memcpy(buf,msg.p,msg.value);
				rt_mp_free(msg.p);
				save(file_name[1],buf,msg.value);
				if(get_file_size(file_name[1]) >= FILE_MAX_SIZE)
				{
					new_file(file_name[1],"/CH2");
				}
				rt_pin_write(0,1);
				break;
			default:
				break;
		}
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
