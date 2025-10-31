#include "air001_ll_Core_Board.h"

/**
 * @brief AIR001 Core Board BSP Driver version number
 */
#define __AIR001_CORE_BOARD_BSP_VERSION_MAIN (0x01U) /*!< [31:24] main version */
#define __AIR001_CORE_BOARD_BSP_VERSION_SUB1 (0x00U) /*!< [23:16] sub1 version */
#define __AIR001_CORE_BOARD_BSP_VERSION_SUB2 (0x00U) /*!< [15:8]  sub2 version */
#define __AIR001_CORE_BOARD_BSP_VERSION_RC (0x00U)   /*!< [7:0]  release candidate */
#define __AIR001_CORE_BOARD_BSP_VERSION ((__AIR001_CORE_BOARD_BSP_VERSION_MAIN << 24) | (__AIR001_CORE_BOARD_BSP_VERSION_SUB1 << 16) | (__AIR001_CORE_BOARD_BSP_VERSION_SUB2 << 8) | (__AIR001_CORE_BOARD_BSP_VERSION_RC))

GPIO_TypeDef *LED_PORT[LEDn] = {LED1_GPIO_PORT};
const uint16_t LED_PIN[LEDn] = {LED1_PIN};

GPIO_TypeDef *BUTTON_PORT[BUTTONn] = {USER_BUTTON_GPIO_PORT};
const uint16_t BUTTON_PIN[BUTTONn] = {USER_BUTTON_PIN};
const uint8_t BUTTON_IRQn[BUTTONn] = {USER_BUTTON_EXTI_IRQn};
const uint32_t BUTTON_EXTI_LINE[BUTTONn] = {USER_BUTTON_EXTI_LINE};

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
  /* Enable the GPIO_LED Clock */
  LEDx_GPIO_CLK_ENABLE(Led);

  /* Configure the GPIO_LED pin */
  LL_GPIO_SetPinMode(LED_PORT[Led], LED_PIN[Led], LL_GPIO_MODE_OUTPUT);
  /* LL_GPIO_SetPinOutputType(LED_PORT[Led], LED_PIN[Led], LL_GPIO_OUTPUT_PUSHPULL); */
  /* LL_GPIO_SetPinSpeed(LED_PORT[Led], LED_PIN[Led], LL_GPIO_SPEED_FREQ_LOW);       */
  /* LL_GPIO_SetPinPull(LED_PORT[Led], LED_PIN[Led], LL_GPIO_PULL_NO);               */

  LL_GPIO_SetOutputPin(LED_PORT[Led], LED_PIN[Led]);
  LL_GPIO_ResetOutputPin(LED_PORT[Led], LED_PIN[Led]);
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
  /* Turn off LED */
  LL_GPIO_ResetOutputPin(LED_PORT[Led], LED_PIN[Led]);
  /* DeInit the GPIO_LED pin */
  LL_GPIO_SetPinMode(LED_PORT[Led], LED_PIN[Led], LL_GPIO_MODE_ANALOG);
  /* LL_GPIO_SetPinOutputType(LED_PORT[Led], LED_PIN[Led], LL_GPIO_OUTPUT_PUSHPULL); */
  /* LL_GPIO_SetPinSpeed(LED_PORT[Led], LED_PIN[Led], LL_GPIO_SPEED_FREQ_LOW);       */
  /* LL_GPIO_SetPinPull(LED_PORT[Led], LED_PIN[Led], LL_GPIO_PULL_NO);               */
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
  LL_GPIO_SetOutputPin(LED_PORT[Led], LED_PIN[Led]);
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
  LL_GPIO_ResetOutputPin(LED_PORT[Led], LED_PIN[Led]);
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
  LL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);
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
  /* Enable the BUTTON Clock */
  BUTTONx_GPIO_CLK_ENABLE(Button);

  /* Configure GPIO for BUTTON */
  LL_GPIO_SetPinMode(BUTTON_PORT[Button], BUTTON_PIN[Button], LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(BUTTON_PORT[Button], BUTTON_PIN[Button], LL_GPIO_PULL_NO);
  /* LL_GPIO_SetPinSpeed(BUTTON_PORT[Button], BUTTON_PIN[Button], LL_GPIO_SPEED_FREQ_HIGH); */

  if (ButtonMode == BUTTON_MODE_EXTI)
  {
    /* Configure Button pin as input with External interrupt */
    LL_EXTI_EnableIT(BUTTON_EXTI_LINE[Button]);
    LL_EXTI_EnableFallingTrig(BUTTON_EXTI_LINE[Button]);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    NVIC_SetPriority((IRQn_Type)(BUTTON_IRQn[Button]), 0x0F);
    NVIC_EnableIRQ((IRQn_Type)(BUTTON_IRQn[Button]));
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
  NVIC_DisableIRQ((IRQn_Type)(BUTTON_IRQn[Button]));
  LL_GPIO_SetPinMode(BUTTON_PORT[Button], BUTTON_PIN[Button], LL_GPIO_MODE_ANALOG);
  /* LL_GPIO_SetPinSpeed(BUTTON_PORT[Button], BUTTON_PIN[Button], LL_GPIO_SPEED_FREQ_LOW); */
  /* LL_GPIO_SetPinPull(BUTTON_PORT[Button], BUTTON_PIN[Button], LL_GPIO_PULL_NO);         */
  /* LL_GPIO_SetAFPin_8_15(BUTTON_PORT[Button], BUTTON_PIN[Button], LL_GPIO_AF_0);         */
}

/**
 * @brief 返回选定的Button状态。
 *@param Button： 指定要检查的Button。
 *这个参数应该是： BUTTON_USER
 * @retval Button state.
 */
uint32_t BSP_PB_GetState(Button_TypeDef Button)
{
  return LL_GPIO_IsInputPinSet(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}

/**
 * @brief DEBUG_USART GPIO配置,模式配置,115200 8-N-1
 * @param None
 * @retval 无
 */
void BSP_USART_Config(void)
{
  DEBUG_USART_CLK_ENABLE();

  /* USART Init */
  LL_USART_SetBaudRate(DEBUG_USART, SystemCoreClock, LL_USART_OVERSAMPLING_16, DEBUG_USART_BAUDRATE);
  LL_USART_SetDataWidth(DEBUG_USART, LL_USART_DATAWIDTH_8B);
  LL_USART_SetStopBitsLength(DEBUG_USART, LL_USART_STOPBITS_1);
  LL_USART_SetParity(DEBUG_USART, LL_USART_PARITY_NONE);
  LL_USART_SetHWFlowCtrl(DEBUG_USART, LL_USART_HWCONTROL_NONE);
  LL_USART_SetTransferDirection(DEBUG_USART, LL_USART_DIRECTION_TX_RX);
  LL_USART_Enable(DEBUG_USART);
  LL_USART_ClearFlag_TC(DEBUG_USART);

  /**USART GPIO Configuration
    PA2     ------> USART1_TX
    PA3     ------> USART1_RX
    */
  DEBUG_USART_RX_GPIO_CLK_ENABLE();
  DEBUG_USART_TX_GPIO_CLK_ENABLE();

  LL_GPIO_SetPinMode(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinSpeed(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinPull(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, LL_GPIO_PULL_UP);
  LL_GPIO_SetAFPin_0_7(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, DEBUG_USART_TX_AF);

  LL_GPIO_SetPinMode(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinSpeed(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinPull(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_PULL_UP);
  LL_GPIO_SetAFPin_0_7(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, DEBUG_USART_RX_AF);
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
  LL_USART_TransmitData8(DEBUG_USART, ch);
  while (!LL_USART_IsActiveFlag_TC(DEBUG_USART))
    ;
  LL_USART_ClearFlag_TC(DEBUG_USART);

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
  while (!LL_USART_IsActiveFlag_RXNE(DEBUG_USART))
    ;
  ch = LL_USART_ReceiveData8(DEBUG_USART);
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
  LL_USART_TransmitData8(DEBUG_USART, ch);
  while (!LL_USART_IsActiveFlag_TC(DEBUG_USART))
    ;
  LL_USART_ClearFlag_TC(DEBUG_USART);

  return (ch);
}
#endif