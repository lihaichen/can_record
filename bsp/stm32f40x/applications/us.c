#include "stm32f4xx_tim.h"

void us_timer_init(){
	TIM_TimeBaseInitTypeDef conf;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  ///使能TIM3时钟
	TIM_TimeBaseStructInit(&conf);
	conf.TIM_Prescaler = 83;
	conf.TIM_CounterMode = TIM_CounterMode_Down;
	conf.TIM_Period = 1000000 -1;
	conf.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM5,&conf);
	TIM_ClearFlag(TIM5,TIM_FLAG_Update);
	TIM_Cmd(TIM5,ENABLE);
}

int get_us_timer(){
	return 1000000 - TIM_GetCounter(TIM5);
}
