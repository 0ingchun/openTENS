/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void GPIO_Init(void)
{	
	GPIO_InitTypeDef  GPIO_InitStruct = {0};
	
  // GPIO Ports Clock Enable 
    __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();                          /* 使能GPIOB时钟 */
	
	// HAL_GPIO_WritePin(LED13_GPIO_Port, LED13_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, NET_P_Pin|NET_N_Pin|AT_STA_Pin, GPIO_PIN_RESET);	// Configure GPIO pin Output Level
	
	  // Configure GPIO pins : PBPin PBPin
  GPIO_InitStruct.Pin = NET_P_Pin|NET_N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Configure GPIO pin : PtPin
  GPIO_InitStruct.Pin = AT_STA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AT_STA_GPIO_Port, &GPIO_InitStruct);
	
  /* GPIO初始化 */
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);      

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
