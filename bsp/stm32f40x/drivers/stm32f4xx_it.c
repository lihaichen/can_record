/**
  ******************************************************************************
  * @file    IO_Toggle/stm32f4xx_it.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <rtthread.h>

#include "board.h"
#include "common.h"
#include "us.h"
/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup IO_Toggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}


/**
  * @brief  This function handles SDIO global interrupt request.
  * @param  None
  * @retval None
  */
void SDIO_IRQHandler(void)
{
	extern void SD_ProcessIRQSrc(void);
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
}

/**
  * @brief  This function handles DMA2 Stream3 or DMA2 Stream6 global interrupts
  *         requests.
  * @param  None
  * @retval None
  */
void DMA2_Stream3_IRQHandler(void)
{
	extern void SD_ProcessDMAIRQ(void);
	/* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
	SD_ProcessDMAIRQ();
}

void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
    /* Clear the EXTI line 0 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}

void CAN1_RX0_IRQHandler(void){
	msg_t msg;
	static CanRxMsg can_msg[MQ_LEN ];
	static int i = 0;
	rt_memset(&msg,0,sizeof(msg_t));
	rt_memset(&can_msg[i],0,sizeof(CanRxMsg));
#if USE_TIMESTAMPE
	msg.timestamp = get_us_timer();
#endif		
	CAN_Receive(CAN1, CAN_FIFO0, &can_msg[i]);
	msg.type = CAN1_RECV;
	msg.p = (void *)&can_msg[i];
	if(rt_mq_send(global.can1_mq, &msg, sizeof(msg_t)) != RT_EOK)
	{
		rt_kprintf("1");
	}
	i = (i+1)%(MQ_LEN);
}


void CAN2_RX0_IRQHandler(void){
	msg_t msg;
	static CanRxMsg can_msg[MQ_LEN ];
	static int i = 0;
	rt_memset(&msg,0,sizeof(msg_t));
	rt_memset(&can_msg[i],0,sizeof(CanRxMsg));
#if USE_TIMESTAMPE
	msg.timestamp = get_us_timer();
#endif	
	CAN_Receive(CAN2, CAN_FIFO0, &can_msg[i]);
	msg.type = CAN2_RECV;
	msg.p = (void *)&can_msg[i];
	if(rt_mq_send(global.can2_mq, &msg, sizeof(msg_t)) != RT_EOK)
	{
		rt_kprintf("2");
	}
	i = (i+1)%(MQ_LEN);
}
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)==SET) //溢出中断
	{
		global.run_time++;
	}
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);  //清除中断标志位
}
/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
