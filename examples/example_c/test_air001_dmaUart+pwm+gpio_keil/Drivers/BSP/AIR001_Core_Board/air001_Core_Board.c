#include "air001_Core_Board.h"

#define __AIR001_CORE_BOARD_BSP_VERSION_MAIN (0x01U) /*!< [31:24] main version */
#define __AIR001_CORE_BOARD_BSP_VERSION_SUB1 (0x00U) /*!< [23:16] sub1 version */
#define __AIR001_CORE_BOARD_BSP_VERSION_SUB2 (0x00U) /*!< [15:8]  sub2 version */
#define __AIR001_CORE_BOARD_BSP_VERSION_RC (0x00U)   /*!< [7:0]  release candidate */
#define __AIR001_CORE_BOARD_BSP_VERSION ((__AIR001_CORE_BOARD_BSP_VERSION_MAIN << 24) | (__AIR001_CORE_BOARD_BSP_VERSION_SUB1 << 16) | (__AIR001_CORE_BOARD_BSP_VERSION_SUB2 << 8) | (__AIR001_CORE_BOARD_BSP_VERSION_RC))

#ifdef HAL_UART_MODULE_ENABLED
UART_HandleTypeDef DebugUartHandle;
#endif
GPIO_TypeDef *LED_PORT[LEDn] = {LED1_GPIO_PORT};
const uint16_t LED_PIN[LEDn] = {LED1_PIN};

GPIO_TypeDef *BUTTON_PORT[BUTTONn] = {USER_BUTTON_GPIO_PORT};
const uint16_t BUTTON_PIN[BUTTONn] = {USER_BUTTON_PIN};
const uint8_t BUTTON_IRQn[BUTTONn] = {USER_BUTTON_EXTI_IRQn};

/** @addtogroup AIR001_CORE_BOARD_Exported_Functions
 * @{
 */

/**
 * @brief 该方法返回AIR001 CORE BOARD BSP Driver的版本。
 * @retval version : 0xXYZR (8bits for each decimal, R for RC)
 */
uint32_t BSP_GetVersion(void)
{
  return __AIR001_CORE_BOARD_BSP_VERSION;
}

/** @addtogroup LED_Functions
 * @{
 */

/**
 * @brief 配置LED的GPIO。
 * @param Led 指定要配置的LED。
 * 这个参数可以是以下的参数之一：
 * @arg LED1
 * @retval 无
 */
void BSP_LED_Init(Led_TypeDef Led)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Enable the GPIO_LED Clock */
  LEDx_GPIO_CLK_ENABLE(Led);

  /* Configure the GPIO_LED pin */
  GPIO_InitStruct.Pin = LED_PIN[Led];
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(LED_PORT[Led], &GPIO_InitStruct);
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
}

/**
 * @brief DeInitialize LED GPIO.
 * @param Led 指定要去配置的LED。
 * 这个参数可以是以下值之一：
 * @arg LED1
 * @注意 BSP_LED_DeInit()不会禁用GPIO时钟。
 * @retval 无
 */
void BSP_LED_DeInit(Led_TypeDef Led)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Turn off LED */
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
  /* DeInit the GPIO_LED pin */
  GPIO_InitStruct.Pin = LED_PIN[Led];
  HAL_GPIO_DeInit(LED_PORT[Led], GPIO_InitStruct.Pin);
}

/**
 * @brief 将选定的LED打开。
 * @param Led 指定要设置的LED。
 * 这个参数可以是以下的参数之一：
 * @arg LED1
 * @retval 无
 */
void BSP_LED_On(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET);
}

/**
 * @brief 将选定的LED关闭。
 * @param Led 指定要关闭的LED。
 * 这个参数可以是下列参数之一：
 * @arg LED1
 * @retval 无
 */
void BSP_LED_Off(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
}

/**
 * @brief 切换选定的LED电平。
 * @param Led 指定要切换的LED。
 * 这个参数可以是下列参数之一：
 * @arg LED1
 * @retval 无
 */
void BSP_LED_Toggle(Led_TypeDef Led)
{
  HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);
}

/**
 * @brief 配置按钮的GPIO和EXTI线。
 * @param Button： 指定要配置的按钮。
 *这个参数应该是： BUTTON_USER
 * @param ButtonMode： 指定按钮模式。
 * 这个参数可以是以下的参数之一：
 * @arg BUTTON_MODE_GPIO: 按钮将被用作普通IO。
 * @arg BUTTON_MODE_EXTI: 按钮将被连接到带有中断的EXTI线。
 * @retval 无
 */
void BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode)
{
  GPIO_InitTypeDef gpioinitstruct;

  /* Enable the BUTTON Clock */
  BUTTONx_GPIO_CLK_ENABLE(Button);

  gpioinitstruct.Pin = BUTTON_PIN[Button];
  gpioinitstruct.Pull = GPIO_NOPULL;
  gpioinitstruct.Speed = GPIO_SPEED_FREQ_MEDIUM;

  if (ButtonMode == BUTTON_MODE_GPIO)
  {
    /* Configure Button pin as input */
    gpioinitstruct.Mode = GPIO_MODE_INPUT;

    HAL_GPIO_Init(BUTTON_PORT[Button], &gpioinitstruct);
  }

  if (ButtonMode == BUTTON_MODE_EXTI)
  {
    /* 配置为上升沿中断 */
    gpioinitstruct.Mode = GPIO_MODE_IT_RISING;
    HAL_GPIO_Init(BUTTON_PORT[Button], &gpioinitstruct);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority((IRQn_Type)(BUTTON_IRQn[Button]), 0x0F, 0);
    HAL_NVIC_EnableIRQ((IRQn_Type)(BUTTON_IRQn[Button]));
  }
}

/**
 * @brief Push Button DeInit.
 *@param Button： 要配置的按钮
 *这个参数应该是： BUTTON_USER
 * @注意 PB DeInit不会禁用GPIO时钟
 * @retval 无
 */
void BSP_PB_DeInit(Button_TypeDef Button)
{
  GPIO_InitTypeDef gpio_init_structure;

  gpio_init_structure.Pin = BUTTON_PIN[Button];
  HAL_NVIC_DisableIRQ((IRQn_Type)(BUTTON_IRQn[Button]));
  HAL_GPIO_DeInit(BUTTON_PORT[Button], gpio_init_structure.Pin);
}

/**
 * @brief 返回选定的Button状态。
 *@param Button： 指定要检查的Button。
 *这个参数应该是： BUTTON_USER
 * @retval Button state.
 */
uint32_t BSP_PB_GetState(Button_TypeDef Button)
{
  return HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}

#ifdef HAL_UART_MODULE_ENABLED
/**
 * @brief DEBUG_USART GPIO配置,模式配置,115200 8-N-1
 * @param None
 * @retval 无
 */
void BSP_USART_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  DEBUG_USART_CLK_ENABLE();

  DebugUartHandle.Instance = DEBUG_USART;

  DebugUartHandle.Init.BaudRate = DEBUG_USART_BAUDRATE;
  DebugUartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  DebugUartHandle.Init.StopBits = UART_STOPBITS_1;
  DebugUartHandle.Init.Parity = UART_PARITY_NONE;
  DebugUartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  DebugUartHandle.Init.Mode = UART_MODE_TX_RX;

  HAL_UART_Init(&DebugUartHandle);

  DEBUG_USART_RX_GPIO_CLK_ENABLE();
  DEBUG_USART_TX_GPIO_CLK_ENABLE();

  /**USART GPIO Configuration
    PA2     ------> USART1_TX
    PA3     ------> USART1_RX
    */
  GPIO_InitStruct.Pin = DEBUG_USART_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = DEBUG_USART_TX_AF;
  HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DEBUG_USART_RX_PIN;
  GPIO_InitStruct.Alternate = DEBUG_USART_RX_AF;

  HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct);

  /* ENABLE NVIC */
  HAL_NVIC_SetPriority(DEBUG_USART_IRQ, 0, 1);
  HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);
}

#if (defined(__CC_ARM)) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
/**
 * @brief 写一个字符到usart中。
 *@param ch
 * *f
 *@retval 该字符
 */
int fputc(int ch, FILE *f)
{
  /* Send a byte to USART */
  HAL_UART_Transmit(&DebugUartHandle, (uint8_t *)&ch, 1, 1000);

  return (ch);
}

/**
 *@brief 从usart中获取一个字符
 *@param *f
 *@retval 一个字符
 */
int fgetc(FILE *f)
{
  int ch;
  HAL_UART_Receive(&DebugUartHandle, (uint8_t *)&ch, 1, 1000);
  return (ch);
}

#elif defined(__ICCARM__)
/**
 * @brief 写一个字符到usart中。
 *@param ch
 * *f
 *@retval 该字符
 */
int putchar(int ch)
{
  /* Send a byte to USART */
  HAL_UART_Transmit(&DebugUartHandle, (uint8_t *)&ch, 1, 1000);

  return (ch);
}
#endif

#endif
