#include "common.h"
#include <stdio.h>
#include <time.h> 
#include "us.h"
global_t global;

rt_err_t mempool_init()
{
	global.mempool2 = rt_mp_create("mp2", MEMPOLL_COUNT, MEMPOLL_SIZE);
	return rt_mp_init(&global.mempool1, "mp1",(void *)0x10000000, 0x10000, MEMPOLL_SIZE);
}

rt_err_t messagequeue_init()
{
	global.can1_mq = rt_mq_create("can1", sizeof(msg_t), MQ_LEN, RT_IPC_FLAG_FIFO);
	global.can2_mq = rt_mq_create("can2", sizeof(msg_t), MQ_LEN, RT_IPC_FLAG_FIFO);
	global.save_mq = rt_mq_create("save", sizeof(msg_t), MQ_LEN, RT_IPC_FLAG_FIFO);
	if(global.can1_mq == RT_NULL || global.can1_mq == RT_NULL || global.can1_mq == RT_NULL)
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
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
		NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
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
    int  i;  
    char tmp[3];  
    for( i = 0; i < len; i++ )  
    {  
        sprintf(tmp, "%02X ", (unsigned char) src[i] );  
        rt_memcpy(&dest[i * 3], tmp, 3);  
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
char * send_save_msg(msg_type_t type,void *buf,int len,int save_index)
{
	msg_t send_msg;		
	char * result = RT_NULL;
	rt_memset(&send_msg,0,sizeof(msg_t));			
	send_msg.type = type;
	send_msg.value = len;
	send_msg.p = buf;
	send_msg.reserve = save_index;
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
	// rt_kprintf("c>%d-%d-%d\n",type,len,save_index);
	return result;
}
// 将can数据解析存储buf，格式为csv
int frame_to_csv(msg_type_t type, CanRxMsg *can_msg, char* buf)
{
	int can_index = 0;
	// can 数据hextostr
	char tmp[64];
	// 帧类型
	char *frame_type = RT_NULL;
		// 时间
	time_t timep;  
  struct tm *tm_p; 
	rt_tick_t tick;
	int us;
	// 帧类型
	static char *frame_type_list[4] = {"SRF","SDF","ERF","EDF"};
	tick = rt_tick_get();
	if(can_msg == RT_NULL)
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
	us = get_us_timer();
	timep = global.time + tick/RT_TICK_PER_SECOND;
	tm_p =localtime(&timep);
	return sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d.%.6d,%s,0x%X,%s\n",
					tm_p->tm_year + 1900,(1+tm_p->tm_mon), tm_p->tm_mday,
					tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec,us,
					frame_type,can_msg->StdId + can_msg->ExtId,tmp);
}

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
#endif
