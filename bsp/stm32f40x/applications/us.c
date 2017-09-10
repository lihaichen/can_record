#include "stm32f4xx_tim.h"

void us_timer_init(){
	TIM_TimeBaseInitTypeDef conf;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  ///使能TIM3时钟
	TIM_TimeBaseStructInit(&conf);
	conf.TIM_Prescaler = 83;
	conf.TIM_CounterMode = TIM_CounterMode_Down;
	conf.TIM_Period = 1000000 -1;
	conf.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM5,&conf);
		
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE); //允许定时器5更新中断
	TIM_Cmd(TIM5,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn; //定时器5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=15; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=15; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

int get_us_timer(){
	return 1000000 - TIM_GetCounter(TIM5);
}
