# ShockModule Arduino 驱动说明

`ShockModule` 是一个用于控制单通道刺激模块的 Arduino C++ 驱动，负责两部分工作：

- 升压阶段：通过 PWM 驱动升压脚。
- 触发阶段：通过两路 H 桥控制脚输出正向、反向或交替极性的刺激脉冲。

当前支持的平台：

- AVR
- ESP8266
- ESP32

## 文件

- `include/ShockModule.hpp`：类型定义和公开接口。
- `src/ShockModule.cpp`：具体实现。
- `src/main.cpp`：当前工程里的最小使用示例。

## 最小使用流程

```cpp
#include "ShockModule.hpp"

shock::Pins_t stimPins{
    6,  // boostPin
    7,  // electrodePosPin
    8,  // electrodeNegPin
#if defined(ARDUINO_ARCH_ESP32)
    0,  // ledcChannel
#endif
};

shock::boostConfig_t boostCfg;
shock::ShockModule stim(stimPins, boostCfg);

void setup() {
  stim.initModule();
}

void loop() {
  stim.runSenseUnit();
}
```

调用顺序建议：

1. 定义 `Pins_t`。
2. 按需定义 `boostConfig_t`。
3. 创建 `ShockModule` 对象。
4. 在 `setup()` 中调用 `initModule()`。
5. 用 `setSenseParams()` 设置刺激参数。
6. 周期性调用 `runSenseUnit()`。

## 引脚配置

```cpp
struct Pins_t {
  uint8_t boostPin;
  uint8_t electrodePosPin;
  uint8_t electrodeNegPin;
#if defined(ARDUINO_ARCH_ESP32)
  uint8_t ledcChannel = 0;
#endif
};
```

- `boostPin`：升压 PWM 输出脚。
- `electrodePosPin`：H 桥正向控制脚。
- `electrodeNegPin`：H 桥反向控制脚。
- `ledcChannel`：仅 ESP32 使用。

## 升压配置

```cpp
struct boostConfig_t {
  uint32_t boostFreqHz = 25000;
  uint32_t boostPeriodUs = 40;
  float boostDutyPercent = 75.0f;
  uint16_t boostUnitsPerLevel = 8;
};
```

- `boostFreqHz`：期望的 PWM 频率。
- `boostPeriodUs`：单个升压周期的延时，运行时会随频率自动更新。
- `boostDutyPercent`：升压 PWM 占空比，范围 `0~100`。
- `boostUnitsPerLevel`：每一级强度对应多少个升压周期。

升压阶段实际执行时长大致为：

```text
boostUnitsPerLevel * level * boostPeriodUs
```

其中 `level` 来自 `SenseParams_t`。

### AVR 说明

在 AVR 平台上，`analogWrite()` 的 PWM 频率由 Arduino core 的定时器配置决定，`setBoostFrequency()` 不会改动硬件定时器。

驱动会自动读取 `boostPin` 所属的硬件定时器，并把：

- `boostFreqHz`
- `boostPeriodUs`

更新为该板子、该引脚对应的实际值。

例如在常见 16 MHz ATmega328P 板上：

- Timer0 PWM 引脚通常约为 `976 Hz`，周期约 `1024 us`
- 其他标准 PWM 引脚通常约为 `490 Hz`，周期约 `2040 us`

这样 `boostOnce()` 使用的软件延时会尽量和实际 PWM 周期保持一致。

## 刺激参数

```cpp
struct SenseParams_t {
  uint8_t level = 1;
  uint32_t triggerWidthUs = 70;
  uint16_t triggerGapMs = 1;
  uint16_t triggerCount = 10;
  uint16_t senseDelayMs = 1;
  triggerMode_e triggerMode = triggerMode_e::AcBidirectional;
};
```

- `level`：刺激强度等级，会影响升压周期数。
- `triggerWidthUs`：单次 H 桥触发持续时间。
- `triggerGapMs`：两次触发之间的间隔。
- `triggerCount`：一个刺激单元内重复多少次。
- `senseDelayMs`：一个刺激单元结束后的等待时间。
- `triggerMode`：触发极性模式。

示例：

```cpp
shock::SenseParams_t params;
params.level = 3;
params.triggerWidthUs = 70;
params.triggerGapMs = 1;
params.triggerCount = 10;
params.senseDelayMs = 1;
params.triggerMode = shock::triggerMode_e::DcForward;

stim.setSenseParams(params);
stim.runSenseUnit();
```

## 触发模式

```cpp
enum class triggerMode_e : uint8_t {
  AcBidirectional,
  DcForward,
  DcReverse,
};
```

- `AcBidirectional`：每次触发时自动交替正反极性。
- `DcForward`：始终正向输出。
- `DcReverse`：始终反向输出。

## 一个刺激单元做了什么

`runSenseUnit()` 的逻辑是：

1. 执行一次 `boostOnce()`。
2. 按当前模式执行一次 `triggerOnce()`。
3. 等待 `triggerGapMs`。
4. 重复以上流程 `triggerCount` 次。
5. 最后等待 `senseDelayMs`。

等价流程：

```cpp
for (uint16_t i = 0; i < triggerCount; ++i) {
  boostOnce();
  triggerOnce(triggerMode);
  delay(triggerGapMs);
}
delay(senseDelayMs);
```

## 常用接口

```cpp
void initModule();
void runSenseUnit();
void stopModule();
void unmountModule();

void setBoostFrequency(uint32_t hz);
void setBoostDuty(float dutyPercent);
void boostOnce();
void setboostConfig(const boostConfig_t& config);

void setSenseParams(const SenseParams_t& params);
const SenseParams_t& senseParams() const;
const boostConfig_t& getBoostConfig() const;

void setHBridgeTrigger(bool direction);
void setHBridgeShort();
void setHBridgeOpen();
void releaseHBridgeResidualCharge(uint32_t durationUs);
void triggerOnce(triggerMode_e triggerMode);
```

### 生命周期相关

- `initModule()`：初始化引脚和 PWM，建议只在启动时调用一次。
- `stopModule()`：关闭升压输出，并让 H 桥进入开路空闲状态。
- `unmountModule()`：先停止输出，再短接 H 桥一段时间释放残余电荷。

### 运行时修改配置

- `setboostConfig()`：修改升压配置，会重新计算频率相关参数。
- `setSenseParams()`：修改刺激参数，并刷新实际升压周期数。
- `setSenseFromLegacyArray()`：兼容旧数组格式：

```cpp
{ level, triggerWidthUs, triggerGapMs, triggerCount, senseDelayMs }
```

## 完整示例

```cpp
#include <Arduino.h>
#include "ShockModule.hpp"

shock::Pins_t stimPins{
    6,
    7,
    8,
#if defined(ARDUINO_ARCH_ESP32)
    0,
#endif
};

shock::boostConfig_t boostCfg;
shock::ShockModule stim(stimPins, boostCfg);

shock::SenseParams_t forwardPattern() {
  shock::SenseParams_t p;
  p.level = 3;
  p.triggerWidthUs = 70;
  p.triggerGapMs = 1;
  p.triggerCount = 10;
  p.senseDelayMs = 1;
  p.triggerMode = shock::triggerMode_e::DcForward;
  return p;
}

void setup() {
  stim.initModule();
  stim.setSenseParams(forwardPattern());
}

void loop() {
  stim.runSenseUnit();
}
```

## 使用注意

- `boostPin` 应优先选择具备硬件 PWM 的引脚。
- AVR 上请求的 `boostFreqHz` 不一定会成为真实 PWM 频率，真实值取决于板型和所选引脚。
- `boostDutyPercent` 会被限制在 `0~100`。
- `boostFreqHz`、`boostUnitsPerLevel` 若传入 `0`，驱动会自动修正为 `1`。
- `stopModule()` 只关闭输出，不释放残余电荷；如果需要释放，请调用 `unmountModule()` 或 `releaseHBridgeResidualCharge()`。

