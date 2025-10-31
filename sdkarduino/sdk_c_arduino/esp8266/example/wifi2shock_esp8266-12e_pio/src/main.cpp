#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#include <shockModule.h>

#define MAX_CONNECTIONS_NUM 3

const int ledPin = 2; // 使用GPIO 2作为蓝灯的控制引脚

// 网页控制缓存参数
int slider_temp[5] = {0};
uint8_t switch_temp = 0;
String selectedMode_temp = "Option 0"; // 默认选项

String generateHTML();

char wifiName[32];
const char* ssid = "SHOCK⚡8266 ";
const char* password = "07210721";

const byte DNS_PORT = 53;//DNS端口号
IPAddress apIP(192, 168, 4, 1);//esp32-AP-IP地址

ESP8266WebServer webServer(80);
DNSServer dnsServer;

shockPluse_t shockPluse_s;	// 申明一个电刺激结构体

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT); // 设置LED引脚为输出模式
  digitalWrite(ledPin, HIGH); // 关闭LED

  //把mac拼接到ap_name后
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("MAC Address: ");
  Serial.println(macStr);
  sprintf(wifiName, "%s%s", ssid, macStr);

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

webServer.on("/mode", HTTP_POST, []() {
    selectedMode_temp = webServer.arg("mode");
    Serial.println("选择的模式: " + selectedMode_temp);
    webServer.send(200, "text/plain", "OK");
});

  // webServer.begin();
  // Serial.println("Web server started");

  shockAllInit(&shockPluse_s);	// 初始化电刺激功能
}

// void loop() {
//   webServer.handleClient();
//   dnsServer.processNextRequest();

//   // 检测是否有设备连接到热点
//   if (WiFi.softAPgetStationNum() > 0) {
//     if (switch_temp == 1) digitalWrite(ledPin, !digitalRead(ledPin));    // 读取当前LED引脚的状态并翻转它
//     else digitalWrite(ledPin, LOW); // 点亮LED（注意：有的开发板上LED点亮是低电平）

//   } else {
//     digitalWrite(ledPin, HIGH); // 关闭LED
//   }

//   if (switch_temp == 1){
//   shockPulseSenseUnit(&shockPluse_s);
//   }
// }

void loop() {
  static bool isAPMode = false; // 用于跟踪当前是否处于AP模式

  if (WiFi.softAPgetStationNum() > 0 && isAPMode) { // 检测到 有 设备连接到热点

    // if (switch_temp == 1) digitalWrite(ledPin, !digitalRead(ledPin));    // 读取当前LED引脚的状态并翻转它
    // else digitalWrite(ledPin, LOW); // 点亮LED（注意：有的开发板上LED点亮是低电平）
    // 有设备连接时，关闭热点广播，启动HTTP和DNS服务器
    // WiFi.softAP(wifiName, password, 0, 1, MAX_CONNECTIONS_NUM); // 禁用SSID广播
    dnsServer.start(DNS_PORT, "*", apIP); // 重新启动DNS服务，以防之前停止了
    webServer.begin(); // 重新启动Web服务，以防之前停止了
    isAPMode = false;
    Serial.println("HTTP and DNS services started, AP broadcast stopped.");

    dnsServer.processNextRequest();
  }

  else if (WiFi.softAPgetStationNum() == 0 && !isAPMode) {  // 检测到 没有 设备连接到热点

    digitalWrite(ledPin, HIGH); // 关闭LED

    // 没有设备连接时，重启热点广播，关闭HTTP和DNS服务器
    // WiFi.softAP(ssid, password); // 重新启动热点广播
    // dnsServer.stop(); // 停止DNS服务
    // webServer.close(); // 停止Web服务

  //初始化AP模式

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  Serial.println(WiFi.softAPIP());
  //  Serial.print("本地IP： ");
  //  Serial.println(WiFi.localIP());
  if(WiFi.softAP(wifiName, password, 0, 0, MAX_CONNECTIONS_NUM)){
      Serial.println("ESP32 SoftAP is right");
  }

  dnsServer.stop(); // 停止DNS服务
  webServer.close(); // 停止Web服务

    isAPMode = true;
    Serial.println("AP broadcast started, HTTP and DNS services stopped.");
  }
  // else {
  //   Serial.println(WiFi.softAPgetStationNum());
  //   Serial.println(isAPMode);
  //   Serial.println();
  // }



  if (!isAPMode) { // 如果不是AP模式，处理HTTP和DNS请求

    if (switch_temp == 1) {
      digitalWrite(ledPin, !digitalRead(ledPin));    // 读取当前LED引脚的状态并翻转它
      // shockPulseSenseUnit(&shockPluse_s);
    }
    else {
      digitalWrite(ledPin, LOW); // 点亮LED（注意：有的开发板上LED点亮是低电平）
    }
    webServer.handleClient();
    // dnsServer.processNextRequest();
  }

  if (switch_temp == 1) {

    shockPulseSenseUnit(&shockPluse_s);

  }

}


String generateHTML() {
  String html = "<html><head><meta charset='UTF-8'><title>SHOCK⚡8266 控制面板</title></head><body>"
                "<h1>原始参数调节</h1>"
                "<script>"
                "function updateSlider(sliderId, value) {"
                " document.getElementById('value' + sliderId).innerText = value;" // 更新页面上的滑条值显示
                " fetch(`/slider`, {"
                " method: 'POST',"
                " headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
                " body: `slider=${sliderId}&value=${value}`"
                " }).then(response => {"
                "   if (!response.ok) throw new Error('Network response was not ok.');"
                " }).catch(error => console.error('There has been a problem with your fetch operation:', error));"
                "}"
                "function updateSwitch(state) {"
                " fetch(`/switch`, {"
                " method: 'POST',"
                " headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
                " body: `state=${state}`"
                " }).then(response => {"
                "   if (!response.ok) throw new Error('Network response was not ok.');"
                " }).catch(error => console.error('There has been a problem with your fetch operation:', error));"
                "}"
                "function updateMode(mode) {"
                " fetch(`/mode`, {"
                " method: 'POST',"
                " headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
                " body: `mode=${mode}`"
                " }).then(response => {"
                "   if (!response.ok) throw new Error('Network response was not ok.');"
                " }).catch(error => console.error('There has been a problem with your fetch operation:', error));"
                "}"
                "</script>";

  // for (int i = 1; i <= 5; i++) {
  //   html += "<div>滑条 " + String(i) + ": <input type='range' min='0' max='100' onchange='updateSlider(" + String(i) + ", this.value)' />";
  //   html += "<span id='value" + String(i) + "'>0</span></div>";
  // }
  html += "<div>感觉 级数 0~15: <input type='range' min='0' max='20' step='1' value='" + String(slider_temp[0]) + "' onchange='updateSlider(1, this.value)' />";
  html += "<span id='value1'>" + String(slider_temp[0]) + "</span></div>";

  html += "<div>触发脚 脉宽 us: <input type='range' min='0' max='200' step='1' value='" + String(slider_temp[1]) + "' onchange='updateSlider(2, this.value)' />";
  html += "<span id='value2'>" + String(slider_temp[1]) + "</span></div>";

  html += "<div>触发脚 周期 ms: <input type='range' min='0' max='30' step='1' value='" + String(slider_temp[2]) + "' onchange='updateSlider(3, this.value)' />";
  html += "<span id='value3'>" + String(slider_temp[2]) + "</span></div>";

  html += "<div>触发脚 触发/感觉 个: <input type='range' min='0' max='30' step='1' value='" + String(slider_temp[3]) + "' onchange='updateSlider(4, this.value)' />";
  html += "<span id='value4'>" + String(slider_temp[3]) + "</span></div>";

  html += "<div>触发每次感觉 周期 ms: <input type='range' min='0' max='500' step='1' value='" + String(slider_temp[4]) + "' onchange='updateSlider(5, this.value)' />";
  html += "<span id='value5'>" + String(slider_temp[4]) + "</span></div>";

  // 根据 switch_temp 的值动态设置按钮的选中状态
  String checkedStr = switch_temp == 1 ? "checked" : "";
  html += "<div><input type='checkbox' id='switch' onchange='updateSwitch(this.checked)' " + checkedStr + " /> 安全磁🥰贴哦</div>";

  // 添加下拉框的HTML代码
  html += "<div>模式选择: <select id='mode' onchange='updateMode(this.value)'>";
  String modes[] = {"Option 0", "Option 1", "Option 2", "Option 3"};
  for (String mode : modes) {
      html += "<option value='" + mode + "'" + (mode.equals(selectedMode_temp) ? " selected" : "") + ">" + mode + "</option>";
  }
  html += "</select></div>";

  html += "</body></html>";

  return html;
}
