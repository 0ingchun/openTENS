/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  * @date
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
#include "crc.h"
// #include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "adc.h"

#include "shockModule.h"

/* Private define ------------------------------------------------------------*/
#ifdef __GNUC__
  /* With GCC, small printf (option LD Linker->Libraries->Small printf
  set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* Private variables ---------------------------------------------------------*/
//UART_HandleTypeDef UartHandle;
uint8_t aTxBuffer[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
uint8_t aRxBuffer[12] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

uint32_t   gADCxConvertedData = 1;

/* Private user code ---------------------------------------------------------*/
static void SystemClock_Config(void);

void uart_dma_rxtx_test()
{
	/* 通过DMA方式接收数据 */
	if (HAL_UART_Receive_DMA(&huart2, (uint8_t *)aRxBuffer, 12) != HAL_OK)
	{
		Error_Handler();
	}
	/* 等待接收数据完成 */
	while(HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY)
	{
	}

	/* 通过DMA方式发送数据 */
	if (HAL_UART_Transmit_DMA(&huart2, (uint8_t *)aRxBuffer, 12) != HAL_OK)
	{
		Error_Handler();
	}
	/* 等待发送数据完成 */
	while(HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY)
	{
	}
}


// uart_dma发包测试
void dmaPushPackage(uint8_t* pushBuf, uint16_t len)
{
	//打包发包测试
	// uint8_t len = 8;
	// uint8_t pushBuf[256] = {0X61,0X62,0x63,0x64,0x65,0x66,0x67,0x68};	// 请设置缓冲
		//uint8_t pushBuf[32] = {0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x21,0x22};
	
	PackageSendData((uint8_t *) pushBuf, &len);
	HAL_Delay(2);
	HAL_UART_Transmit(&huart2,(uint8_t *)pushBuf,len,0xFF);
	// HAL_Delay(2);
	//UART2_TX_DMA_Send((uint8_t *)pushBuf,len,0xFF);
	HAL_Delay(2);
}

// uart_dma收包测试
void dmaGetPackage(void)
{
	//dma_printf("%d\r\n", usart1_recv_end_flag);
	if(usart2_recv_end_flag == 1)
		{
			// 接收原始数据包测试
			dma_printf("rxlen=%d\r\n",usart2_rx_len);	// 打印原始数据包接收长度
HAL_Delay(10);
			HAL_UART_Transmit(&huart1,usart2_rx_buffer, usart2_rx_len, 0xFF);	// 接收原始数据包打印出来
HAL_Delay(10);
			UART2_TX_DMA_Send(usart2_rx_buffer, usart2_rx_len, 0xFF);
HAL_Delay(10);
			dma_printf("\r\n");
HAL_Delay(10);
			
			// 解包测试
			uint8_t temp_len = usart2_rx_len;
			uint8_t temp_buffer[256] = {0};
			memcpy(temp_buffer, usart2_rx_buffer, usart2_rx_len);
			
			if (UnpackReceivedData(temp_buffer, &temp_len) == 0){
				dma_printf("OK to Unpack\r\n");
HAL_Delay(15);
				dma_printf("temp_len=%d\r\n",temp_len);	// 打印解包后长度
HAL_Delay(10);
				for (int i=0; i<temp_len; i++)
				{
					dma_printf("UP%d=%X\r\n", i,temp_buffer[i]);
HAL_Delay(10);
				}
				HAL_Delay(10);
				HAL_UART_Transmit(&huart2,&temp_buffer[0], temp_len, 0xFF);	// 接收数据打印出来
				HAL_Delay(10);
				dma_printf("\r\n");
HAL_Delay(10);
			}
			else 
			{
				dma_printf("Unpack ERROR %d\r\n", UnpackReceivedData(temp_buffer, &temp_len));	// 打印解包错误信息
HAL_Delay(10);
			}
			
			dma_printf("\r\n");
			HAL_Delay(2);
			
			memset(&usart2_rx_buffer[0], 0, usart2_rx_len);
			usart2_rx_len = 0;	// 清除计数
			usart2_recv_end_flag = 0;	// 清除接收结束标志位
		}
		HAL_UART_Receive_DMA(&huart2,usart2_rx_buffer,sizeof(usart2_rx_buffer));	// 重新打开DMA接收
		
}

void dmaUartGetAppControl(shockPluse_t* shockPluse_s_p)
{
	  if(usart2_recv_end_flag == 1)
		{			
			uint8_t temp_len = usart2_rx_len;
			uint8_t temp_buffer[256] = {0};
			memcpy(temp_buffer, usart2_rx_buffer, usart2_rx_len);
			
			if (UnpackReceivedData(temp_buffer, &temp_len) == 0){
                shockParameterUnpacking(temp_buffer, temp_len, shockPluse_s_p);
                //HAL_Delay(2);
            }
            else 
            {
                dma_printf("get nothing\r\n");
        
            }
			memset(&usart2_rx_buffer[0], 0, usart2_rx_len);
			usart2_rx_len = 0;	// 清除计数
			usart2_recv_end_flag = 0;	// 清除接收结束标志位
		}
		HAL_UART_Receive_DMA(&huart2,usart2_rx_buffer,sizeof(usart2_rx_buffer));	// 重新打开DMA接收
}	

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  应用程序入口函数.
  * @retval int
  */
int main(void)
{
  /* systick初始化 */
  HAL_Init();
	
	SystemClock_Config();
	
	GPIO_Init();
	// DMA_Init();
	USART2_UART_Init();
    TIM1_Init();
    ADC1_Init();
    
	CRC_Init();
	
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);  //开启PWM通道2
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, 20);

  /* ADC校准 */
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)                          
    {
      Error_Handler();
    }
  
	shockPluse_t shockPluse_s;
	
	shockAllInit(&shockPluse_s);	// 初始化电刺激功能
    
    shockPluseSenseSet(&shockPluse_s);
	
		HAL_GPIO_WritePin(AT_STA_GPIO_Port, AT_STA_Pin, GPIO_PIN_SET);
		HAL_Delay(50);
		dma_printf("AT\r\n");
		HAL_Delay(50);
		dma_printf("AT+NAME=Shock001\r\n");
		HAL_Delay(50);
		HAL_GPIO_WritePin(AT_STA_GPIO_Port, AT_STA_Pin, GPIO_PIN_RESET);
		
  while (1)
  {
		/*
        uint8_t cnt = 6;
		uint8_t uartBUF1[16] = {0x00,0x01,0x00,0x00,0x00,0x00};
		dmaPushPackage(uartBUF1, cnt);
        HAL_Delay(20);
		uint8_t uartBUF2[8] = {0x00,0x0b,0x32,0x01,0xff,0x02};
		dmaPushPackage(uartBUF2, cnt);
        HAL_Delay(20);
		*/
		
		//dmaGetPackage();
		//HAL_Delay(2);
        
		
		dmaUartGetAppControl(&shockPluse_s);	// 第一步
		/*
			HAL_GPIO_WritePin(NET_P_GPIO_Port, NET_P_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(NET_N_GPIO_Port, NET_N_Pin, GPIO_PIN_RESET);
		HAL_Delay(1000);
			HAL_GPIO_WritePin(NET_P_GPIO_Port, NET_P_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(NET_N_GPIO_Port, NET_N_Pin, GPIO_PIN_SET);
		HAL_Delay(1000);
		*/
		
		//shockPluseSenseTest(&shockPluse_s);
        
		
		shockPulseSenseUnit(&shockPluse_s);	// 第二步
      
      //HAL_Delay(200);
      
		/*
        HAL_ADC_Start(&hadc1);     //启动ADC转换
		 HAL_ADC_PollForConversion(&hadc1, 100);   //等待转换完成，时间为50ms
		 if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC)){
				uint32_t GSR_ADC_Value = HAL_ADC_GetValue(&hadc1);   //获取AD值
				//dma_printf("ADC1读取:%d\r\n",GSR_ADC_Value);
                HAL_Delay(100);
				//dma_printf("PA3实时电压:%.4f\r\n",GSR_ADC_Value*3.3f/4096);
			 HAL_Delay(12);
		 }
         */    
        
		//HAL_Delay(200);
  }
}


/**
  * @brief   系统时钟配置函数
  * @param   无
  * @retval  无
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI \
                                   | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;  /* 配置HSE、HSI、LSI、LSE */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                             /* 开启HSI */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;	// 想试试满配置24m,8m,16m,24 allok. 高频率记得 FLASH_LATENCY_1提升闪存延时	/* HSI校准频率8MHz */
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                             /* HSI不分频 */
	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                            /* 关闭HSE */
  /* RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz; */                                  /* HSE频率范围16~32MHz */
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                                            /* 关闭LSI */
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                                            /* 关闭LSE */
  /* RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM; */                             /* 默认LSE驱动能力 */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;                                        /* 关闭PLL */
  /* RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_NONE; */                          /* PLL无时钟源 */
  /* 配置振荡器初始化 */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;/* 配置SYSCLK、HCLK、PCLK */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;                                        /* 配置系统时钟为HSI */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                            /* AHB时钟不分频 */	// 决定HCLK
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                             /* APB时钟不分频 */	// 决定PCLK
  /* 配置时钟源初始化 */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)	// 注意修改闪存访问延时，高频率需要高延时
  {
    Error_Handler();
  }
	//在调整系统时钟配置时，特别是提高时钟频率时，你可能需要调整闪存访问延时（FLASH_LATENCY），以确保系统稳定运行。上面的例子中使用了FLASH_LATENCY_0，适用于较低的时钟频率如8Mhz。
}

/**
  * @brief  错误执行函数
  * @param  无
  * @retval 无
  */
void Error_Handler(void)
{
  /* 无限循环 */
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  输出产生断言错误的源文件名及行号
  * @param  file：源文件名指针
  * @param  line：发生断言错误的行号
  * @retval 无
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* 用户可以根据需要添加自己的打印信息,
     例如: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* 无限循环 */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT AirM2M *****END OF FILE******************/
