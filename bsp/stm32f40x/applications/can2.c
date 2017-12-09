#include <board.h>
#include <rtthread.h>
#include "common.h"
#include "us.h"
#include <drivers/pin.h>

#define CAN_FILTER_CHANNEL	14

static void rt_can2_thread_entry(void* parameter)
{
	// 消息队列
	static msg_t msg;
	// can数据指针
	static CanRxMsg *can_msg;
	// 存储够SD卡页大小后再发送进行写入
	static char * buf = RT_NULL;
	// buf 数据长度
	static int len = 0, i;
	
	rt_kprintf("can2 thread start...\n");
	can_init(CAN2,CAN_DEFAULT_BPS);
	can_filter_init(CAN_FILTER_CHANNEL, ENABLE);
	buf = (char *)rt_mp_alloc(global.mempool2,RT_WAITING_FOREVER);
	rt_memset(buf,0,MEMPOLL_SIZE);
	while(1)
	{		
		if(rt_mq_recv(global.can2_mq, &msg,sizeof(msg_t),  IDLE_SAVE) != RT_EOK)
		{
			if(len > 0)
			{
				buf = send_save_msg(CAN2_SAVE,buf,len,get_us_timer());
				len = 0;
			}
			continue;
		}
	
		switch(msg.type)
		{
			case CAN2_STOP:
				can_filter_init(CAN_FILTER_CHANNEL, DISABLE);
				break;
			case CAN2_START:
				can_filter_init(CAN_FILTER_CHANNEL,ENABLE);
				break;
			case CAN2_RECV:
				rt_pin_write(3,0);
				can_msg = msg.p;
				frame_to_bin(msg.type,can_msg, buf+len);
				len += FRAME_SIZE;
#if SERIAL_UPLOAD			
				for(i = 0; i < global.id_len; i++)
				{
					if(global.filter_id[i] == (can_msg->StdId + can_msg->ExtId))
					{
						char *p = (char *)rt_mp_alloc(global.mempool3,RT_WAITING_NO);
						if(p != RT_NULL)
						{
							rt_memset(p,0,UART_FRAME_SIZE);
							rt_memcpy(p,buf+len,UART_FRAME_SIZE);
							rt_mq_send(global.uart_mq, (void *)&p, sizeof(char *));
						}
						break;
					}
				}
#endif				
				if(len >= CAN_BUF_MAX_SIZE)
				{
					// 进行存储
					buf = send_save_msg(CAN2_SAVE,buf,len,get_us_timer());
					len = 0;
				}	
				rt_pin_write(3,1);
#if USE_TIMESTAMPE		
				global.timestamp[1].start = msg.timestamp;
				calc_timestampe(&global.timestamp[1]);
#endif
				break;
			default:
				break;
		}
	}
}

int rt_can2_init()
{
	 rt_thread_t tid;

    tid = rt_thread_create("can2",
        rt_can2_thread_entry, RT_NULL,
        1024, 5, 50);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
