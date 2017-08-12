#include <board.h>
#include <rtthread.h>
#include <time.h> 
#include <stdio.h>
#include "common.h"

#define CAN_FILTER_CHANNEL	0



void rt_can1_thread_entry(void* parameter)
{
	// 消息队列
	static msg_t msg;
	// can数据指针
	static CanRxMsg *can_msg;
	// 存储够SD卡页大小后再发送进行写入
	static char * buf = RT_NULL;
	// buf 数据长度
	static int len = 0;
	// can 数据hextostr
	static char tmp[8*3 + 1];
	// 帧类型
	static char *frame_type_list[4] = {"SRF","SDF","ERF","EDF"};
	static char *frame_type = RT_NULL;
	// 每次存储帧数量
	static int save_frame_sum = 0;
	// 存储的次数
	static int save_sum = 0;
	// 时间
	time_t timep;  
  struct tm *tm_p; 
	
	rt_kprintf("can1 thread start...\n");
	can_init(CAN1,1000000);
	can_filter_init(CAN_FILTER_CHANNEL, ENABLE);
	buf = (char *)rt_mp_alloc(&global.mempool,RT_WAITING_FOREVER);
	rt_memset(buf,0,MEMPOLL_SIZE);
	while(1)
	{		
		if(rt_mq_recv (global.can1_mq, &msg,sizeof(msg_t),  RT_WAITING_FOREVER) != RT_EOK)
		{
			rt_kprintf("can1 ==> recv mq error\n");
			rt_thread_delay(RT_TICK_PER_SECOND * 10/1000);
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
				can_msg = msg.p;
				rt_memset(tmp,0,sizeof(tmp));
				if(!can_msg->RTR){
					hex_2_str((const char *)can_msg->Data,tmp,can_msg->DLC);
				} 
				if(can_msg->IDE)
				{
					// 扩展帧
					if(can_msg->RTR)
					{
						// 远程帧
						global.frame_info[0].ERF ++;
						frame_type = frame_type_list[2];
					}else
					{
						// 数据帧
						global.frame_info[0].EDF ++;
						frame_type = frame_type_list[3];
					}
				}else
				{
					// 标准帧
						// 扩展帧
					if(can_msg->RTR)
					{
						// 远程帧
						global.frame_info[0].SRF ++;
						frame_type = frame_type_list[0];
					}else
					{
						// 数据帧
						global.frame_info[0].SDF ++;
						frame_type = frame_type_list[1];
					}
				}
				time(&timep);
				tm_p =localtime(&timep);
				sprintf(&buf[len],"%04d-%02d-%02d %02d:%02d:%02d,%s,0x%X,%s\n",
					tm_p->tm_year + 1900,(1+tm_p->tm_mon), tm_p->tm_mday,
					tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec,
					frame_type,can_msg->StdId + can_msg->ExtId,tmp);
				len = rt_strlen(buf);
				save_frame_sum ++;
				if(len > CAN_BUF_MAX_SIZE){
					// 进行存储
					msg_t send_msg;
					rt_memset(&send_msg,0,sizeof(msg_t));
					send_msg.type = CAN1_SAVE;
					send_msg.value = len;
					send_msg.p = buf;
					send_msg.reserve = save_sum;
					rt_mq_send(global.save_mq, &send_msg, sizeof(msg_t));
					buf = (char *)rt_mp_alloc(&global.mempool,RT_WAITING_FOREVER);
					rt_memset(buf,0,MEMPOLL_SIZE);
					rt_kprintf("==>%d-%d-%d\n",save_frame_sum,len,save_sum);
					len = 0;
					save_frame_sum = 0;
					save_sum ++;
				}
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
