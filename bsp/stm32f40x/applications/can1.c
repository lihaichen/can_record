#include <board.h>
#include <rtthread.h>
#include "common.h"
#include <drivers/pin.h>

#define CAN_FILTER_CHANNEL	0

static void rt_can1_thread_entry(void* parameter)
{
	// 消息队列
	static msg_t msg;
	// can数据指针
	static CanRxMsg *can_msg;
	// 存储够SD卡页大小后再发送进行写入
	static char * buf = RT_NULL;
	// buf 数据长度
	static int len = 0;
	// 存储的次数
	static int save_index = 0;
	
	rt_kprintf("can1 thread start...\n");
	can_init(CAN1,CAN_DEFAULT_BPS);
	can_filter_init(CAN_FILTER_CHANNEL, ENABLE);
	buf = (char *)rt_mp_alloc(&global.mempool1,RT_WAITING_FOREVER);
	rt_memset(buf,0,MEMPOLL_SIZE);
	while(1)
	{		
		if(rt_mq_recv (global.can1_mq, &msg,sizeof(msg_t),  IDLE_SAVE) != RT_EOK)
		{
			if(len > 0)
			{
				buf = send_save_msg(CAN1_SAVE,buf,len,save_index);
				len = 0;
				save_index ++;
			}
			continue;
		}
	
		switch(msg.type)
		{
			case CAN1_STOP:
				can_filter_init(CAN_FILTER_CHANNEL, DISABLE);
				break;
			case CAN1_START:
				can_filter_init(CAN_FILTER_CHANNEL,ENABLE);
				break;
			case CAN1_RECV:
				rt_pin_write(2,0);
				can_msg = msg.p;
				frame_to_csv(msg.type,can_msg,buf+len);
				len += FRAME_SIZE;
				if(len >= CAN_BUF_MAX_SIZE){
					// 进行存储
					buf = send_save_msg(CAN1_SAVE,buf,len,save_index);
					len = 0;
					save_index ++;
				}
				rt_pin_write(2,1);
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
