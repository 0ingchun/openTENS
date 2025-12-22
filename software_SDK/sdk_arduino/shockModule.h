// Copyright (C), 2024
// File name: shockModule.h
// Description: Library for esp8266 in Arduino
// Author: 0ingChun    
// Version: 1.1
// Date: 2024/3/18
#pragma once

/* USER CODE BEGIN Header */

/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SHOCK_MODULE_H__
#define __SHOCK_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
// #include "main.h"
#include <stdint.h>

#define USE_LEDC_HAL_API
// #define USE_LEDC_PIN_API

#if defined(ARDUINO_ARCH_ESP32)
	#if defined(USE_LEDC_PIN_API)
		#include "driver/ledc.h"
	#elif defined(USE_LEDC_HAL_API)
		#include "esp32-hal-ledc.h"
	#else
		#error "Please define USE_LEDC_PIN_API or USE_LEDC_HAL_API for ESP32 in src/shockModule.h"
	#endif
#endif

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
// #define NET_P_Pin 8
// #define NET_N_Pin 7
// #define BOOST_L_Pin 6

#if defined(ARDUINO_ARCH_ESP32)
	#define LEDC_CHANNEL_NUM_DEFAULT 0
	#define LEDC_RESOLUTION_BIT_DEFAULT 8
#endif

/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */

// 输出模式
typedef enum {
	SHOCK_MODE_AC_NEGATIVE = 0,    // 交流反向
	SHOCK_MODE_AC_POSITIVE,        // 交流正向
	SHOCK_MODE_AC_BIDIR,      	   // 交流双向
	SHOCK_MODE_DC,        	       // 直流
} shock_output_mode_t;

// 电刺激参数结构体	//
typedef struct
{
	// 模块引脚配置
	uint8_t GPIO_Pin_Boost_L;
	uint8_t GPIO_Pin_Net_P;
	uint8_t GPIO_Pin_Net_N;
#if defined(ARDUINO_ARCH_ESP32)
	uint8_t LEDC_CHANNEL;
	uint8_t LEDC_RESOLUTION;
#endif

	
	// 以下这些值理论按原理图器件计算后不变
	uint32_t boost_T;	// 升压 周期 us
	uint32_t boost_F;	// 升压 频率 hz
	float boost_Width;	// 升压 脉宽 占空比 0.0~100.0
	uint16_t boost_uGroupCount;	// 升压 脉冲每组单位个数

	// 以下为调变的参
	uint8_t boost_Level;	// 感觉 级数 0~15
	uint32_t trig_Width;	// 触发脚 脉宽 us
	float trig_T;	// 触发脚 周期 ms
	uint16_t trig_Count;	// 触发脚 合成每次感觉要触发输出的次数 个
	float usense_T;	// 触发每次感觉 周期 ms
	
	uint16_t boost_Count;	// 升压 脉冲 每级总个数

	shock_output_mode_t output_mode; // 输出模式：DC/ACx
	
} shockPluse_t;


// 微秒级延时函数
void delay_us(uint32_t delay);

// 将一个值从一个范围映射到另一个范围（整数版本）
long longMap(long value, long in_min, long in_max, long out_min, long out_max);

// 将一个值从一个范围映射到另一个范围（浮点数版本，高精度）
double doubleMap(double value, double in_min, double in_max, double out_min, double out_max);

// 各引脚配置
void shockIOPinConfig(shockPluse_t* shockPluse_s_p);

// 相对常量配置
void shockConstConfig(shockPluse_t* shockPluse_s_p);

// 设置pwm频率
uint8_t shockBoostSetFreq(shockPluse_t* shockPluse_s_p, uint32_t boostPwmFreq_HZ);

// 设置pwm占空比
void shockBoostSetDuty(shockPluse_t* shockPluse_s_p, uint32_t pwmDutyCycle);

// 电刺激总初始化
void shockAllInit(shockPluse_t* shockPluse_s_p);

// 升压控制
uint8_t shockBoostVol(shockPluse_t* shockPluse_s_p);

// 输出触发 交流
void shockTriggerAC(shockPluse_t* shockPluse_s_p, uint8_t GPIO_PIN_Sta);

// 输出触发 直流
void shockTriggerDC(shockPluse_t* shockPluse_s_p);

// 感觉 参数控制
void shockPluseSenseSet(shockPluse_t* shockPluse_s_p, int* p_temp);

// 脉冲输出 单位感觉
void shockPulseSenseUnit(shockPluse_t* shockPluse_s_p);

// 脉冲输出 停止，自动释放残留电压
void shockPluseStop(shockPluse_t* shockPluse_s_p);

// 脉冲输出 函数波形
void shockPluseFunction(shockPluse_t* shockPluse_s_p);


/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SHOCK_MODULE_H__ */
