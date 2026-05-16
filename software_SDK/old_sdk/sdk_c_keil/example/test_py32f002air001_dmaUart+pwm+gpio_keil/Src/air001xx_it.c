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
	// 串口部分改自https://blog.csdn.net/GQ_Sonofgod/article/details/118335944	
	uint32_t tmp_flag = 0;
	uint32_t temp;
	
	if(USART2 == huart2.Instance){

		tmp_flag =__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE); //获取IDLE标志位
		if((tmp_flag != RESET))//idle标志被置位
		{
			__HAL_UART_CLEAR_IDLEFLAG(&huart2);//清除标志位
			temp = huart2.Instance->SR;  //清除状态寄存器SR,读取SR寄存器可以实现清除SR寄存器的功能
			temp = huart2.Instance->DR; //读取数据寄存器中的数据
			HAL_UART_DMAStop(&huart2); //	停止uart的dma
			//temp  = hdma_usart2_rx.Instance->CNDTR;// 获取DMA中未传输的数据个数，NDTR寄存器分析见下面
			temp  =  __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);// 获取DMA中未传输的数据个数
			usart2_rx_len =  sizeof(usart2_rx_buffer) - temp; //总计数减去未传输的数据个数，得到已经接收的数据个数
			//usart2_recv_end_flag = 1;	// 接受完成标志位置1	
			usart2_recv_end_flag = usart2_rx_len?1:0;	// 如果接收到数据，设置完成标志
			
			HAL_UART_Receive_DMA(&huart2,usart2_rx_buffer,sizeof(usart2_rx_buffer));	// 重新打开DMA接收	//？要加吗			
		}
		
		if (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) && (HAL_UART_GetState(&huart2) == HAL_UART_STATE_READY || __HAL_DMA_GET_COUNTER(&hdma_usart2_tx) == 0) )	// 获取DMA中未传输的数据个数
		{
			// 在F7系列是可以不写的，F1必须写
			__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC1); //清除DMA2_Steam7传输完成标志
			HAL_UART_DMAStop(&huart2);		//传输完成以后关闭串口DMA,缺了这一句会死机
			
			usart2_trans_end_flag = 1;
		}
	
	}
	
  HAL_UART_IRQHandler(&huart2);
}

/************************ (C) COPYRIGHT AirM2M *****END OF FILE******************/
