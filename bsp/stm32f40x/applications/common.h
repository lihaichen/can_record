#ifndef __COMMON_H__
#define __COMMON_H__
#include "stm32f4xx_can.h"

extern int rt_can1_init(void);
extern int rt_can2_init(void);
extern void can_init(CAN_TypeDef* CANx, unsigned int bps);
extern void can_filter_init(unsigned int num);
extern void can_nvic_config(CAN_TypeDef* CANx,FunctionalState NewState);
extern void can_send_test(CAN_TypeDef* CANx,char data);
#endif
