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
volatile uint8_t usart2_recv_end_flag = 0;	//	��ʼ��Ϊδ����״̬

uint8_t usart2_tx_buffer[256] = {0};
volatile uint8_t usart2_tx_len = 0;
volatile uint8_t usart2_trans_end_flag = 1;	//	��ʼ��Ϊ����״̬

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

/* usart2 init function */

void USART2_UART_Init(void)
{
    /* USART2��ʼ�� */
  huart2.Instance          = USART2;
  huart2.Init.BaudRate     = 9600;
  huart2.Init.WordLength   = UART_WORDLENGTH_8B;
  huart2.Init.StopBits     = UART_STOPBITS_1;
  huart2.Init.Parity       = UART_PARITY_NONE;
  huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart2.Init.Mode         = UART_MODE_TX_RX;
  if (HAL_UART_Init(&huart2) != HAL_OK)	// ��ʱ����msp�� void HAL_UART_MspInit(UART_HandleTypeDef *huart)
  {
    Error_Handler();
  }

	//ʹ���շ��жϣ�������������������ôʹ���գ�����
	//HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer[0], sizeof(usart2_rx_buffer));
	//HAL_UART_Transmit_IT(&huart2, &usart2_tx_buffer[0], sizeof(usart2_tx_buffer));
	//���ʹ��idle�жϺʹ򿪴���DMA�������
	
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//ʹ��idle�ж�,�������н����ж�
	HAL_UART_Receive_DMA(&huart2,usart2_rx_buffer,sizeof(usart2_rx_buffer));//��DMA���գ����ݴ���rx_buffer�����У�ָ�����ջ������ͽ��մ�С

}

/**
  * @brief ��ʼ��USART��MSP
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  /* ʹ��ʱ�� */
  __HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_DMA_CLK_ENABLE();
  
  /**USART2 ��������
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

  /* USART2 DMA������� */
  /* USART2_TX ��ʼ�� */
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
	
  /* USART2_RX ��ʼ�� */
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
  /* ʹ��NVIC */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 1);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

//����1��DMA���ͣ���Ҫ�ȵȴ���һ֡���ݷ�������ٷ�����һ֡���ݣ���ε��������ʱ�ȴ����ѯdma�������

/*
void UART2_TX_DMA_Send(uint8_t *buffer, uint16_t length, uint32_t Timeout)	// �ɰ���������
{
    //�ȴ���һ�ε����ݷ������
	while(HAL_DMA_GetState(&hdma_usart2_tx) != HAL_DMA_STATE_READY);
    //while(__HAL_DMA_GET_COUNTER(&hdma_usart2_tx));
	
    //�ر�DMA
    __HAL_DMA_DISABLE(&hdma_usart2_tx);

    //��ʼ��������
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

void UART2_TX_DMA_Send(uint8_t *buffer, uint16_t length, uint32_t Timeout)	// �Ż����һ����
{
	// ����������
	if (buffer == NULL || length == 0) {
			return; // ������Ч��ֱ�ӷ���
	}

	// �ȴ���һ�ε�DMA������ɣ�ʹ�÷�������ʽ
	uint32_t timeout = HAL_GetTick() + Timeout; // ���ó�ʱʱ��ΪTimeout(��λms)
	
	while (!usart2_trans_end_flag) {
		// �ٴμ��
		if (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) && (HAL_UART_GetState(&huart2) == HAL_UART_STATE_READY || __HAL_DMA_GET_COUNTER(&hdma_usart2_tx) == 0) )	// ���DMA��δ��������ݸ����Ƿ�Ϊ0
		{
			//ǿ�ƽ����ϴκͱ��η���
			// ��F7ϵ���ǿ��Բ�д�ģ�F1����д
			__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC1); //���DMA2_Steam7������ɱ�־
			HAL_UART_DMAStop(&huart2);		//��������Ժ�رմ���DMA,ȱ����һ�������
			usart2_trans_end_flag = 1;
		}
		else if (HAL_GetTick() > timeout) {
			
			// ��ʱ�������Ը�����Ҫ�����־��ӡ�����ص�
			printf("DMA transmit timeout.\n");
			
			//ǿ�ƽ����ϴκͱ��η���
			// ��F7ϵ���ǿ��Բ�д�ģ�F1����д
			//__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC4); //���DMA2_Steam7������ɱ�־
			HAL_UART_DMAStop(&huart2);		//��������Ժ�رմ���DMA,ȱ����һ�������
			usart2_trans_end_flag = 1;
			
			// return;	// ��ֹ���η���
		}
	}
	usart2_trans_end_flag = 0; // �����ɱ�־��׼������
	
	//�ر�DMA
    __HAL_DMA_DISABLE(&hdma_usart2_tx);

	// �����µ�DMA����
	if (HAL_UART_Transmit_DMA(&huart2, buffer, length) != HAL_OK) {
		// DMA��������ʧ�ܴ���
		usart2_trans_end_flag = 1; // �ָ���־λ������������һ�δ���
		// ������Ҫ��Ӵ�������룬����־��¼�����ص�
		printf("Failed to start DMA transmit.\n");
	}
	
	    
    //while(HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY);	// �ȴ������������
	//while(__HAL_DMA_GET_COUNTER(&hdma_usart2_tx) != 0);	// �򿪿��ܻ���ѭ��������
	if (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) && (HAL_UART_GetState(&huart2) == HAL_UART_STATE_READY || __HAL_DMA_GET_COUNTER(&hdma_usart2_tx) == 0) )// ���DMA��δ��������ݸ����Ƿ�Ϊ0
	{
		// ��F7ϵ���ǿ��Բ�д�ģ�F1����д
		__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC1); //���DMA2_Steam7������ɱ�־
		HAL_UART_DMAStop(&huart2);		//��������Ժ�رմ���DMA,ȱ����һ�������
		usart2_trans_end_flag = 1;
		return;
	}
}


// ע���	#define NRF_MODULE_ENABLED(APP_UART)	�궨��
//����1��DMA����printf
void dma_printf(const char *format, ...)
{
	uint32_t length = 0;
	va_list args;
	
	va_start(args, format);
	
	length = vsnprintf((char*)usart2_tx_buffer, sizeof(usart2_tx_buffer), (char*)format, args);
	
	UART2_TX_DMA_Send(usart2_tx_buffer, length, 0x04ff);
		//	vprintf(format, args);	// ʾ����
	
	va_end(args);
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) //����Ǵ���1
	{

			if (__HAL_DMA_GET_COUNTER(&hdma_usart2_tx) == 0)// ���DMA��δ��������ݸ����Ƿ�Ϊ0
			{
				// ��F7ϵ���ǿ��Բ�д�ģ�F1����д
				__HAL_DMA_CLEAR_FLAG(&hdma_usart2_tx, DMA_FLAG_TC1); //���DMA2_Steam7������ɱ�־
				HAL_UART_DMAStop(&huart2);		//��������Ժ�رմ���DMA,ȱ����һ�������
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


// ԭ�����ӣ�https://blog.csdn.net/qq_30267617/article/details/118877845


/**
  * ��������: �ض���c�⺯��printf��DEBUG_USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
// int fputc(int ch, FILE *f){  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xffff);  return ch;}
 
/**
  * ��������: �ض���c�⺯��getchar,scanf��DEBUG_USARTx
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
// int fgetc(FILE *f){  uint8_t ch = 0;  HAL_UART_Receive(&huart2, &ch, 1, 0xffff);  return ch;}

// ���ڲ��ָ���https://blog.csdn.net/GQ_Sonofgod/article/details/118335944

// CRC16MODBUSУ�麯��
uint16_t CheckSumCrc16(uint8_t *ptr,uint8_t len)
{
	unsigned long wcrc=0XFFFF;//Ԥ��16λcrc�Ĵ�������ֵȫ��Ϊ1
	int i=0,j=0;//�������
	for(i=0;i<len;i++)//ѭ������ÿ������
	{
		wcrc^=*ptr++;//����λ������crc�Ĵ������.ָ���ַ���ӣ�ָ���¸�����
		for(j=0;j<8;j++)//ѭ���������ݵ�
		{
			if(wcrc&0X0001)//�ж����Ƴ����ǲ���1�������1�������ʽ�������
			{
				wcrc=wcrc>>1^0XA001;//�Ƚ���������һλ��������Ķ���ʽ�������
			}
			else//�������1����ֱ���Ƴ�
			{
				wcrc>>=1;//ֱ���Ƴ�
			}
		}
	}
	return wcrc<<8|wcrc>>8;//�Ͱ�λ��ǰ���߰�λ�ں�
}


#ifndef CRC16MODBUS
#define CRC16MODBUS CheckSumCrc16

uint8_t FrameHeader[] = {0x01, 0x02, 0x03}; // ʾ����ͷ
uint8_t FrameEnd[] = {0x07, 0x08, 0x09}; // ʾ����β
uint8_t FrameHeaderSize = sizeof(FrameHeader) / sizeof(FrameHeader[0]);
uint8_t FrameEndSize = sizeof(FrameEnd) / sizeof(FrameEnd[0]);

/***********************************************************************************************************
* ��������: PackageSendData()
* �������: data,Ҫ���͵����ݵ��׵�ַ�� *len,���͵�ԭʼ�������ݵ��ܳ���
* ����ֵ  : ��
* ��    ��: ����ͨ��Э�飬��װҪ���͵�����
************************************************************************************************************/
void PackageSendData(uint8_t* data, uint16_t* len)
{
  uint8_t temp[*len+FrameHeaderSize+FrameEndSize+3];
  uint16_t crc;
  
  memset(&temp[0], 0, sizeof(temp));
	
  memcpy(&temp[0], FrameHeader, FrameHeaderSize);	// ����֡ͷ
	
  memcpy(&temp[FrameHeaderSize+1], &data[0], *len); //	���ƴ�������
	
  temp[FrameHeaderSize] = *len; //	���볤��ֵ,����ʵ���ݵĳ���
	
  crc = CRC16MODBUS(&temp[FrameHeaderSize], *len+1); //	���� (����λ+����λ) ��CRC16У��ֵ
  memcpy(&temp[*len+FrameHeaderSize+1], &crc, 2); //	����У��ֵ
	
	memcpy(&temp[*len+FrameHeaderSize+3], FrameEnd, FrameEndSize);	// ���ư�β
	
  memcpy(&data[0], &temp[0], *len+FrameHeaderSize+FrameEndSize+3);	// ��ֵ������
	
  *len = *len+FrameHeaderSize+FrameEndSize+3;	// ��ֵ�������ĳ���
}

/*
	//UART_Transmit test BEGIN �������ݷ��Ͳ���
	uint8_t cnt;
	cnt = 5;
	uint8_t uart2BUF[32] = {0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x21,0x22};	// �����û���
	PackageSendData((uint8_t *) uart2BUF, &cnt);
	HAL_UART_Transmit(&huart2,(uint8_t *)uart2BUF,cnt,0XFF);
		// UART_Transmit test END
*/

/***********************************************************************************************************
* ��������: UnpackReceivedData()
* �������: data,Ҫ��������ݵ��׵�ַ��*len Ҫ�����ԭʼ���ݵ��ܳ��ȣ��������еİ��������
* ����ֵ  : 0�� ������Ч�� ������������Ч��
* ��    ��: ������յ������ݲ��ж�������Ч��
************************************************************************************************************/

// �ٶ�CRC16MODBUS������FrameHeader��FrameEnd�Ķ����Ѿ�����

// �������
// data: ���յ�����������
// totalLen: ָ��������ݳ��ȵ�ָ��

/*
uint8_t UnpackReceivedData(uint8_t* data, uint8_t* totalLen) {
    uint16_t crc, crctemp;
    uint8_t len;
    uint32_t headerStartIndex = -1;
    uint8_t i, j;
	
		// ������ͷ
    for (i = 0; i <= *totalLen - FrameHeaderSize; i++) {
        // ���Ǳ�ڵİ�ͷ
        for (j = 0; j < FrameHeaderSize; j++) {
            if (data[i + j] != FrameHeader[j]) {
                break; // ����κ�һ���ֽڲ�ƥ�䣬����ֹͣ�ڲ�ѭ��
            }
        }
        if (j == FrameHeaderSize) { // ����ҵ�Ǳ�ڵİ�ͷ
            // ��ʱ�����ҵ��˰�ͷ�����Զ�ȡ����λ����֤��β
            len = data[i + FrameHeaderSize]; // ԭʼ���ݳ���

            // ��֤��βǰ��ȷ���ܳ����㹻
            if (i + FrameHeaderSize + 1 + len + 2 + FrameEndSize <= *totalLen) {
                // ����β
                uint8_t endPtr = i + FrameHeaderSize + 1 + len + 2; // ��β��ʼλ��
                for (j = 0; j < FrameEndSize; j++) {
									if (data[endPtr + j] != FrameEnd[j]) {
											break; // ��β��ƥ��
									}
                }
                if (j == FrameEndSize) { // ��βƥ��
									
									headerStartIndex = i; // ȷ���ҵ��������İ�ͷ
									
									//
									// ���ƽ��������е�CRCУ��ֵ
									memcpy(&crctemp, data + headerStartIndex + FrameHeaderSize + 1 + len, 2);

									// ����CRC
									crc = CRC16MODBUS(data + headerStartIndex + FrameHeaderSize, len + 1);

									if (crctemp == crc) {
										// CRCУ��ɹ����޳���ͷǰ���޹����ݺͰ�ͷ
									
										for (i = 0; i < len; i++) {	// ��ֵ����������
												data[i] = data[headerStartIndex + FrameHeaderSize + 1 + i];
										}
										memset(&data[len], 0, *totalLen - len); // ���ʣ�ಿ��

										*totalLen = len; // ���³���Ϊʵ�����ݳ���
										return 0; // У��ɹ�
									}
									else {
										return 1; // CRCУ��ʧ��
									}
									//
										
									break; // ֹͣ����
                }
                // �����β��ƥ�䣬�����������һ��Ǳ�ڵİ�ͷ
            }
        }
    }
    if (headerStartIndex == -1) {
        // printf("Header not found\r\n");
        return 2; // �Ҳ�����ͷ������ƥ��İ�β
    }
		else return 9;	// δ֪����
}
*/

uint8_t UnpackReceivedData(uint8_t* data, uint8_t* totalLen) {
    uint16_t crc, crctemp;
    uint8_t len;
    uint32_t headerStartIndex = -1;
    uint8_t i, j;

    // ������ͷ
    for (i = 0; i <= *totalLen - FrameHeaderSize; i++) {
        for (j = 0; j < FrameHeaderSize; j++) {
            if (data[i + j] != FrameHeader[j]) {
                break; // ����κ�һ���ֽڲ�ƥ�䣬����ֹͣ�ڲ�ѭ��
            }
            if (j == FrameHeaderSize - 1) {
                headerStartIndex = i; // �ҵ���ͷ����ʼλ��
            }
        }
        if (headerStartIndex != -1) {
            break; // �ҵ���ͷ��ֹͣ����
        }
    }

    if (headerStartIndex == -1) {
        printf("Header not found\r\n");
        return 2; // �Ҳ�����ͷ
    }
		
    // ���ҵ��İ�ͷλ�ÿ�ʼ��ȡ����λ
    len = data[headerStartIndex + FrameHeaderSize]; // ԭʼ���ݳ���

    // ����ܳ����Ƿ��㹻
    if (*totalLen < headerStartIndex + FrameHeaderSize + 1 + len + 2 + FrameEndSize) {
        return 3; // ���ݳ��Ȳ���
    }
		
		// ���ݳ���λ��λ��β����ʼλ��
    // ��β��ʼλ�� = ��ͷ��С + ����λ(1) + ���ݳ��� + CRCУ���볤��(2)
    uint8_t endPtr = headerStartIndex + FrameHeaderSize + 1 + len + 2;

    // ��֤��β
    for (i = 0; i < FrameEndSize; ++i) {
        if (data[endPtr + i] != FrameEnd[i]) {
            printf("End Mismatch at byte %d: %X\r\n", i, data[endPtr + i]);
            return 4; // ��β��ƥ��
        }
    }

    // ���ƽ��������е�CRCУ��ֵ
    memcpy(&crctemp, data + headerStartIndex + FrameHeaderSize + 1 + len, 2);

    // ����CRC
    crc = CRC16MODBUS(data + headerStartIndex + FrameHeaderSize, len + 1);

    if (crctemp == crc) {
        // CRCУ��ɹ����޳���ͷǰ���޹����ݺͰ�ͷ
        for (i = 0; i < len; i++) {	// ��ֵ����������
            data[i] = data[headerStartIndex + FrameHeaderSize + 1 + i];
            dma_printf("P%d=%d\r\n", i,data[i]);
            HAL_Delay(10);
        }
        memset(&data[len], 0, *totalLen - len); // ���ʣ�ಿ��

        *totalLen = len; // ���³���Ϊʵ�����ݳ���
        return 0; // У��ɹ�
    } else {
        return 1; // CRCУ��ʧ��
    }
		
}

#endif



/* USER CODE END 1 */
