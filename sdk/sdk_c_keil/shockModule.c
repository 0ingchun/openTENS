// Copyright (C), 2024
// File name: shockModule.c
// Description: Library for c dev
// Author: 0ingChun    
// Version: 1.0     
// Date: 2024/2/29

#include "shockModule.h"

#include "tim.h"
// #include "gpio.h"
#include "usart.h"


#define CPU_FREQUENCY_MHZ    24		// STM32时钟主频24MHZ
void delay_us(__IO uint32_t delay)	// 微妙延时函数
{
    int last, curr, val;
    int temp;

    while (delay != 0)
    {
        temp = delay > 900 ? 900 : delay;
        last = SysTick->VAL;
        curr = last - CPU_FREQUENCY_MHZ * temp;
        if (curr >= 0)
        {
            do
            {
                val = SysTick->VAL;
            }
            while ((val < last) && (val >= curr));
        }
        else
        {
            curr += CPU_FREQUENCY_MHZ * 1000;
            do
            {
                val = SysTick->VAL;
            }
            while ((val <= last) || (val > curr));
        }
        delay -= temp;
    }
}


/**
 * 将一个值从一个范围映射到另一个范围
 * 
 * @param value 要映射的值。
 * @param in_min 输入范围的最小值。
 * @param in_max 输入范围的最大值。
 * @param out_min 输出范围的最小值。
 * @param out_max 输出范围的最大值。
 * @return 映射后的值。
 */
long longMap(long value, long in_min, long in_max, long out_min, long out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * 将一个浮点数值从一个范围映射到另一个范围，提供高精度的映射
 */
double doubleMap(double value, double in_min, double in_max, double out_min, double out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//const uint16_t boost_T = 40  /* 升压 周期 us */, boost_W = 750 /* 升压 脉宽 占空比 0~1023 */, boost_G = 8 /* 升压脉冲每组单位个数 */; //这些值理论计算后不变

// 调的参
// uint16_t boost_L = 1 /* 感觉 级数 0~15 */;
// uint16_t out1_W = 50 /* 触发脚 脉宽 us */;
// out1_T = 3.0 /* 触发脚 周期 ms */;
// uint16_t out1_C = 6 /* 触发脚 合成每次感觉要触发输出的次数 个 */;
// float once_T = 100.5 /* 触发每次感觉 周期 ms */;

// uint16_t boost_C = boost_G * boost_L /* 升压脉冲 每级总个数 */;


// 各引脚配置	//
void shockIOPinConfig(shockPluse_t* shockPluse_s_p)
{
	//shockPluse_s_p->GPIOx_Boost_L = NULL;
	//shockPluse_s_p->GPIO_Pin_Boost_L = NULL;
	shockPluse_s_p->htim_Boost_L = &htim1;
	shockPluse_s_p->Channel_Boost_L = TIM_CHANNEL_2;
	
	shockPluse_s_p->GPIOx_Net_P = NET_P_GPIO_Port;
	shockPluse_s_p->GPIO_Pin_Net_P = NET_P_Pin;
	shockPluse_s_p->GPIOx_Net_N = NET_N_GPIO_Port;
	shockPluse_s_p->GPIO_Pin_Net_N = NET_N_Pin;
}

// 相对常量配置	//
void shockConstConfig(shockPluse_t* shockPluse_s_p)
{
	// 这些值理论按原理图RC器件计算后不变
	shockPluse_s_p->boost_T = 40;	// 升压 周期 us
	shockPluse_s_p->boost_F = 25000;	// 升压 频率 hz
	shockPluse_s_p->boost_Width = 75;	// 升压 脉宽 占空比 0.0~100.0
	shockPluse_s_p->boost_uGroupCount = 8;	// 升压 脉冲每组单位个数
	
	// 参数计算
	shockPluse_s_p->boost_Count = shockPluse_s_p->boost_uGroupCount * shockPluse_s_p->boost_Level;	// 升压 脉冲 每级总个数
}

// 计算pwm频率至寄存器值	//
#define SystemCoreClock 24000000U // 示例：24MHz的系统时钟频率
/*
void pwmCalculatePSC_ARR(uint32_t targetFreq, uint32_t* PSC, uint32_t* ARR) {	// 有暗病，不敢用
	uint64_t tempPsc = 0, tempArr;
	uint64_t minDiff = UINT64_MAX;
	uint64_t targetPeriod = SystemCoreClock / targetFreq;

	// 遍历寻找最接近的PSC和ARR组合
	for (tempPsc = 0; tempPsc < 65536; ++tempPsc) {
		tempArr = targetPeriod / (tempPsc + 1) - 1;
		if (tempArr < 65536) {
			uint64_t currentFreq = SystemCoreClock / (tempPsc + 1) / (tempArr + 1);
			uint64_t diff = targetFreq > currentFreq ? targetFreq - currentFreq : currentFreq - targetFreq;
			if (diff < minDiff) {
				minDiff = diff;
				*PSC = tempPsc;
				*ARR = tempArr;
				if (diff == 0) break; // 找到精确匹配的频率
			}
		}
	}

	if (minDiff == UINT64_MAX) {
		printf("Unable to find suitable PSC and ARR values.\n");
		
	} else {
		printf("PSC: %lu, ARR: %lu\n", (unsigned long)*PSC, (unsigned long)*ARR);
	}
}
*/

void pwmCalculatePSC_ARR(uint32_t targetFreq, uint32_t* PSC, uint32_t* ARR) {
	uint32_t psc = CPU_FREQUENCY_MHZ-1, arr = 0;
	uint64_t tempPscArrProduct = SystemCoreClock / targetFreq;

	// 寻找合适的PSC和ARR值
	arr = (tempPscArrProduct / (psc+1) ) - 1;

	// 检查是否找到合适的值
	if (psc != 0 && arr != 0 && psc < 65536 && arr < 65536) {
		*PSC = psc;
		*ARR = arr;
		printf("PSC: %lu, ARR: %lu\n", (unsigned long)psc, (unsigned long)arr);
	} 
	else {
		*PSC = 0;
		*ARR = 0;
		printf("Unable to find suitable PSC and ARR values.\n");
	}
}

// 设置pwm频率	//
uint8_t shockBoostSetFreq(shockPluse_t* shockPluse_s_p, uint32_t boostPwmFreq_HZ)
{
	// 计算新的PSC和ARR值
	uint32_t newPrescalerValue = 0; // 根据新的PWM频率计算
	uint32_t newArrValue = 0; // 根据新的PWM频率计算
	
	pwmCalculatePSC_ARR(boostPwmFreq_HZ, &newPrescalerValue, &newArrValue);	// 根据频率计算定时器寄存器值
	
	if (newPrescalerValue != 0 && newArrValue != 0 && newPrescalerValue < 65536 && newArrValue < 65536)
	{		
		// 停止PWM输出
		HAL_TIM_PWM_Stop(shockPluse_s_p->htim_Boost_L, shockPluse_s_p->Channel_Boost_L);

		// 更新PSC和ARR
		__HAL_TIM_SET_PRESCALER(shockPluse_s_p->htim_Boost_L, newPrescalerValue);	// 16位定时器的最大PSC值为65535
		__HAL_TIM_SET_AUTORELOAD(shockPluse_s_p->htim_Boost_L, newArrValue);	// 16位定时器的最大ARR值为65535

		// 使更新生效
		if (HAL_TIM_GenerateEvent(shockPluse_s_p->htim_Boost_L, TIM_EVENTSOURCE_UPDATE) != HAL_OK) {
			printf("Fail to Set pwmFreq");
		}

		// 重新启动PWM输出
		HAL_TIM_PWM_Start(shockPluse_s_p->htim_Boost_L, shockPluse_s_p->Channel_Boost_L);
		
		return 0;
	}
	else {
		printf("Fail to set pwmFreq");
		return 1;
	}
}

// 设置pwm占空比	//
void shockBoostSetDuty(shockPluse_t* shockPluse_s_p, float pwmDutyCycle)
{
		// 利用ARR值，映射占空比到定时器寄存器，得出CCRx
	__HAL_TIM_SET_COMPARE(shockPluse_s_p->htim_Boost_L, shockPluse_s_p->Channel_Boost_L,
	 doubleMap(pwmDutyCycle, 0, 100, 0, __HAL_TIM_GET_AUTORELOAD(shockPluse_s_p->htim_Boost_L)) );    //通过修改比较值来改变占空比
	// printf("CCRx is %f\r\n", doubleMap(pwmDutyCycle, 0, 100, 0, __HAL_TIM_GET_AUTORELOAD(shockPluse_s_p->htim_Boost_L)));
}

// 电刺激总初始化	//
void shockAllInit(shockPluse_t* shockPluse_s_p)
{
	
	// 初始化 引脚，参数
	shockIOPinConfig(shockPluse_s_p);
	shockConstConfig(shockPluse_s_p);
	
	// 初始化boost_L的pwm频率配置
	shockBoostSetFreq(shockPluse_s_p, shockPluse_s_p->boost_F); // 设置周期为40微秒，对应25kHz的频率
	// 初始化boost_L的pwm输出置0
	shockBoostSetDuty(shockPluse_s_p, 0);
}

// 升压控制	//
void shockBoostVol(shockPluse_t* shockPluse_s_p)
{
	// 参数计算
	shockPluse_s_p->boost_Count = shockPluse_s_p->boost_uGroupCount * shockPluse_s_p->boost_Level;	// 升压 脉冲 每级总个数
	//printf("boost_C is %d\r\n", shockPluse_s_p->boost_Count);
	
	shockBoostSetDuty(shockPluse_s_p, shockPluse_s_p->boost_Width); // 打开占空比发送脉冲
	for (uint16_t i = 0; i < shockPluse_s_p->boost_Count; i++) {
		delay_us(shockPluse_s_p->boost_T + 1); // 循环等待所有升压脉冲结束
	}
	shockBoostSetDuty(shockPluse_s_p, 0); // 占空比置零 关闭升压脉冲
}

// 输出触发 交流	//
void shockTriggerAC(shockPluse_t* shockPluse_s_p, GPIO_PinState GPIO_PIN_Sta)
{
	// 触发脚电平反置
	HAL_GPIO_WritePin(NET_P_GPIO_Port, NET_P_Pin, GPIO_PIN_Sta);
	HAL_GPIO_WritePin(NET_N_GPIO_Port, NET_N_Pin, !GPIO_PIN_Sta);
	
	// 等待输出触发脉宽一周期
	delay_us(shockPluse_s_p->trig_Width);
	
	// 触发脚全置低
	HAL_GPIO_WritePin(NET_P_GPIO_Port, NET_P_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(NET_N_GPIO_Port, NET_N_Pin, GPIO_PIN_RESET);
}

// 输出触发 直流	//
void shockTriggerDC(shockPluse_t* shockPluse_s_p, GPIO_PinState GPIO_PIN_Sta)
{
	// 直流输出触发通常设计为触发是低电位脚
	// 触发脚电平拉高
	HAL_GPIO_WritePin(NET_N_GPIO_Port, NET_N_Pin, GPIO_PIN_Sta);
	// 等待输出触发脉宽一周期
	delay_us(shockPluse_s_p->trig_Width);
	// 触发脚置低
	HAL_GPIO_WritePin(NET_N_GPIO_Port, NET_N_Pin, GPIO_PIN_RESET);
}

// 感觉 参数控制	//
void shockPluseSenseSet(shockPluse_t* shockPluse_s_p)
{
	// 要调的参
	shockPluse_s_p->boost_Level = 0;	// 感觉 级数 0~15
	shockPluse_s_p->trig_Width = 0;	// 触发脚 脉宽 us
	shockPluse_s_p->trig_T = 0;	// 触发脚 周期 ms
	shockPluse_s_p->trig_Count = 0;	// 触发脚 合成每次感觉要触发输出的次数 个
	shockPluse_s_p->usense_T = 0;	// 触发每次感觉 周期 ms
	
	/*
	shockPluse_s_p->boost_Level = 10;	// 感觉 级数 0~15
	shockPluse_s_p->trig_Width = 50;	// 触发脚 脉宽 us
	shockPluse_s_p->trig_T = 3.0;	// 触发脚 周期 ms
	shockPluse_s_p->trig_Count = 6;	// 触发脚 合成每次感觉要触发输出的次数 个
	shockPluse_s_p->usense_T = 100.5;	// 触发每次感觉 周期 ms
*/
}

// 感觉 参数测试	//
void shockPluseSenseTest(shockPluse_t* shockPluse_s_p)
{
	// 要调的参
	shockPluse_s_p->boost_Level = 1;	// 感觉 级数 0~15
	shockPluse_s_p->trig_Width = 50;	// 触发脚 脉宽 us
	shockPluse_s_p->trig_T = 3.0;	// 触发脚 周期 ms
	shockPluse_s_p->trig_Count = 6;	// 触发脚 合成每次感觉要触发输出的次数 个
	shockPluse_s_p->usense_T = 100.5;	// 触发每次感觉 周期 ms
	
	

}

// 脉冲输出 单位感觉	//
void shockPulseSenseUnit(shockPluse_t* shockPluse_s_p)
{
	// 合成一次感觉
  for (uint16_t j = 0; j < shockPluse_s_p->trig_Count; j++){
		// HAL_GPIO_TogglePin(LED13_GPIO_Port,LED13_Pin);	// 指示灯
		
    // 升压脉冲
		shockBoostVol(shockPluse_s_p);
    // 升压后 触发输出
		shockTriggerAC(shockPluse_s_p, GPIO_PIN_SET);
		
		// 升压脉冲
		shockBoostVol(shockPluse_s_p);
		// 升压后 触发输出
		shockTriggerAC(shockPluse_s_p, GPIO_PIN_RESET);


    // HAL_GPIO_TogglePin(LED13_GPIO_Port,LED13_Pin);	// 指示灯

    // 等待单位触发输出间隔时间
    HAL_Delay(shockPluse_s_p->trig_T);
  }


  // 等待 开始发起下一次感觉
  HAL_Delay(shockPluse_s_p->usense_T);
	
}

// 脉冲输出 函数波形	//
void shockPluseFunction(shockPluse_t* shockPluse_s_p)
{
	
	
	
}

uint8_t shockParameterUnpacking(uint8_t* unpack_buffer, uint8_t buffer_len, shockPluse_t* shockPluse_s_p)
{
    dma_printf("ParameterUnpacking()...\r\n");
    HAL_Delay(10);
    if (unpack_buffer != NULL && buffer_len > 0)
    {
        if (unpack_buffer[0] == 0x00) {   //原生参数调节
            shockPluse_s_p->boost_Level = unpack_buffer[1];	// 感觉 级数 0~15
            shockPluse_s_p->trig_Width = unpack_buffer[2];	// 触发脚 脉宽 us
            shockPluse_s_p->trig_T = unpack_buffer[3];	// 触发脚 周期 ms
            shockPluse_s_p->trig_Count = unpack_buffer[4];	// 触发脚 合成每次感觉要触发输出的次数 个
            shockPluse_s_p->usense_T = unpack_buffer[5];	// 触发每次感觉 周期 ms
            
            dma_printf("Para Changeing...\r\n");
            HAL_Delay(100);
            dma_printf("boost_Level: %d\n", shockPluse_s_p->boost_Level); // 打印感觉级数
            HAL_Delay(100);
            dma_printf("trig_Width: %d us\n", shockPluse_s_p->trig_Width); // 打印触发脚脉宽，单位微秒(us)
            HAL_Delay(100);
            dma_printf("trig_T: %f ms\n", shockPluse_s_p->trig_T); // 打印触发脚周期，单位毫秒(ms)
            HAL_Delay(100);
            dma_printf("trig_Count: %d\n", shockPluse_s_p->trig_Count); // 打印每次感觉要触发输出的次数
            HAL_Delay(100);
            dma_printf("usense_T: %f ms\n", shockPluse_s_p->usense_T); // 打印触发每次感觉周期，单位毫秒(ms)
            HAL_Delay(100);
                // 要调的参
        }
        else if (unpack_buffer[0] == 0x01) {  //相对指令调节
            
        }
        return 0;
    }
    else return 1;
}



#define SAMPLE_SIZE 10  // 滑动平均滤波器的样本大小
#define ADC_MAX 4096    // ADC采样的最大值
#define ADC_MIN 0       // ADC采样的最小值
#define INTENSITY_MAX 12 // 电刺激强度的最大值
#define INTENSITY_MIN 0   // 电刺激强度的最小值
#define IMPACT_K 0.5 // ADC采样结果变化对强度变化的影响系数
#define IMPACT_A 0.5 // ADC采样结果变化对强度变化的附加系数

// 全局变量，用于存储当前电刺激强度值
static int currentIntensity = INTENSITY_MIN; 

void shockAutoAdjust(uint32_t adcSample, shockPluse_t* shockPluse_s_p){
    static int samples[SAMPLE_SIZE] = {0};
    static int sampleIndex = 0;
    static int sum = 0; // 采样值的累计总和，用于计算平均值
    
    // 更新采样值，并计算滑动平均值
    sum -= samples[sampleIndex]; // 减去旧的样本值
    samples[sampleIndex] = adcSample; // 更新为新的样本值
    sum += adcSample; // 加上新的样本值
    sampleIndex = (sampleIndex + 1) % SAMPLE_SIZE; // 更新索引，循环使用数组
    int averageADC = sum / SAMPLE_SIZE; // 计算平均ADC值
    dma_printf("adcSample=%d; sampleIndex=%d; sum=%d;\r\n", adcSample, sampleIndex, sum);
    HAL_Delay(100);
    // 计算新的电刺激强度
    int adcRange = ADC_MAX - ADC_MIN;
    int mappedIntensity = ((ADC_MAX - averageADC) * (INTENSITY_MAX - INTENSITY_MIN) / adcRange);
    int intensityDelta = (mappedIntensity - currentIntensity) * IMPACT_K + IMPACT_A;
    currentIntensity += intensityDelta;
    
    // 限制电刺激强度在设定的范围内
    if (currentIntensity > INTENSITY_MAX) {
        currentIntensity = INTENSITY_MAX;
    } else if (currentIntensity < INTENSITY_MIN) {
        currentIntensity = INTENSITY_MIN;
    }
    
    dma_printf("ADC Sample: %d, Average ADC: %d, Adjusted Intensity: %d\n", adcSample, averageADC, currentIntensity);
    HAL_Delay(100);
}