#include <Arduino.h>
#include <WiFi.h>           // ESP32 版 WiFi 库
#include <WebServer.h>      // ESP32 版 WebServer 库
#include <DNSServer.h>      // ESP32 同样支持该库
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "shockModule.h"    // 电击功能头文件
#include "Adafruit_NeoPixel.h"
#include "Arduino_BMI270_BMM150.h"


// ------------------ 宏定义及全局变量 ------------------
#define MAX_CONNECTIONS_NUM 3

// const int ledPin = 2; // 如果你的板载 LED 在 GPIO2，那就保留；否则请改为实际 IO 口
const int batDetPin = 0;
const int swPin = 3;
const int pwCtlPin = 1;

// 网页控制缓存参数
int slider_temp[5] = {0};
uint8_t switch_temp = 0;
String selectedMode_temp = "Option 0"; // 默认选项

String generateHTML();

char wifiName[32];
const char* ssid = "SHOCK⚡32c3 ";  // 可以自行改成 "SHOCK⚡32 " 等
const char* password = "07210721";

const byte DNS_PORT = 53;                // DNS 端口号
IPAddress apIP(192, 168, 4, 1);          // ESP32 AP 模式的 IP

WebServer webServer(80);   // ESP32 版 WebServer
DNSServer dnsServer;

// 声明一个电刺激结构体
shockPluse_t shockPluse_s;

// Matrix Data PIN
#define PIN_PIXS 2
#define PIX_NUM 1

Adafruit_NeoPixel pixels(PIX_NUM, PIN_PIXS, NEO_GRB + NEO_KHZ800);


#ifndef I2CSPEED
#define I2CSPEED 100000
#endif

BoschSensorClass *myIMU;

void inline initBoard() {
  pixels.begin();
  pixels.setBrightness(10);
  pixels.clear();
}

void inline pixelsCheck() {
  pixels.fill(0xFF0000, 0, PIX_NUM);
  pixels.show();
  sleep(1);
  pixels.fill(0xFF00, 0, PIX_NUM);
  pixels.show();
  sleep(1);
  pixels.fill(0xFF, 0, PIX_NUM);
  pixels.show();
  sleep(1);
  pixels.fill(0xFFFFFF, 0, PIX_NUM);
  pixels.show();
  sleep(1);
  pixels.clear();
}

// ------------------ Setup 函数 ------------------
void setup() {
  Serial.begin(115200);
  // pinMode(ledPin, OUTPUT); 
  // digitalWrite(ledPin, HIGH); // 先关闭 LED

  pinMode(batDetPin, INPUT); // GPIO0 电量检测 输入模式
  pinMode(swPin, INPUT_PULLUP); // GPIO3 复用按钮 输入模式
  pinMode(pwCtlPin, OUTPUT); // GPIO1 电源自锁 输出模式
  digitalWrite(pwCtlPin,  HIGH);

  while (digitalRead(swPin) == LOW)
  {
    if (digitalRead(swPin) == LOW)
    {
      delay(1000);
      if (digitalRead(swPin) == LOW)
      {
        digitalWrite(pwCtlPin,  HIGH); // 电源自锁
        Serial.println("SHOCKER BEGIAN");
        break;
      }
      else digitalWrite(pwCtlPin,  LOW); // 电源自锁
    }
    else digitalWrite(pwCtlPin,  LOW); // 电源自锁
  }

  // 获取 MAC 地址并拼接到 AP 名称后
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("MAC Address: ");
  Serial.println(macStr);
  sprintf(wifiName, "%s%s", ssid, macStr);

  // 配置 WebServer 路由
  webServer.on("/", HTTP_GET, []() {
    webServer.send(200, "text/html", generateHTML());
  });

  webServer.on("/slider", HTTP_POST, []() {
    String sliderId = webServer.arg("slider");
    String value = webServer.arg("value");
    Serial.println("滑条 " + sliderId + " 的值: " + value);
    slider_temp[sliderId.toInt()-1] = value.toInt();
    shockPluseSenseSet(&shockPluse_s, slider_temp);
    webServer.send(200, "text/plain", "OK");
  });

  webServer.on("/switch", HTTP_POST, []() {
    String state = webServer.arg("state");
    Serial.println("开关状态: " + String(state == "true" ? "ON" : "OFF"));
    switch_temp = state == "true" ? 1 : 0;
    webServer.send(200, "text/plain", "OK");
  });


  // 初始化电刺激功能
  shockAllInit(&shockPluse_s);

  initBoard();
  pixelsCheck();

  // 初始化 IMU
  Wire.begin(4, 5, I2CSPEED);
  myIMU = new BoschSensorClass(Wire);
  if (!myIMU->begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(myIMU->accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration in G's");
  Serial.println("X\tY\tZ");

  Serial.print("Gyroscope sample rate = ");
  Serial.print(myIMU->gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Gyroscope in degrees/second");
  Serial.println("X\tY\tZ");
}

// ------------------ Loop 函数 ------------------
void loop() {
  static bool isAPMode = false; // 用于跟踪当前是否处于 AP 模式

  // 如果 AP 下有人连接，且当前非 AP 广播模式(isAPMode=false 说明之前已停播)，则只处理 web
  // 如果 AP 下无人连接，且当前是 AP 模式(isAPMode=true)，则启动/恢复广播
  // 以下逻辑可按自身需求进行修改

  // 检测到有设备连接到热点
  if (WiFi.softAPgetStationNum() > 0 && isAPMode) {
    // 启动 Web 和 DNS 服务
    dnsServer.start(DNS_PORT, "*", apIP);
    webServer.begin();
    isAPMode = false;
    Serial.println("HTTP and DNS services started, AP broadcast stopped.");
  }
  // 检测到没有设备连接到热点
  else if (WiFi.softAPgetStationNum() == 0 && !isAPMode) {
    // digitalWrite(ledPin, HIGH); // 关闭 LED

    // 重新初始化 AP
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    // channel=0 表示自动选择，隐藏(SSID hidden)=0(不隐藏)，最大连接数=MAX_CONNECTIONS_NUM
    if(WiFi.softAP(wifiName, password, 0, 0, MAX_CONNECTIONS_NUM)) {
      Serial.println("ESP32 SoftAP is right");
    }

    // 停止 Web 和 DNS 服务
    dnsServer.stop();
    webServer.close();

    isAPMode = true;
    Serial.println("AP broadcast started, HTTP and DNS services stopped.");
  }

  // 如果不是 AP 模式，则处理 WebServer 和 DNSServer 请求
  if (!isAPMode) {
    // 如果开关被打开，则让 LED 闪烁/或者其他动作
    if (switch_temp == 1) {
      pixels.setPixelColor(0, pixels.Color(128, 0, 128)); // 设置第一个灯为紫色
      pixels.show();
    } 
    else {
      pixels.setPixelColor(0, pixels.Color(0, 128, 128)); // 设置第一个灯为紫色
      pixels.show();
    }

    // 处理 web 请求
    webServer.handleClient();
    // dnsServer.processNextRequest(); // 若你需要 DNS 劫持，可以开启
  }
  else {
    pixels.setPixelColor(0, pixels.Color(60, 60, 60)); // 设置第一个灯为灰色
    pixels.show();
  }

  // switch_temp == 1 才执行 shockPulseSenseUnit
  if (switch_temp == 1) {
    shockPulseSenseUnit(&shockPluse_s);
  }

  float x, y, z;
  if (myIMU->accelerationAvailable()) {
    myIMU->readAcceleration(x, y, z);
    Serial.printf(">accelX: %f\n>accelY: %f\n>accelz: %f\n", x, y, z);
  }
  if (myIMU->gyroscopeAvailable()) {
    myIMU->readGyroscope(x, y, z);
    Serial.printf(">gyroX: %f\n>gyroY: %f\n>gyroZ: %f\n", x, y, z);
  }

  int adcVal = analogRead(0); // 读 ADC1_CH0
  Serial.print("ADC value on GPIO0 = ");
  Serial.println(adcVal);
  delay(1);

  if (digitalRead(swPin) == LOW)
  {
    pixels.fill(pixels.Color(128, 128, 0)); // 设置所有灯为黄色
    pixels.show();
    delay(2000);
    if (digitalRead(swPin) == LOW)
    {
      Serial.println("SHOCKER STOP");
      pixels.clear();
      pixels.show();
      digitalWrite(pwCtlPin,  LOW); // 电源解锁
      while (1)
      {
      }
      
    }
  }

}

// ------------------ 生成 HTML 的函数 ------------------



String generateHTML() {
  String html = "<html><head><meta charset='UTF-8'><title>SHOCK⚡8266 控制面板</title></head><body>"
                "<h1>原始参数调节</h1>"
                "<script>"
                "function updateSlider(sliderId, value) {"
                " document.getElementById('value' + sliderId).innerText = value;"
                " fetch(`/slider`, {"
                "   method: 'POST',"
                "   headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
                "   body: `slider=${sliderId}&value=${value}`"
                " }).then(response => {"
                "   if (!response.ok) throw new Error('Network response was not ok.');"
                " }).catch(error => console.error('Fetch error:', error));"
                "}"
                "function updateSwitch(state) {"
                " fetch(`/switch`, {"
                "   method: 'POST',"
                "   headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
                "   body: `state=${state}`"
                " }).then(response => {"
                "   if (!response.ok) throw new Error('Network response was not ok.');"
                " }).catch(error => console.error('Fetch error:', error));"
                "}"
                "function updateMode(mode) {"
                " fetch(`/mode`, {"
                "   method: 'POST',"
                "   headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
                "   body: `mode=${mode}`"
                " }).then(response => {"
                "   if (!response.ok) throw new Error('Network response was not ok.');"
                " }).catch(error => console.error('Fetch error:', error));"
                "}"
                "</script>";

  // 滑条 1: 感觉级数
  html += "<div>感觉 级数 0~15: <input type='range' min='0' max='150' step='1' value='" + String(slider_temp[0]) + "' onchange='updateSlider(1, this.value)' />"
          "<span id='value1'>" + String(slider_temp[0]) + "</span></div>";

  // 滑条 2: 触发脚脉宽
  html += "<div>触发脚 脉宽 us: <input type='range' min='0' max='150' step='1' value='" + String(slider_temp[1]) + "' onchange='updateSlider(2, this.value)' />"
          "<span id='value2'>" + String(slider_temp[1]) + "</span></div>";

  // 滑条 3: 触发脚周期
  html += "<div>触发脚 周期 ms: <input type='range' min='0' max='30' step='1' value='" + String(slider_temp[2]) + "' onchange='updateSlider(3, this.value)' />"
          "<span id='value3'>" + String(slider_temp[2]) + "</span></div>";

  // 滑条 4: 触发/感觉 次数
  html += "<div>触发脚 触发/感觉 个: <input type='range' min='0' max='100' step='1' value='" + String(slider_temp[3]) + "' onchange='updateSlider(4, this.value)' />"
          "<span id='value4'>" + String(slider_temp[3]) + "</span></div>";

  // 滑条 5: 触发每次感觉周期
  html += "<div>触发每次感觉 周期 ms: <input type='range' min='0' max='1000' step='1' value='" + String(slider_temp[4]) + "' onchange='updateSlider(5, this.value)' />"
          "<span id='value5'>" + String(slider_temp[4]) + "</span></div>";

  // 开关对应 checkbox
  String checkedStr = (switch_temp == 1) ? "checked" : "";
  html += "<div><input type='checkbox' id='switch' onchange='updateSwitch(this.checked)' " + checkedStr + " /> 安全磁🥰贴哦</div>";

  // 下拉框
  html += "<div>模式选择: <select id='mode' onchange='updateMode(this.value)'>";
  String modes[] = {"Option 0", "Option 1", "Option 2", "Option 3"};
  for (String mode : modes) {
      html += "<option value='" + mode + "'" + (mode.equals(selectedMode_temp) ? " selected" : "") + ">" + mode + "</option>";
  }
  html += "</select></div>";

  html += "</body></html>";

  return html;
}
