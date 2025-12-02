#include <Arduino.h>

#include "shockModule.h"

const int ledPin = 13; // 使用GPIO 2作为蓝灯的控制引脚

shockPluse_t shockPluse_s;	// 申明一个电刺激结构体

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(ledPin, OUTPUT); // 设置LED引脚为输出模式
  digitalWrite(ledPin, LOW); // 关闭LED

  shockAllInit(&shockPluse_s);	// 初始化电刺激功能
}

int settings_temp[5] = {0};

void SenseSetting_1()
{
  settings_temp[0] = 10; // Level
  settings_temp[1] = 70;  // Width %
  settings_temp[2] = 1;  // Trig Period ms
  settings_temp[3] = 10;  // Count
  settings_temp[4] = 1; // Sense Period ms
}

void SenseSetting_2()
{
  settings_temp[0] = 10; // Level
  settings_temp[1] = 50;  // Width %
  settings_temp[2] = 5;  // Trig Period ms
  settings_temp[3] = 10;  // Count
  settings_temp[4] = 1; // Sense Period ms
}

void loop() {

  SenseSetting_2();
	// Pass the address of the struct and the array
  shockPluseSenseSet(&shockPluse_s, settings_temp);
  shockPulseSenseUnit(&shockPluse_s);
	
	// 0rder:{Level,Width us, Trig Period ms, Count, Sense Period ms}
	SenseSetting_1();
	// Pass the address of the struct and the array
  shockPluseSenseSet(&shockPluse_s, settings_temp);
  shockPulseSenseUnit(&shockPluse_s);

  SenseSetting_2();
	// Pass the address of the struct and the array
  shockPluseSenseSet(&shockPluse_s, settings_temp);
  shockPulseSenseUnit(&shockPluse_s);

}