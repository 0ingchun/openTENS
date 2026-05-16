/**
  ******************************************************************************
  * @file    main.h
  * @author  MCU Application Team
  * @brief   Header for main.c file.
  *          This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "air001xx_hal.h"
//#include "air001xx_hal_conf.h"	// 添加外设功能hal库时请注意修改该配置！！！
#include "air001_Core_Board.h"
#include <stdbool.h>

/* Private includes ----------------------------------------------------------*/
#include "stdio.h"
#include "string.h"
#include "stdint.h"
//#include "stdbool.h"
#include "stdarg.h"

/* Exported functions prototypes ---------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

// #define LED13_Pin GPIO_PIN_13
// #define LED13_GPIO_Port GPIOC

#define NET_P_Pin GPIO_PIN_1
#define NET_P_GPIO_Port GPIOB

#define NET_N_Pin GPIO_PIN_0
#define NET_N_GPIO_Port GPIOB

#define AT_STA_Pin GPIO_PIN_2
#define AT_STA_GPIO_Port GPIOB

#define ADC_GRS_Pin GPIO_PIN_5
#define ADC_GRS_GPIO_Port GPIOA

/* Exported variables prototypes ---------------------------------------------*/


/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT AirM2M *****END OF FILE******************/
