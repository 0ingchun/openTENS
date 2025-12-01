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

const int batDetPin = 0;

// 网页控制缓存参数
int slider_temp[5] = {0};
uint8_t switch_temp = 0;
String selectedMode_temp = "Option 0"; // 默认选项

String generateHTML();

char wifiName[32];
const char* ssid = "SHOCK⚡32 ";  // 可以自行改成 "SHOCK⚡32 " 等
const char* password = ""; // 可选择设置密码

const byte DNS_PORT = 53;                // DNS 端口号
IPAddress apIP(192, 168, 4, 1);          // ESP32 AP 模式的 IP

WebServer webServer(80);   // ESP32 版 WebServer
DNSServer dnsServer;

// 声明一个电刺激结构体
shockPluse_t shockPluse_s;

// Matrix Data PIN
#define PIN_PIXS 8
#define PIX_NUM 1

Adafruit_NeoPixel pixels(PIX_NUM, PIN_PIXS, NEO_GRB + NEO_KHZ800);


#ifndef I2CSPEED
#define I2CSPEED 100000
#endif

BoschSensorClass *myIMU;


// ------------------ Setup 函数 ------------------
void setup() {
  Serial.begin(115200);
  // pinMode(ledPin, OUTPUT); 
  // digitalWrite(ledPin, HIGH); // 先关闭 LED

  pinMode(batDetPin, INPUT); // GPIO0 电量检测 输入模式

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
      // pixels.setPixelColor(0, pixels.Color(128, 0, 128)); // 设置第一个灯为紫色
      // pixels.show();
    } 
    else {
      // pixels.setPixelColor(0, pixels.Color(0, 128, 128)); // 设置第一个灯为紫色
      // pixels.show();
    }

    // 处理 web 请求
    webServer.handleClient();
    // dnsServer.processNextRequest(); // 若你需要 DNS 劫持，可以开启
  }
  else {
    // pixels.setPixelColor(0, pixels.Color(60, 60, 60)); // 设置第一个灯为灰色
    // pixels.show();
  }

  // switch_temp == 1 才执行 shockPulseSenseUnit
  if (switch_temp == 1) {
    shockPulseSenseUnit(&shockPluse_s);
  }

  int adcVal = analogRead(0); // 读 ADC1_CH0
  Serial.print("ADC value on GPIO0 = ");
  Serial.println(adcVal);
  delay(1);

}

// ------------------ 生成 HTML 的函数 ------------------

String generateHTML() {
  String html =
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>SHOCK⚡8266 控制面板</title>"
    "<style>"
    "body {"
    "  margin: 0;"
    "  padding: 16px;"
    "  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif;"
    "  background: #020617;"          /* 深色背景 */
    "  color: #e5e7eb;"
    "}"
    ".container {"
    "  max-width: 520px;"            /* 比原来更大一点 */
    "  margin: 0 auto;"
    "}"
    "h1 {"
    "  font-size: 24px;"
    "  margin: 0 0 8px 0;"
    "}"
    ".subtitle {"
    "  font-size: 14px;"
    "  color: #9ca3af;"
    "  margin-bottom: 16px;"
    "}"
    ".card {"
    "  background: rgba(15,23,42,0.96);"
    "  border-radius: 18px;"
    "  padding: 16px 18px;"
    "  box-shadow: 0 18px 45px rgba(0,0,0,0.45);"
    "  border: 1px solid rgba(148,163,184,0.25);"
    "}"
    ".control-group {"
    "  margin-bottom: 18px;"
    "}"
    ".control-header {"
    "  display: flex;"
    "  justify-content: space-between;"
    "  align-items: center;"
    "  font-size: 15px;"
    "  margin-bottom: 6px;"
    "}"
    ".control-label {"
    "  font-weight: 500;"
    "}"
    ".control-value {"
    "  font-family: 'SF Mono', ui-monospace, Menlo, Monaco, Consolas, 'Liberation Mono', 'Courier New', monospace;"
    "  font-size: 14px;"
    "  color: #a5b4fc;"
    "}"
    "input[type=range] {"
    "  -webkit-appearance: none;"
    "  width: 100%;"
    "  height: 8px;"                 /* 滑条更粗一点 */
    "  border-radius: 999px;"
    "  background: #1f2937;"
    "  outline: none;"
    "}"
    "input[type=range]::-webkit-slider-thumb {"
    "  -webkit-appearance: none;"
    "  width: 18px;"                 /* 拇指变大，触控更方便 */
    "  height: 18px;"
    "  border-radius: 50%;"
    "  background: #38bdf8;"
    "  box-shadow: 0 0 0 4px rgba(56,189,248,0.25);"
    "  cursor: pointer;"
    "}"
    "input[type=range]::-moz-range-thumb {"
    "  width: 18px;"
    "  height: 18px;"
    "  border-radius: 50%;"
    "  background: #38bdf8;"
    "  box-shadow: 0 0 0 4px rgba(56,189,248,0.25);"
    "  cursor: pointer;"
    "}"
    ".section-title {"
    "  font-size: 14px;"
    "  font-weight: 500;"
    "  margin: 18px 0 8px 0;"
    "  color: #9ca3af;"
    "  text-transform: uppercase;"
    "  letter-spacing: 0.08em;"
    "}"
    ".row {"
    "  display: flex;"
    "  align-items: center;"
    "  justify-content: space-between;"
    "  gap: 12px;"
    "  margin-top: 8px;"
    "}"
    ".row-left {"
    "  display: flex;"
    "  align-items: center;"
    "  gap: 8px;"
    "  font-size: 15px;"
    "}"
    "#switch {"
    "  transform: scale(1.4);"       /* 开关放大 */
    "}"
    "select {"
    "  padding: 6px 10px;"
    "  border-radius: 999px;"
    "  border: 1px solid #4b5563;"
    "  background: #020617;"
    "  color: #e5e7eb;"
    "  font-size: 14px;"
    "}"
    "</style>"
    "</head><body>"
    "<div class='container'>"
    "<h1>控制面板</h1>"
    "<p class='subtitle'>原始参数调节 · 请谨慎升档，注意安全</p>"
    "<div class='card'>"

    "<div class='section-title'>PARAMETERS</div>";

  // 滑条 1: 感觉级数
  html +=
    "<div class='control-group'>"
    "  <div class='control-header'>"
    "    <span class='control-label'>感觉级数 (0–15)</span>"
    "    <span class='control-value' id='value1'>" + String(slider_temp[0]) + "</span>"
    "  </div>"
    "  <input type='range' min='0' max='150' step='1'"
    "         value='" + String(slider_temp[0]) + "'"
    "         onchange='updateSlider(1, this.value)' />"
    "</div>";

  // 滑条 2: 触发脚脉宽
  html +=
    "<div class='control-group'>"
    "  <div class='control-header'>"
    "    <span class='control-label'>触发脚 脉宽 (μs)</span>"
    "    <span class='control-value' id='value2'>" + String(slider_temp[1]) + "</span>"
    "  </div>"
    "  <input type='range' min='0' max='150' step='1'"
    "         value='" + String(slider_temp[1]) + "'"
    "         onchange='updateSlider(2, this.value)' />"
    "</div>";

  // 滑条 3: 触发脚周期
  html +=
    "<div class='control-group'>"
    "  <div class='control-header'>"
    "    <span class='control-label'>触发脚 周期 (ms)</span>"
    "    <span class='control-value' id='value3'>" + String(slider_temp[2]) + "</span>"
    "  </div>"
    "  <input type='range' min='0' max='30' step='1'"
    "         value='" + String(slider_temp[2]) + "'"
    "         onchange='updateSlider(3, this.value)' />"
    "</div>";

  // 滑条 4: 触发/感觉 次数
  html +=
    "<div class='control-group'>"
    "  <div class='control-header'>"
    "    <span class='control-label'>触发脚 触发/感觉 个数</span>"
    "    <span class='control-value' id='value4'>" + String(slider_temp[3]) + "</span>"
    "  </div>"
    "  <input type='range' min='0' max='100' step='1'"
    "         value='" + String(slider_temp[3]) + "'"
    "         onchange='updateSlider(4, this.value)' />"
    "</div>";

  // 滑条 5: 触发每次感觉周期
  html +=
    "<div class='control-group'>"
    "  <div class='control-header'>"
    "    <span class='control-label'>触发每次感觉 周期 (ms)</span>"
    "    <span class='control-value' id='value5'>" + String(slider_temp[4]) + "</span>"
    "  </div>"
    "  <input type='range' min='0' max='1000' step='1'"
    "         value='" + String(slider_temp[4]) + "'"
    "         onchange='updateSlider(5, this.value)' />"
    "</div>";

  // 开关 + 模式选择 分成一行
  String checkedStr = (switch_temp == 1) ? "checked" : "";
  html +=
    "<div class='section-title'>SAFETY & MODE</div>"
    "<div class='row'>"
    "  <div class='row-left'>"
    "    <input type='checkbox' id='switch' onchange='updateSwitch(this.checked)' " + checkedStr + " />"
    "    <span>注意使用安全</span>"
    "  </div>"
    "  <div>"
    "    <label for='mode' style='font-size:13px;color:#9ca3af;margin-right:4px;'>模式</label>"
    "    <select id='mode' onchange='updateMode(this.value)'>";

  String modes[] = {"Option 0", "Option 1", "Option 2", "Option 3"};
  for (String mode : modes) {
    html += "<option value='" + mode + "'" +
            (mode.equals(selectedMode_temp) ? " selected" : "") +
            ">" + mode + "</option>";
  }

  html +=
    "    </select>"
    "  </div>"
    "</div>"   // .row
    "</div>"   // .card
    "</div>"   // .container
    "<script>"
    "function updateSlider(sliderId, value) {"
    "  document.getElementById('value' + sliderId).innerText = value;"
    "  fetch(`/slider`, {"
    "    method: 'POST',"
    "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
    "    body: `slider=${sliderId}&value=${value}`"
    "  }).then(response => {"
    "    if (!response.ok) throw new Error('Network response was not ok.');"
    "  }).catch(error => console.error('Fetch error:', error));"
    "}"
    "function updateSwitch(state) {"
    "  fetch(`/switch`, {"
    "    method: 'POST',"
    "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
    "    body: `state=${state}`"
    "  }).then(response => {"
    "    if (!response.ok) throw new Error('Network response was not ok.');"
    "  }).catch(error => console.error('Fetch error:', error));"
    "}"
    "function updateMode(mode) {"
    "  fetch(`/mode`, {"
    "    method: 'POST',"
    "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
    "    body: `mode=${mode}`"
    "  }).then(response => {"
    "    if (!response.ok) throw new Error('Network response was not ok.');"
    "  }).catch(error => console.error('Fetch error:', error));"
    "}"
    "</script>"
    "</body></html>";

  return html;
}
