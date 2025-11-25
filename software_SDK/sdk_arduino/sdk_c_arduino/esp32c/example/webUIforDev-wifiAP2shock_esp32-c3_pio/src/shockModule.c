#include "shockModule.h"

#include <Arduino.h>


void delay_us(uint32_t delay)	// 微妙延时函数
{
    delayMicroseconds(delay);
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
	shockPluse_s_p->GPIO_Pin_Net_P = NET_P_Pin;
	shockPluse_s_p->GPIO_Pin_Net_N = NET_N_Pin;
	shockPluse_s_p->GPIO_Pin_Boost_L = BOOST_L_Pin;
	shockPluse_s_p->LEDC_CHANNEL = LEDC_CHANNEL_NUM;
	pinMode(shockPluse_s_p->GPIO_Pin_Boost_L, OUTPUT);
	pinMode(shockPluse_s_p->GPIO_Pin_Net_P, OUTPUT);
	pinMode(shockPluse_s_p->GPIO_Pin_Net_N, OUTPUT);
}

// 相对常量配置	//
void shockConstConfig(shockPluse_t* shockPluse_s_p)
{
	// 这些值理论按原理图器件计算后不变
	shockPluse_s_p->boost_T = 40; //40;	// 升压 周期 us
	shockPluse_s_p->boost_F = 25000;	// 升压 频率 hz
	shockPluse_s_p->boost_Width = 75;	// 升压 脉宽 占空比 0.0~100.0
	shockPluse_s_p->boost_uGroupCount = 8;	// 升压 脉冲每组单位个数8
	
	// 参数计算
	shockPluse_s_p->boost_Count = shockPluse_s_p->boost_uGroupCount * shockPluse_s_p->boost_Level;	// 升压 脉冲 每级总个数
}

// // 设置pwm频率	//
// uint8_t shockBoostSetFreq(shockPluse_t* shockPluse_s_p, uint32_t boostPwmFreq_HZ)
// {
// 	// 停止PWM输出
// 	analogWrite(shockPluse_s_p->GPIO_Pin_Boost_L, 0);
// 	// 更新PSC和ARR
// 	analogWriteFreq(boostPwmFreq_HZ);
// 	// 使更新生效
// 	analogWriteRange(100);
// 	// 重新启动PWM输出
// 	return 0;
// }

// 选择一个 LEDC 通道与分辨率
// #define LEDC_CHANNEL_0      0
// #define LEDC_TIMER_13_BIT   13  // 例如 13 位分辨率
// #define LEDC_BASE_FREQ      5000 // 可设置一个默认频率

uint8_t shockBoostSetFreq(shockPluse_t* shockPluse_s_p, uint32_t boostPwmFreq_HZ)
{
    // 1. 若需要，先取消之前可能绑定的 PWM（确保开始前没有残留）
    ledcDetachPin(shockPluse_s_p->GPIO_Pin_Boost_L);

    // 2. 根据你想要的新频率 & 分辨率配置 LEDC 通道
    //    这里以 8 位分辨率为例：
    ledcSetup(shockPluse_s_p->LEDC_CHANNEL, boostPwmFreq_HZ, 8);

    // 3. 将引脚绑定到 LEDC 通道
    ledcAttachPin(shockPluse_s_p->GPIO_Pin_Boost_L, shockPluse_s_p->LEDC_CHANNEL);

    // 4. (重新)启动 PWM，设置一个占空比（如 50%）
    ledcWrite(shockPluse_s_p->LEDC_CHANNEL, 191); // 8 位分辨率时 128/255 ≈ 50%

    return 0;
}

// 设置pwm占空比	//
void shockBoostSetDuty(shockPluse_t* shockPluse_s_p, float pwmDutyCycle)
{
	// analogWrite(shockPluse_s_p->GPIO_Pin_Boost_L, doubleMap(pwmDutyCycle, 0, 100, 0, 1000));
	// analogWrite(shockPluse_s_p->GPIO_Pin_Boost_L, pwmDutyCycle);
	ledcWrite(shockPluse_s_p->LEDC_CHANNEL, pwmDutyCycle);
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
void shockTriggerAC(shockPluse_t* shockPluse_s_p, uint8_t GPIO_PIN_Sta)
{
	if (shockPluse_s_p->trig_Width > 0)
	{
		// 触发脚电平反置
		digitalWrite(NET_P_Pin, GPIO_PIN_Sta);
		digitalWrite(NET_N_Pin, !GPIO_PIN_Sta);
		// digitalWrite(NET_P_Pin, HIGH);
		// digitalWrite(NET_N_Pin, LOW);
		
		// 等待输出触发脉宽一周期
		delay_us(shockPluse_s_p->trig_Width);
	}
	
	// 触发脚全置低
	digitalWrite(NET_P_Pin, LOW);
	digitalWrite(NET_N_Pin, LOW);
}

// // 输出触发 直流	//
// void shockTriggerDC(shockPluse_t* shockPluse_s_p, uint8_t GPIO_PIN_Sta)
// {
// 	// 直流输出触发通常设计为触发是低电位脚
// 	// 触发脚电平拉高
// 	digitalWrite(NET_N_Pin, GPIO_PIN_Sta);
// 	// 等待输出触发脉宽一周期
// 	delay_us(shockPluse_s_p->trig_Width);
// 	// 触发脚置低
// 	digitalWrite(NET_N_Pin, LOW);
// }

// 感觉 参数控制	//
void shockPluseSenseSet(shockPluse_t* shockPluse_s_p, int* p_temp)
{
	// 要调的参
	// shockPluse_s_p->boost_Level = 6;	// 感觉 级数 0~15
	// shockPluse_s_p->trig_Width = 50;	// 触发脚 脉宽 us
	// shockPluse_s_p->trig_T = 0.0;	// 触发脚 周期 ms    3.0
	// shockPluse_s_p->trig_Count = 6;	// 触发脚 合成每次感觉要触发输出的次数 个
	// shockPluse_s_p->usense_T = 0.0;	// 触发每次感觉 周期 ms 100.5
	shockPluse_s_p->boost_Level = p_temp[0];	// 感觉 级数 0~15
	shockPluse_s_p->trig_Width = p_temp[1];	// 触发脚 脉宽 us
	shockPluse_s_p->trig_T = p_temp[2];	// 触发脚 周期 ms    3.0
	shockPluse_s_p->trig_Count = p_temp[3];	// 触发脚 合成每次感觉要触发输出的次数 个
	shockPluse_s_p->usense_T = p_temp[4];	// 触发每次感觉 周期 ms 100.5
}

// 脉冲输出 单位感觉	//
void shockPulseSenseUnit(shockPluse_t* shockPluse_s_p)
{
	// 合成一次感觉
	for (uint16_t j = 0; j < shockPluse_s_p->trig_Count; j++) {
		//HAL_GPIO_TogglePin(LED13_GPIO_Port,LED13_Pin);	// 指示灯

    // 升压脉冲
		shockBoostVol(shockPluse_s_p);
    // 升压后 触发输出
		static uint8_t gpioState = 0;
		gpioState = !gpioState;
		shockTriggerAC(shockPluse_s_p, gpioState);
		
		// // 升压脉冲
		// shockBoostVol(shockPluse_s_p);
		// // 升压后 触发输出
		// shockTriggerAC(shockPluse_s_p, LOW);

    //HAL_GPIO_TogglePin(LED13_GPIO_Port,LED13_Pin);	// 指示灯

    // 等待单位触发输出间隔时间
    delay(shockPluse_s_p->trig_T);
	}

  	// 等待 开始发起下一次感觉
	delay(shockPluse_s_p->usense_T);
	
}

// 脉冲输出 函数波形	//
void shockPluseFunction(shockPluse_t* shockPluse_s_p)
{
	
	
	
}

