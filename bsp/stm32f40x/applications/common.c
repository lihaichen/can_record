#include "common.h"
#include <stdio.h>
#include <dfs_posix.h>
#include <time.h> 
#include "us.h"
global_t global;

const char* str_00_99[] = {
	"00","01","02","03","04","05","06","07","08","09",
	"10","11","12","13","14","15","16","17","18","19",
	"20","21","22","23","24","25","26","27","28","29",
	"30","31","32","33","34","35","36","37","38","39",
	"40","41","42","43","44","45","46","47","48","49",
	"50","51","52","53","54","55","56","57","58","59",
	"60","61","62","63","64","65","66","67","68","69",
	"70","71","72","73","74","75","76","77","78","79",
	"80","81","82","83","84","85","86","87","88","89",
	"90","91","92","93","94","95","96","97","98","99",
};

const char* str_00_FF[] = {
	"00","01","02","03","04","05","06","07","08","09","0A","0B","0C","0D","0E","0F",
	"10","11","12","13","14","15","16","17","18","19","1A","1B","1C","1D","1E","1F",
	"20","21","22","23","24","25","26","27","28","29","2A","2B","2C","2D","2E","2F",
	"30","31","32","33","34","35","36","37","38","39","3A","3B","3C","3D","3E","3F",
	"40","41","42","43","44","45","46","47","48","49","4A","4B","4C","4D","4E","4F",
	"50","51","52","53","54","55","56","57","58","59","5A","5B","5C","5D","5E","5F",
	"60","61","62","63","64","65","66","67","68","69","6A","6B","6C","6D","6E","6F",
	"70","71","72","73","74","75","76","77","78","79","7A","7B","7C","7D","7E","7F",
	"80","81","82","83","84","85","86","87","88","89","8A","8B","8C","8D","8E","8F",
	"90","91","92","93","94","95","96","97","98","99","9A","9B","9C","9D","9E","9F",
	"A0","A1","A2","A3","A4","A5","A6","A7","A8","A9","AA","AB","AC","AD","AE","AF",
	"B0","B1","B2","B3","B4","B5","B6","B7","B8","B9","BA","BB","BC","BD","BE","BF",
	"C0","C1","C2","C3","C4","C5","C6","C7","C8","C9","CA","CB","CC","CD","CE","CF",
	"D0","D1","D2","D3","D4","D5","D6","D7","D8","D9","DA","DB","DC","DD","DE","DF",
	"E0","E1","E2","E3","E4","E5","E6","E7","E8","E9","EA","EB","EC","ED","EE","EF",
	"F0","F1","F2","F3","F4","F5","F6","F7","F8","F9","FA","FB","FC","FD","FE","FF",
};

int read_filter_id()
{
	int fd = 0, i = 0;
	for(i = 0; i < FILTER_ID_SIZE; i++)
	{
		global.filter_id[i] = -1;
	}
	fd = open(FILTER_ID_FILE,O_RDONLY,0);
	if(fd < 0)
	{
		rt_kprintf("open file[%s] error\n", FILTER_ID_FILE);
		return -1;
	}
	read(fd,global.filter_id,sizeof(global.filter_id));
	close(fd);
	for(i = 0;i < FILTER_ID_SIZE;i ++)
	{
		if(global.filter_id[i] == -1)
		{
			global.id_len = i;
			break;
		}
	}
	return 0;
}

rt_err_t mempool_init()
{
	global.mempool2 = rt_mp_create("mp2", MEMPOLL_COUNT, MEMPOLL_SIZE);
	global.mempool3 = rt_mp_create("mp3", MQ_LEN, UART_FRAME_SIZE);
	return rt_mp_init(&global.mempool1, "mp1",(void *)0x10000000, 0x10000, MEMPOLL_SIZE);
}

rt_err_t messagequeue_init()
{
	global.can1_mq = rt_mq_create("can1", sizeof(msg_t), MQ_LEN, RT_IPC_FLAG_FIFO);
	global.can2_mq = rt_mq_create("can2", sizeof(msg_t), MQ_LEN, RT_IPC_FLAG_FIFO);
	global.save_mq = rt_mq_create("save", sizeof(msg_t), MQ_LEN, RT_IPC_FLAG_FIFO);
	global.uart_mq = rt_mq_create("uart", sizeof(msg_t), MQ_LEN, RT_IPC_FLAG_FIFO);
	if(global.can1_mq == RT_NULL || global.can1_mq == RT_NULL 
		|| global.can1_mq == RT_NULL || global.uart_mq == RT_NULL)
		return RT_ERROR;
	return RT_EOK;
}

void can_init(CAN_TypeDef* CANx, unsigned int bps)
{	
	CAN_InitTypeDef CAN_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	// gpio
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_CAN2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_CAN2);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	if(CANx == CAN1)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
		NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
		NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	CAN_ITConfig(CANx, CAN_IT_FMP0, DISABLE);
	/*
		TTCM = time triggered communication mode
		ABOM = automatic bus-off management 
		AWUM = automatic wake-up mode
		NART = no automatic retransmission
		RFLM = receive FIFO locked mode 
		TXFP = transmit FIFO priority		
	*/
	CAN_InitStructure.CAN_TTCM = DISABLE;			/* 禁止时间触发模式（不生成时间戳), T  */
	CAN_InitStructure.CAN_ABOM = DISABLE;			/* 禁止自动总线关闭管理 */
	CAN_InitStructure.CAN_AWUM = DISABLE;			/* 禁止自动唤醒模式 */
	CAN_InitStructure.CAN_NART = DISABLE;			/* 禁止仲裁丢失或出错后的自动重传功能 */
	CAN_InitStructure.CAN_RFLM = DISABLE;			/* 禁止接收FIFO加锁模式 */
	CAN_InitStructure.CAN_TXFP = DISABLE;			/* 禁止传输FIFO优先级 */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Silent;	/* 设置CAN为回环模式 */
	
	/* 
		CAN 波特率 = RCC_APB1Periph_CAN1 / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		本例中，设置CAN波特率为1Mbps		
		CAN 波特率 = 420000000 / 2 / (1 + 12 + 8) / = 1 Mbps		
	*/
				
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	// 2Mbps
	switch(bps)
	{
		case 500000:
			CAN_InitStructure.CAN_Prescaler = 4;
			break;
		case 250000:
			CAN_InitStructure.CAN_Prescaler = 8;
			break;
		case 100000:
			CAN_InitStructure.CAN_Prescaler = 20;
			break;
		default:
			CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
			CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
			CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
			CAN_InitStructure.CAN_Prescaler = 2;
			break;
	}
	CAN_Init(CANx, &CAN_InitStructure);
	CAN_SlaveStartBank(14);
	CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);	
}
void can_filter_init(unsigned int num, FunctionalState NewState)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = num;	/* 筛选器序号，0-27，共28个筛选器 */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;		/* 筛选器模式，设置ID掩码模式 */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	/* 32位筛选 */
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;					/* 掩码后ID的高16bit */
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;					/* 掩码后ID的低16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;				/* ID掩码值高16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;				/* ID掩码值低16bit */
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;		/* 筛选器绑定FIFO 0 */
	CAN_FilterInitStructure.CAN_FilterActivation = NewState;				/* 使能筛选器 */
	CAN_FilterInit(&CAN_FilterInitStructure);
}

//字节流转换为十六进制字符串
void hex_2_str(const char *src,  char *dest, int len )  
{  
    int  i, res_len;  
    char tmp[3]; 
    for( i = 0; i < len; i++ )  
    {  
        sprintf(tmp, "%02X ", (unsigned char) src[i] );  
        rt_memcpy(&dest[i * 3], tmp, 3);  
    }	
		res_len = rt_strlen(dest);
		if(res_len > 0)
		{
				dest[res_len - 1] = 0;
		}
    return ;  
}  

void can_send_test(CAN_TypeDef* CANx,unsigned int data)
{
	static CanTxMsg TxMessage;
	TxMessage.StdId = 0x321;
	TxMessage.ExtId = 0x452;
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.DLC = 8;
	TxMessage.Data[0] = 0x09;
	TxMessage.Data[1] = 0xAB;
	TxMessage.Data[2] = 0xCD;
	TxMessage.Data[3] = 0xEF;
	TxMessage.Data[4] = (data>>24) & 0xFF;
	TxMessage.Data[5] = (data>>16) & 0xFF;
	TxMessage.Data[6] = (data>>8) & 0xFF;
	TxMessage.Data[7] = data & 0xFF;
	CAN_Transmit(CANx, &TxMessage);	
}

// 发送一个存储信息，返回一个新的内存块
char * send_save_msg(msg_type_t type,void *buf,int len,int timestamp)
{
	msg_t send_msg;		
	char * result = RT_NULL;
	rt_memset(&send_msg,0,sizeof(msg_t));			
	send_msg.type = type;
	send_msg.value = len;
	send_msg.p = buf;
#if USE_TIMESTAMPE	
	send_msg.timestamp = timestamp;
#endif	
	rt_mq_send(global.save_mq, &send_msg, sizeof(msg_t));
	switch(type)
	{
		case CAN1_SAVE:
			result = (char *)rt_mp_alloc(&global.mempool1,RT_WAITING_FOREVER);
			break;
		case CAN2_SAVE:
			result = (char *)rt_mp_alloc(global.mempool2,RT_WAITING_FOREVER);
			break;
		default:
			break;
	}
	rt_memset(result,0,MEMPOLL_SIZE);
	return result;
}

int frame_to_bin(msg_type_t type, CanRxMsg *can_msg, char* buf)
{
	int can_index = 0;
	// can 数据长度
	char can_data_len = 0;
	save_msg_t *msg = (save_msg_t *)buf;
	if(can_msg == RT_NULL || buf == RT_NULL)
		return -1;
	can_data_len = can_msg->DLC%9;
	switch(type)
	{
		case CAN1_RECV:
			can_index = 0;
			break;
		case CAN2_RECV:
			can_index = 1;
			break;
		default:
			return -1;
	}
	// 填充魔法数
	msg->magic = MAGIC_DTAA;
	msg->type = 0;
	msg->date = global.power_time + global.run_time;
	msg->us = get_us_timer();
	msg->id = can_msg->StdId + can_msg->ExtId;
	msg->len = can_data_len;
	if(can_msg->IDE)
	{	
		msg->type = EXTENDED_FRAME;
		// 扩展帧
		if(can_msg->RTR)
		{
			// 远程帧
			msg->type += REMOTE_FRAME;
			global.frame_info[can_index].ERF ++;
		}else
		{	
			// 数据帧
			msg->type += DATE_FRAME;
			global.frame_info[can_index].EDF ++;
		}
	}else
	{
		msg->type = STANDARD_FRASME;
		// 标准帧
		if(can_msg->RTR)
		{
			// 远程帧
			msg->type += REMOTE_FRAME;
			global.frame_info[can_index].SRF ++;
		}else
		{
			// 数据帧
			msg->type += DATE_FRAME;
			global.frame_info[can_index].SDF ++;
		}
	}
	if(!can_msg->RTR)
	{
		int i =0 ;
		rt_memset(msg->data,0,8);
		for(i=0; i < can_data_len; i++)
		{
			msg->data[i] = can_msg->Data[i];
		}
	}
	return sizeof(save_msg_t);
}

int frame_to_csv(msg_type_t type, CanRxMsg *can_msg, char* buf)
{
	int us = get_us_timer();
	int can_index = 0;
	int frame_len = 0;
	// 帧类型
	char *frame_type = RT_NULL;
	// can 数据长度
	char can_data_len = can_msg->DLC%10;
	// 帧类型
	static char *frame_type_list[4] = {"SRF","SDF","ERF","EDF"};
	// 时间
	time_t timep; 
	struct tm *tm_p; 
	if(can_msg == RT_NULL || buf == RT_NULL)
		return -1;
	switch(type)
	{
		case CAN1_RECV:
			can_index = 0;
			break;
		case CAN2_RECV:
			can_index = 1;
			break;
		default:
			return -1;
	}
		if(can_msg->IDE)
	{	
		// 扩展帧
		if(can_msg->RTR)
		{
			// 远程帧
			global.frame_info[can_index].ERF ++;
			frame_type = frame_type_list[2];
		}else
		{	
			// 数据帧
			global.frame_info[can_index].EDF ++;
			frame_type = frame_type_list[3];
		}
	}else
	{
		// 标准帧
		// 扩展帧
		if(can_msg->RTR)
		{
			// 远程帧
			global.frame_info[can_index].SRF ++;
			frame_type = frame_type_list[0];
		}else
		{
			// 数据帧
			global.frame_info[can_index].SDF ++;
			frame_type = frame_type_list[1];
		}
	}
	timep = global.power_time + global.run_time;
	tm_p =localtime(&timep);
	// 月份
	rt_strncpy(buf,str_00_99[1+tm_p->tm_mon], 2);
	frame_len += 2;
	buf[frame_len] = '-';
	frame_len++;
	// 日期
	rt_strncpy(buf + frame_len,str_00_99[tm_p->tm_mday], 2);
	frame_len += 2;
	buf[frame_len] = ' ';
	frame_len++;
	// 小时
	rt_strncpy(buf + frame_len,str_00_99[tm_p->tm_hour], 2);
	frame_len += 2;
	buf[frame_len] = ':';
	frame_len++;
	// 分钟
	rt_strncpy(buf + frame_len,str_00_99[tm_p->tm_min], 2);
	frame_len += 2;
	buf[frame_len] = ':';
	frame_len++;
	// 秒
	rt_strncpy(buf + frame_len,str_00_99[tm_p->tm_sec], 2);
	frame_len += 2;
	buf[frame_len] = '.';
	frame_len++;
	// 毫秒
	rt_sprintf(buf + frame_len,"%06d",us);
	frame_len += 6;
	buf[frame_len] = ',';
	frame_len++;
	// 类型
	rt_strncpy(buf + frame_len,frame_type, 3);
	frame_len += 3;
	buf[frame_len] = ',';
	frame_len++;
	//ID
	rt_sprintf(buf + frame_len,"0x%08X",can_msg->StdId + can_msg->ExtId);
	frame_len += 10;
	buf[frame_len] = ',';
	frame_len++;
	// 长度
	buf[frame_len] = can_data_len + '0';
	frame_len++;
	buf[frame_len] = ',';
	frame_len++;
	// 数据
	if(!can_msg->RTR)
	{
		int i = 0;
		for(i = 0; i < can_data_len; i++)
		{
			rt_strncpy(buf + frame_len,str_00_FF[can_msg->Data[i]], 2);
			frame_len += 2;
			buf[frame_len] = ' ';
			frame_len++;
		}
		if(i > 0)
			frame_len--;
	}
	
	buf[frame_len] = '\n';
	frame_len++;
	return frame_len;
}

#if USE_TIMESTAMPE
void calc_timestampe(timestamp_t *t)
{
	int finish_time = 0;
	finish_time = get_us_timer() - t->start;
	finish_time = finish_time > 0 ? finish_time: finish_time + 1000000;
	if(t->min > finish_time || t->min == 0)
		t->min = finish_time;
	if(t->max < finish_time)
		t->max = finish_time;
	if(t->sum == 0)
	{
		t->sum = 1;
		t->avg = finish_time;
	}
	else
	{
		t->avg = ((unsigned long)((unsigned long)t->avg * t->sum + finish_time))/(++t->sum);
	}
}
#endif

#ifdef RT_USING_FINSH
#include "finsh.h"
void show_frame(void)
{
	int i = 0;
	for(i = 0; i < 2; i++)
	{
		rt_kprintf("can[%d] ==> SRF[%d] SDF[%d] ERF[%d] EDF[%d]\r\n",
			i+1, global.frame_info[i].SRF,global.frame_info[i].SDF,
		global.frame_info[i].ERF,global.frame_info[i].EDF);
	}
}
FINSH_FUNCTION_EXPORT(show_frame, show can frame info.)
#if USE_TIMESTAMPE
void show_timestampe(void)
{
	rt_kprintf("can0 ==> min[%d] avg[%d] max[%d]\n", global.timestamp[0].min, 
	global.timestamp[0].avg,global.timestamp[0].max);
	rt_kprintf("can1 ==> min[%d] avg[%d] max[%d]\n", global.timestamp[1].min, 
	global.timestamp[1].avg,global.timestamp[1].max);
	rt_kprintf("save1 ==> min[%d] avg[%d] max[%d]\n", global.timestamp[2].min, 
	global.timestamp[2].avg,global.timestamp[2].max);
	rt_kprintf("save2 ==> min[%d] avg[%d] max[%d]\n", global.timestamp[3].min, 
	global.timestamp[3].avg,global.timestamp[3].max);
	rt_kprintf("save idle ==> min[%d] avg[%d] max[%d]\n", global.timestamp[4].min, 
	global.timestamp[4].avg,global.timestamp[4].max);
}
FINSH_FUNCTION_EXPORT(show_timestampe, show  timestampe info.)
#endif
#endif
