/**
  ******************************************************************************
  * @file    air001xx_it.c
  * @author  MCU Application Team
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) AirM2M.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "air001xx_it.h"

/* Private includes ----------------------------------------------------------*/
#include "usart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*          Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
extern ADC_HandleTypeDef             hadc1;
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/* AIR001xx Peripheral Interrupt Handlers                                     */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file.                                          */
/******************************************************************************/

/**
  * @brief This function handles TIM1 interrupt.
  */
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
//  HAL_TIM_IRQHandler(&htim1);
}

/******************************************************************************/
/* AIR001xx Peripheral Interrupt Handlers                                     */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file.                                          */
/******************************************************************************/
/**
  * @brief This function handles DMA1 channel1 Interrupt .
  */

void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(huart2.hdmatx);
}

/**
  * @brief This function handles DMA1 channel2 and channel3 Interrupt .
  */
void DMA1_Channel2_3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(huart2.hdmarx);
}

void DMA1_Channel3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hadc1.DMA_Handle);
}

/**
  * @brief This function handles USART1 Interrupt .
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}

/**
  * @brief This function handles USART2 Interrupt .
  */
void USART2_IRQHandler(void)
{
	// ���ڲ��ָ���https://blog.csdn.net/GQ_Sonofgod/article/details/118335944	
	uint32_t tmp_flag = 0;
	uint32_t temp;
	
	if(USART2 == huart2.Instance){

		tmp_flag =__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE); //��ȡIDLE��־λ
		if((tmp_flag != RESET))//idle��־����λ
		{
			__HAL_UART_CLEAR_IDLEFLAG(&huart2);//�����־λ
			temp = huart2.Instance->SR;  //���״̬�Ĵ���SR,��ȡSR�Ĵ�������ʵ�����SR�Ĵ����Ĺ���
			temp = huart2.Instance->DR; //��ȡ���ݼĴ����е�����
			HAL_UART_DMAStop(&huart2); //	ֹͣuart��dma
			//temp  = hdma_usart2_rx.Instance->CNDTR;// ��ȡDMA��δ��������ݸ�����NDTR�Ĵ�������������
			temp  =  __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);// ��ȡDMA��δ��������ݸ���
			usart2_rx_len =  sizeof(usart2_rx_buffer) - temp; //�ܼ�����ȥδ��������ݸ������õ��Ѿ����յ����ݸ���
			//usart2_recv_end_flag = 1;	// ������ɱ�־λ��1	
			usart2_recv_end_flag = usart2_rx_len?1:0;	// ������յ����ݣ�������ɱ�־
			
			HAL_UART_Receive_DMA(&huart2,usart2_rx_buffer,sizeof(usart2_rx_buffer));	// ���´�DMA����	//��Ҫ����			
		}
		
		if (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) && (HAL_UART_GetState(&huart2) == HAL_UART_STATE_READY || __HAL_DMA_GET_COUNTER(&hdma_usart2_tx) == 0) )	// ��ȡDMA��δ��������ݸ���
		{
			// ��F7ϵ���ǿ��Բ�д�ģ�F1����д
			__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC1); //���DMA2_Steam7������ɱ�־
			HAL_UART_DMAStop(&huart2);		//��������Ժ�رմ���DMA,ȱ����һ�������
			
			usart2_trans_end_flag = 1;
		}
	
	}
	
  HAL_UART_IRQHandler(&huart2);
}

/************************ (C) COPYRIGHT AirM2M *****END OF FILE******************/
