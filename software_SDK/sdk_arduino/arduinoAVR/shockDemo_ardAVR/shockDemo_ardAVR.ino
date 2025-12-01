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

void loop() {
	
	// 0rder:{Level,Width us, Trig Period ms, Count, Sense Period ms}
	int settings[5] = {5, 70, 40, 7, 500};
	
	// Pass the address of the struct and the array
  shockPluseSenseSet(&shockPluse_s, settings);
  
  
  shockPulseSenseUnit(&shockPluse_s);

}