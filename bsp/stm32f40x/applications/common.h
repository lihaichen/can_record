#ifndef __COMMON_H__
#define __COMMON_H__
#include <rthw.h>
#include <rtthread.h>
#include <time.h>
#include "stm32f4xx_can.h"

// 不够64字节，补0,为扇区的整数倍
#define FRAME_SIZE				32

// 串口帧大小
#define UART_FRAME_SIZE  (FRAME_SIZE + 4)

#define	CAN_BUF_MAX_SIZE	(8*1024)

// 内存池的块大小
#define	MEMPOLL_SIZE			(CAN_BUF_MAX_SIZE + (FRAME_SIZE * 2))
#define MEMPOLL_COUNT			4
#define	MQ_LEN						128

// 文件最大大小
#define	FILE_MAX_SIZE			20*1024*1024

// can默认速率
#define	CAN_DEFAULT_BPS		(1000*1000)

// can 空闲时间进行存储
#define IDLE_SAVE					RT_TICK_PER_SECOND/2

// 不够64字节是否填充
#define CAN_FILL					1
#define FILTER_ID_SIZE 		16
#define FILTER_ID_FILE		"/can_id"

// 是否使用时间采样统计
#define USE_TIMESTAMPE		1

// 是否串口上报
#define SERIAL_UPLOAD			1

// 扩展帧
#define EXTENDED_FRAME		(1<<1)
// 标准帧
#define STANDARD_FRASME		(0<<1)
// 远程帧
#define REMOTE_FRAME			(1<<0)
// 数据帧
#define DATE_FRAME				(0<<0)
// 魔法数
#define MAGIC_DTAA				(0xEE8855AA)

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
#if USE_TIMESTAMPE
	int timestamp;
#endif
	unsigned int reserve;
}msg_t;

// 保存msg结构体
typedef struct 
{
	// 魔法值
	unsigned int magic; 
	// 数据长度
	unsigned short len;
	// 帧类型
	unsigned short type;
	// 时间秒数
	unsigned long long date;
	// can id
	unsigned int id;
	// us 
	unsigned int us;
	// 数据信息
	unsigned char data[8];
}save_msg_t;

typedef struct 
{
	unsigned int SRF;
	unsigned int SDF;
	unsigned int ERF;
	unsigned int EDF;
}frame_info_t;

#if USE_TIMESTAMPE
typedef struct 
{
	 int start;
	 int max;
	 int min;
	 int avg;
	 // 数量
	 unsigned int sum;
}timestamp_t;
#endif

// 全局接口
typedef struct 
{
	status_type_t status;
	// 系统加电时间
	time_t power_time;
	// 系统运行时间
	time_t run_time;
	struct rt_mempool mempool1;
	struct rt_mempool *mempool2;
	struct rt_mempool *mempool3;
	rt_mq_t can1_mq;
	rt_mq_t can2_mq;
	rt_mq_t save_mq;
	rt_mq_t uart_mq;
	// 帧信息统计
	frame_info_t frame_info[2];
#if USE_TIMESTAMPE	
	// 时间戳统计
	// can1 can2 save1 save2 saveIdle
	timestamp_t timestamp[5];
#endif	
	// 文件长度
	unsigned int file_len[2];
	// 关注id的长度
	int id_len ;
	// 关注id的值
	int filter_id[FILTER_ID_SIZE];
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
extern char * send_save_msg(msg_type_t type,void *buf,int len,int timestamp);

// 将can数据解析存储buf，格式为csv
extern int frame_to_csv(msg_type_t type, CanRxMsg *can_msg, char* buf);
// 将can数据解析存储buf，格式为bin
extern int frame_to_bin(msg_type_t type, CanRxMsg *can_msg, char* buf);
extern void can_init(CAN_TypeDef* CANx, unsigned int bps);
extern void can_filter_init(unsigned int num, FunctionalState NewState);
// 存储线程初始化
extern int rt_file_init(void);
extern void can_send_test(CAN_TypeDef* CANx,unsigned int data);
// 看门狗线程
extern int rt_wd_init(void);
// 导出线程
extern int rt_export_init(void);
// 读取过滤的id
extern int read_filter_id(void);
// 上报线程
extern int rt_upload_init(void);
// 时间戳计算
#if USE_TIMESTAMPE
extern void calc_timestampe(timestamp_t *t);
#endif
#endif
