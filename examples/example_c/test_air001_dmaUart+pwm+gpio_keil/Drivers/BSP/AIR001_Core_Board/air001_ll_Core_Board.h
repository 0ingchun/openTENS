#ifndef AIR001_LL_CORE_BOARD_H
#define AIR001_LL_CORE_BOARD_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "air001xx_ll_rcc.h"
#include "air001xx_ll_bus.h"
#include "air001xx_ll_system.h"
#include "air001xx_ll_exti.h"
#include "air001xx_ll_cortex.h"
#include "air001xx_ll_utils.h"
#include "air001xx_ll_pwr.h"
#include "air001xx_ll_dma.h"
#include "air001xx_ll_gpio.h"
#include "air001xx_ll_usart.h"

  /** @addtogroup BSP
   * @{
   */

  /** @defgroup air001_Core_Board
   * @brief 本节包含导出的类型、内容和函数
   * 使用Air001 Core Board 板所需的类型和函数。
   * @{
   */

  /** @defgroup air001_Core_Board_Exported_Types 出口的类型
   * @{
   */
  typedef enum
  {
    LED1 = 0,
    LED_RED = LED1
  } Led_TypeDef;

  typedef enum
  {
    BUTTON_USER = 0,
    /* Alias */
    BUTTON_KEY = BUTTON_USER
  } Button_TypeDef;

  typedef enum
  {
    BUTTON_MODE_GPIO = 0,
    BUTTON_MODE_EXTI = 1
  } ButtonMode_TypeDef;

#define LEDn 1

#define LED1_PIN LL_GPIO_PIN_0
#define LED1_GPIO_PORT GPIOB
#define LED1_GPIO_CLK_ENABLE() LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB)
#define LED1_GPIO_CLK_DISABLE() LL_IOP_GRP1_DisableClock(LL_IOP_GRP1_PERIPH_GPIOB)

#define LEDx_GPIO_CLK_ENABLE(__INDEX__) \
  do                                    \
  {                                     \
    LED1_GPIO_CLK_ENABLE();             \
  } while (0U)
#define LEDx_GPIO_CLK_DISABLE(__INDEX__)   LED1_GPIO_CLK_DISABLE())

#define BUTTONn 1

/**
 * @brief User push-button
 */
#define USER_BUTTON_PIN LL_GPIO_PIN_4
#define USER_BUTTON_GPIO_PORT GPIOF
#define USER_BUTTON_GPIO_CLK_ENABLE() LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOF)
#define USER_BUTTON_GPIO_CLK_DISABLE() LL_IOP_GRP1_DisableClock(LL_IOP_GRP1_PERIPH_GPIOF)
#define USER_BUTTON_EXTI_IRQn EXTI4_15_IRQn
#define USER_BUTTON_EXTI_LINE LL_EXTI_LINE_4
#define USER_BUTTON_EXTI_LINE_ENABLE() LL_EXTI_EnableIT(USER_BUTTON_EXTI_LINE)
#define USER_BUTTON_EXTI_FALLING_TRIG_ENABLE() LL_EXTI_EnableFallingTrig(USER_BUTTON_EXTI_LINE)
#define USER_BUTTON_IRQHANDLER EXTI4_15_IRQHandler

/* Aliases */
#define KEY_BUTTON_PIN USER_BUTTON_PIN
#define KEY_BUTTON_GPIO_PORT USER_BUTTON_GPIO_PORT
#define KEY_BUTTON_GPIO_CLK_ENABLE() USER_BUTTON_GPIO_CLK_ENABLE()
#define KEY_BUTTON_GPIO_CLK_DISABLE() USER_BUTTON_GPIO_CLK_DISABLE()
#define KEY_BUTTON_EXTI_IRQn USER_BUTTON_EXTI_IRQn

#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__) \
  do                                       \
  {                                        \
    if ((__INDEX__) == 0)                  \
      USER_BUTTON_GPIO_CLK_ENABLE();       \
  } while (0)
#define BUTTONx_GPIO_CLK_DISABLE(__INDEX__) (((__INDEX__) == 0) ? USER_BUTTON_GPIO_CLK_DISABLE() : 0)

// debug printf redirect config
#define DEBUG_USART_BAUDRATE 115200

#define DEBUG_USART USART2
#define DEBUG_USART_CLK_ENABLE() LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2)

#define __GPIOA_CLK_ENABLE()                            \
  do                                                    \
  {                                                     \
    __IO uint32_t tmpreg = 0x00U;                       \
    SET_BIT(RCC->IOPENR, RCC_IOPENR_GPIOAEN);           \
    /* Delay after an RCC peripheral clock enabling */  \
    tmpreg = READ_BIT(RCC->IOPENR, RCC_IOPENR_GPIOAEN); \
    UNUSED(tmpreg);                                     \
  } while (0U)

#define DEBUG_USART_RX_GPIO_PORT GPIOA
#define DEBUG_USART_RX_GPIO_CLK_ENABLE() LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)
#define DEBUG_USART_RX_PIN LL_GPIO_PIN_3
#define DEBUG_USART_RX_AF LL_GPIO_AF_4

#define DEBUG_USART_TX_GPIO_PORT GPIOA
#define DEBUG_USART_TX_GPIO_CLK_ENABLE() LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)
#define DEBUG_USART_TX_PIN LL_GPIO_PIN_2
#define DEBUG_USART_TX_AF LL_GPIO_AF_4

#define DEBUG_USART_IRQHandler USART2_IRQHandler
#define DEBUG_USART_IRQ USART2_IRQn
  /************************************************************/

  /** @defgroup Functions
   * @{
   */
  uint32_t BSP_GetVersion(void);

  void BSP_LED_Init(Led_TypeDef Led);
  void BSP_LED_DeInit(Led_TypeDef Led);
  void BSP_LED_On(Led_TypeDef Led);
  void BSP_LED_Off(Led_TypeDef Led);
  void BSP_LED_Toggle(Led_TypeDef Led);

  void BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
  void BSP_PB_DeInit(Button_TypeDef Button);
  uint32_t BSP_PB_GetState(Button_TypeDef Button);

  void BSP_USART_Config(void);
  /**
   * @}
   */

  /**
   * @}
   */

#ifdef __cplusplus
}
#endif

#endif