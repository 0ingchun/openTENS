/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */

extern uint8_t usart2_rx_buffer[256];
extern volatile uint8_t usart2_rx_len;
extern volatile uint8_t usart2_recv_end_flag;

extern uint8_t usart2_tx_buffer[256];
extern volatile uint8_t usart2_tx_len;
extern volatile uint8_t usart2_trans_end_flag;

extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE END Private defines */

void USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */

void UART2_TX_DMA_Send(uint8_t *buffer, uint16_t length, uint32_t Timeout);

void dma_printf(const char *format, ...);

//int fputc(int ch, FILE *f);
//int fgetc(FILE *f)

uint16_t CheckSumCrc16(uint8_t *ptr,uint8_t len);

void PackageSendData(uint8_t* data, uint16_t* len);
uint8_t UnpackReceivedData(uint8_t* data, uint8_t* len);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

