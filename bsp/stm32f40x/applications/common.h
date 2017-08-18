#ifndef __COMMON_H__
#define __COMMON_H__
#include <rthw.h>
#include <rtthread.h>
#include "stm32f4xx_can.h"

// 不够64字节，补0,为扇区的整数倍
#define FRAME_SIZE				64

#define	CAN_BUF_MAX_SIZE	21*1024

// 内存池的块大小
#define	MEMPOLL_SIZE			(CAN_BUF_MAX_SIZE + (FRAME_SIZE * 2))
#define MEMPOLL_COUNT			3
#define	MQ_LEN						32

// 文件最大大小
#define	FILE_MAX_SIZE			100*1024*1024

// can默认速率
#define	CAN_DEFAULT_BPS		1000000

// can 空闲时间进行存储
#define IDLE_SAVE					RT_TICK_PER_SECOND/2

// 运行状态机
typedef enum 
{
	INIT,
	STOP,
	RUN,
	RTC_ERROR,
	SD_ERROR,
	INIT_ERROR
}status_type_t;

typedef enum
{
	CAN1_STOP,
	CAN1_START,
	CAN2_STOP,
	CAN2_START,
	CAN1_RECV,
	CAN2_RECV,
	CAN1_SAVE,
	CAN2_SAVE,
	KEY
}msg_type_t;

typedef struct 
{
	msg_type_t type;
	unsigned int value;
	void *p;
	unsigned int reserve;
}msg_t;

typedef struct 
{
	unsigned int SRF;
	unsigned int SDF;
	unsigned int ERF;
	unsigned int EDF;
}frame_info_t;

// 全局接口
typedef struct 
{
	status_type_t status;
	struct rt_mempool mempool1;
	struct rt_mempool *mempool2;
	rt_mq_t can1_mq;
	rt_mq_t can2_mq;
	rt_mq_t save_mq;
	frame_info_t frame_info[2];
}global_t;


extern global_t global;

extern int rt_can1_init(void);
extern int rt_can2_init(void);
// 初始化内存池
extern rt_err_t mempool_init(void);
// 初始化消息队列
extern rt_err_t messagequeue_init(void);
//字节流转换为十六进制字符串
extern void hex_2_str(const char *src,  char *dest, int len );

// 发送一个存储信息，返回一个新的内存块
extern char * send_save_msg(msg_type_t type,void *buf,int len,int save_index);

// 将can数据解析存储buf，格式为csv
extern int frame_to_csv(msg_type_t type, CanRxMsg *can_msg, char* buf);

extern void can_init(CAN_TypeDef* CANx, unsigned int bps);
extern void can_filter_init(unsigned int num, FunctionalState NewState);
// 存储线程初始化
extern int rt_file_init(void);
extern void can_send_test(CAN_TypeDef* CANx,unsigned int data);
// 看门狗线程
extern int rt_wd_init(void);
#endif
