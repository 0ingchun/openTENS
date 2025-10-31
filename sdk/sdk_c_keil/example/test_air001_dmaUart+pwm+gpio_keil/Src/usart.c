/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"
#include "stdio.h"

/* USER CODE BEGIN 0 */

uint8_t usart2_rx_buffer[256] = {0};
volatile uint8_t usart2_rx_len = 0;
volatile uint8_t usart2_recv_end_flag = 0;	//	初始化为未接收状态

uint8_t usart2_tx_buffer[256] = {0};
volatile uint8_t usart2_tx_len = 0;
volatile uint8_t usart2_trans_end_flag = 1;	//	初始化为空闲状态

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

/* usart2 init function */

void USART2_UART_Init(void)
{
    /* USART2初始化 */
  huart2.Instance          = USART2;
  huart2.Init.BaudRate     = 9600;
  huart2.Init.WordLength   = UART_WORDLENGTH_8B;
  huart2.Init.StopBits     = UART_STOPBITS_1;
  huart2.Init.Parity       = UART_PARITY_NONE;
  huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart2.Init.Mode         = UART_MODE_TX_RX;
  if (HAL_UART_Init(&huart2) != HAL_OK)	// 此时调用msp了 void HAL_UART_MspInit(UART_HandleTypeDef *huart)
  {
    Error_Handler();
  }

	//使能收发中断，但是这两个好像不是这么使能哒？？？
	//HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer[0], sizeof(usart2_rx_buffer));
	//HAL_UART_Transmit_IT(&huart2, &usart2_tx_buffer[0], sizeof(usart2_tx_buffer));
	//添加使能idle中断和打开串口DMA接收语句
	
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//使能idle中断,开启空闲接收中断
	HAL_UART_Receive_DMA(&huart2,usart2_rx_buffer,sizeof(usart2_rx_buffer));//打开DMA接收，数据存入rx_buffer数组中，指定接收缓存区和接收大小

}

/**
  * @brief 初始化USART的MSP
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  /* 使能时钟 */
  __HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_DMA_CLK_ENABLE();
  
  /**USART2 引脚配置
    PA0     ------> USART2_TX
    PA1    ------> USART2_RX
    */
  GPIO_InitTypeDef  GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART2 DMA相关配置 */
  /* USART2_TX 初始化 */
  hdma_usart2_tx.Instance = DMA1_Channel1;
  hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart2_tx.Init.Mode = DMA_NORMAL;
  hdma_usart2_tx.Init.Priority = DMA_PRIORITY_HIGH;
  if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_LINKDMA(&huart2, hdmatx, hdma_usart2_tx);
	
  /* USART2_RX 初始化 */
  hdma_usart2_rx.Instance = DMA1_Channel2;
  hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart2_rx.Init.Mode = DMA_NORMAL;
  hdma_usart2_rx.Init.Priority = DMA_PRIORITY_HIGH;
  if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_LINKDMA(&huart2, hdmarx, hdma_usart2_rx);
	
  /*UART2_TX DMA_CH1-0x7  ; UART2_RX DMA_CH2-0x8*/
  HAL_SYSCFG_DMA_Req(0x0807);
  /* 使能NVIC */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 1);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

//串口1的DMA发送，主要先等待上一帧数据发送完成再发送下一帧数据，多次调用需加延时等待或查询dma传输完毕

/*
void UART2_TX_DMA_Send(uint8_t *buffer, uint16_t length, uint32_t Timeout)	// 旧版阻塞发送
{
    //等待上一次的数据发送完毕
	while(HAL_DMA_GetState(&hdma_usart2_tx) != HAL_DMA_STATE_READY);
    //while(__HAL_DMA_GET_COUNTER(&hdma_usart2_tx));
	
    //关闭DMA
    __HAL_DMA_DISABLE(&hdma_usart2_tx);

    //开始发送数据
    HAL_UART_Transmit_DMA(&huart2, buffer, length);
	
	HAL_UART_Transmit_IT(&huart2, &usart2_tx_buffer[0], sizeof(usart2_tx_buffer));	
}
*/


/*
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}
 

int fgetc(FILE *f)
{
  uint8_t ch = 0;
  HAL_UART_Receive(&huart1, &ch, 1, 0xffff);
  return ch;
}
*/

void UART2_TX_DMA_Send(uint8_t *buffer, uint16_t length, uint32_t Timeout)	// 优化后的一坨大便
{
	// 检查输入参数
	if (buffer == NULL || length == 0) {
			return; // 输入无效，直接返回
	}

	// 等待上一次的DMA传输完成，使用非阻塞方式
	uint32_t timeout = HAL_GetTick() + Timeout; // 设置超时时间为Timeout(单位ms)
	
	while (!usart2_trans_end_flag) {
		// 再次检查
		if (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) && (HAL_UART_GetState(&huart2) == HAL_UART_STATE_READY || __HAL_DMA_GET_COUNTER(&hdma_usart2_tx) == 0) )	// 检查DMA中未传输的数据个数是否为0
		{
			//强制结束上次和本次发送
			// 在F7系列是可以不写的，F1必须写
			__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC1); //清除DMA2_Steam7传输完成标志
			HAL_UART_DMAStop(&huart2);		//传输完成以后关闭串口DMA,缺了这一句会死机
			usart2_trans_end_flag = 1;
		}
		else if (HAL_GetTick() > timeout) {
			
			// 超时处理，可以根据需要添加日志打印或错误回调
			printf("DMA transmit timeout.\n");
			
			//强制结束上次和本次发送
			// 在F7系列是可以不写的，F1必须写
			//__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC4); //清除DMA2_Steam7传输完成标志
			HAL_UART_DMAStop(&huart2);		//传输完成以后关闭串口DMA,缺了这一句会死机
			usart2_trans_end_flag = 1;
			
			// return;	// 终止本次发送
		}
	}
	usart2_trans_end_flag = 0; // 清除完成标志，准备发送
	
	//关闭DMA
    __HAL_DMA_DISABLE(&hdma_usart2_tx);

	// 发起新的DMA传输
	if (HAL_UART_Transmit_DMA(&huart2, buffer, length) != HAL_OK) {
		// DMA传输启动失败处理
		usart2_trans_end_flag = 1; // 恢复标志位，以允许尝试下一次传输
		// 根据需要添加错误处理代码，如日志记录或错误回调
		printf("Failed to start DMA transmit.\n");
	}
	
	    
    //while(HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY);	// 等待发送数据完成
	//while(__HAL_DMA_GET_COUNTER(&hdma_usart2_tx) != 0);	// 打开可能会死循环在里面
	if (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) && (HAL_UART_GetState(&huart2) == HAL_UART_STATE_READY || __HAL_DMA_GET_COUNTER(&hdma_usart2_tx) == 0) )// 检查DMA中未传输的数据个数是否为0
	{
		// 在F7系列是可以不写的，F1必须写
		__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC1); //清除DMA2_Steam7传输完成标志
		HAL_UART_DMAStop(&huart2);		//传输完成以后关闭串口DMA,缺了这一句会死机
		usart2_trans_end_flag = 1;
		return;
	}
}


// 注意打开	#define NRF_MODULE_ENABLED(APP_UART)	宏定义
//串口1的DMA发送printf
void dma_printf(const char *format, ...)
{
	uint32_t length = 0;
	va_list args;
	
	va_start(args, format);
	
	length = vsnprintf((char*)usart2_tx_buffer, sizeof(usart2_tx_buffer), (char*)format, args);
	
	UART2_TX_DMA_Send(usart2_tx_buffer, length, 0x04ff);
		//	vprintf(format, args);	// 示例？
	
	va_end(args);
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) //如果是串口1
	{

			if (__HAL_DMA_GET_COUNTER(&hdma_usart2_tx) == 0)// 检查DMA中未传输的数据个数是否为0
			{
				// 在F7系列是可以不写的，F1必须写
				__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC1); //清除DMA2_Steam7传输完成标志
				HAL_UART_DMAStop(&huart2);		//传输完成以后关闭串口DMA,缺了这一句会死机
				usart2_trans_end_flag = 1;
			}
		
		//HAL_UART_Transmit_IT(&huart2, &usart2_tx_buffer[0], sizeof(usart2_tx_buffer));	
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2)	
	{
		
		//HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer[0], sizeof(usart2_rx_buffer));	
	}
}


// 原文链接：https://blog.csdn.net/qq_30267617/article/details/118877845


/**
  * 函数功能: 重定向c库函数printf到DEBUG_USARTx
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
// int fputc(int ch, FILE *f){  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xffff);  return ch;}
 
/**
  * 函数功能: 重定向c库函数getchar,scanf到DEBUG_USARTx
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
// int fgetc(FILE *f){  uint8_t ch = 0;  HAL_UART_Receive(&huart2, &ch, 1, 0xffff);  return ch;}

// 串口部分改自https://blog.csdn.net/GQ_Sonofgod/article/details/118335944

// CRC16MODBUS校验函数
uint16_t CheckSumCrc16(uint8_t *ptr,uint8_t len)
{
	unsigned long wcrc=0XFFFF;//预置16位crc寄存器，初值全部为1
	int i=0,j=0;//定义计数
	for(i=0;i<len;i++)//循环计算每个数据
	{
		wcrc^=*ptr++;//将八位数据与crc寄存器亦或.指针地址增加，指向下个数据
		for(j=0;j<8;j++)//循环计算数据的
		{
			if(wcrc&0X0001)//判断右移出的是不是1，如果是1则与多项式进行异或。
			{
				wcrc=wcrc>>1^0XA001;//先将数据右移一位再与上面的多项式进行异或
			}
			else//如果不是1，则直接移出
			{
				wcrc>>=1;//直接移出
			}
		}
	}
	return wcrc<<8|wcrc>>8;//低八位在前，高八位在后
}


#ifndef CRC16MODBUS
#define CRC16MODBUS CheckSumCrc16

uint8_t FrameHeader[] = {0x01, 0x02, 0x03}; // 示例包头
uint8_t FrameEnd[] = {0x07, 0x08, 0x09}; // 示例包尾
uint8_t FrameHeaderSize = sizeof(FrameHeader) / sizeof(FrameHeader[0]);
uint8_t FrameEndSize = sizeof(FrameEnd) / sizeof(FrameEnd[0]);

/***********************************************************************************************************
* 函数名称: PackageSendData()
* 输入参数: data,要发送的数据的首地址； *len,发送的原始数据数据的总长度
* 返回值  : 无
* 功    能: 根据通信协议，封装要发送的数据
************************************************************************************************************/
void PackageSendData(uint8_t* data, uint16_t* len)
{
  uint8_t temp[*len+FrameHeaderSize+FrameEndSize+3];
  uint16_t crc;
  
  memset(&temp[0], 0, sizeof(temp));
	
  memcpy(&temp[0], FrameHeader, FrameHeaderSize);	// 复制帧头
	
  memcpy(&temp[FrameHeaderSize+1], &data[0], *len); //	复制传输数据
	
  temp[FrameHeaderSize] = *len; //	加入长度值,仅真实数据的长度
	
  crc = CRC16MODBUS(&temp[FrameHeaderSize], *len+1); //	计算 (长度位+数据位) 的CRC16校验值
  memcpy(&temp[*len+FrameHeaderSize+1], &crc, 2); //	复制校验值
	
	memcpy(&temp[*len+FrameHeaderSize+3], FrameEnd, FrameEndSize);	// 复制包尾
	
  memcpy(&data[0], &temp[0], *len+FrameHeaderSize+FrameEndSize+3);	// 赋值整个包
	
  *len = *len+FrameHeaderSize+FrameEndSize+3;	// 赋值整个包的长度
}

/*
	//UART_Transmit test BEGIN 串口数据发送测试
	uint8_t cnt;
	cnt = 5;
	uint8_t uart2BUF[32] = {0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x21,0x22};	// 请设置缓冲
	PackageSendData((uint8_t *) uart2BUF, &cnt);
	HAL_UART_Transmit(&huart2,(uint8_t *)uart2BUF,cnt,0XFF);
		// UART_Transmit test END
*/

/***********************************************************************************************************
* 函数名称: UnpackReceivedData()
* 输入参数: data,要解包的数据的首地址，*len 要解包的原始数据的总长度（包括所有的包括干扰项）
* 返回值  : 0， 数据有效， 其他，数据无效。
* 功    能: 解包接收到的数据并判断数据有效性
************************************************************************************************************/

// 假定CRC16MODBUS函数和FrameHeader、FrameEnd的定义已经存在

// 解包函数
// data: 接收到的数据数组
// totalLen: 指向接收数据长度的指针

/*
uint8_t UnpackReceivedData(uint8_t* data, uint8_t* totalLen) {
    uint16_t crc, crctemp;
    uint8_t len;
    uint32_t headerStartIndex = -1;
    uint8_t i, j;
	
		// 搜索包头
    for (i = 0; i <= *totalLen - FrameHeaderSize; i++) {
        // 检查潜在的包头
        for (j = 0; j < FrameHeaderSize; j++) {
            if (data[i + j] != FrameHeader[j]) {
                break; // 如果任何一个字节不匹配，立即停止内层循环
            }
        }
        if (j == FrameHeaderSize) { // 如果找到潜在的包头
            // 暂时假设找到了包头，尝试读取长度位并验证包尾
            len = data[i + FrameHeaderSize]; // 原始数据长度

            // 验证包尾前，确保总长度足够
            if (i + FrameHeaderSize + 1 + len + 2 + FrameEndSize <= *totalLen) {
                // 检查包尾
                uint8_t endPtr = i + FrameHeaderSize + 1 + len + 2; // 包尾起始位置
                for (j = 0; j < FrameEndSize; j++) {
									if (data[endPtr + j] != FrameEnd[j]) {
											break; // 包尾不匹配
									}
                }
                if (j == FrameEndSize) { // 包尾匹配
									
									headerStartIndex = i; // 确认找到了真正的包头
									
									//
									// 复制接收数据中的CRC校验值
									memcpy(&crctemp, data + headerStartIndex + FrameHeaderSize + 1 + len, 2);

									// 计算CRC
									crc = CRC16MODBUS(data + headerStartIndex + FrameHeaderSize, len + 1);

									if (crctemp == crc) {
										// CRC校验成功，剔除包头前的无关数据和包头
									
										for (i = 0; i < len; i++) {	// 赋值解析后数据
												data[i] = data[headerStartIndex + FrameHeaderSize + 1 + i];
										}
										memset(&data[len], 0, *totalLen - len); // 清空剩余部分

										*totalLen = len; // 更新长度为实际数据长度
										return 0; // 校验成功
									}
									else {
										return 1; // CRC校验失败
									}
									//
										
									break; // 停止搜索
                }
                // 如果包尾不匹配，则继续搜索下一个潜在的包头
            }
        }
    }
    if (headerStartIndex == -1) {
        // printf("Header not found\r\n");
        return 2; // 找不到包头或与其匹配的包尾
    }
		else return 9;	// 未知错误
}
*/

uint8_t UnpackReceivedData(uint8_t* data, uint8_t* totalLen) {
    uint16_t crc, crctemp;
    uint8_t len;
    uint32_t headerStartIndex = -1;
    uint8_t i, j;

    // 搜索包头
    for (i = 0; i <= *totalLen - FrameHeaderSize; i++) {
        for (j = 0; j < FrameHeaderSize; j++) {
            if (data[i + j] != FrameHeader[j]) {
                break; // 如果任何一个字节不匹配，立即停止内层循环
            }
            if (j == FrameHeaderSize - 1) {
                headerStartIndex = i; // 找到包头的起始位置
            }
        }
        if (headerStartIndex != -1) {
            break; // 找到包头后停止搜索
        }
    }

    if (headerStartIndex == -1) {
        printf("Header not found\r\n");
        return 2; // 找不到包头
    }
		
    // 从找到的包头位置开始读取长度位
    len = data[headerStartIndex + FrameHeaderSize]; // 原始数据长度

    // 检查总长度是否足够
    if (*totalLen < headerStartIndex + FrameHeaderSize + 1 + len + 2 + FrameEndSize) {
        return 3; // 数据长度不足
    }
		
		// 根据长度位定位包尾的起始位置
    // 包尾起始位置 = 包头大小 + 长度位(1) + 数据长度 + CRC校验码长度(2)
    uint8_t endPtr = headerStartIndex + FrameHeaderSize + 1 + len + 2;

    // 验证包尾
    for (i = 0; i < FrameEndSize; ++i) {
        if (data[endPtr + i] != FrameEnd[i]) {
            printf("End Mismatch at byte %d: %X\r\n", i, data[endPtr + i]);
            return 4; // 包尾不匹配
        }
    }

    // 复制接收数据中的CRC校验值
    memcpy(&crctemp, data + headerStartIndex + FrameHeaderSize + 1 + len, 2);

    // 计算CRC
    crc = CRC16MODBUS(data + headerStartIndex + FrameHeaderSize, len + 1);

    if (crctemp == crc) {
        // CRC校验成功，剔除包头前的无关数据和包头
        for (i = 0; i < len; i++) {	// 赋值解析后数据
            data[i] = data[headerStartIndex + FrameHeaderSize + 1 + i];
            dma_printf("P%d=%d\r\n", i,data[i]);
            HAL_Delay(10);
        }
        memset(&data[len], 0, *totalLen - len); // 清空剩余部分

        *totalLen = len; // 更新长度为实际数据长度
        return 0; // 校验成功
    } else {
        return 1; // CRC校验失败
    }
		
}

#endif



/* USER CODE END 1 */
