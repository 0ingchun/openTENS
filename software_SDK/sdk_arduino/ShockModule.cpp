#include "ShockModule.hpp"

#if defined(ARDUINO_ARCH_ESP32)
#include "esp32-hal-ledc.h"
#endif

namespace shock {

ShockModule::ShockModule(Pins_t pins, boostConfig_t boost)
    : pins_(pins), boost_(boost) {
  // 规范化构造参数，后续逻辑即可假定参数范围有效。
  setboostConfig(boost_);
  // 保留默认刺激参数，但仍走一遍规范化流程。
  setSenseParams(sense_);
}

void ShockModule::initModule() {
  // 配置升压 PWM 输出脚和两路 H 桥触发脚为输出。
  pinMode(pins_.boostPin, OUTPUT);
  pinMode(pins_.electrodePosPin, OUTPUT);
  pinMode(pins_.electrodeNegPin, OUTPUT);

  // 在任何刺激输出前，强制 H 桥保持空闲状态。
  setHBridgeOpen();

  // 配置 PWM 后端，然后保持升压输出关闭。
  setBoostFrequency(boost_.boostFreqHz);
  setBoostDuty(0.0f);
}

void ShockModule::runSenseUnit() {
  // 一个刺激单元内，重复执行“升压 -> H 桥触发 -> 触发间隔”。
  for (uint16_t i = 0; i < sense_.triggerCount; ++i) {
    boostOnce();
    triggerOnce(sense_.triggerMode);
    delay(sense_.triggerGapMs);
  }
  delay(sense_.senseDelayMs);
}

void ShockModule::stopModule() {
  setBoostDuty(0.0f);
  setHBridgeOpen();
}

void ShockModule::unmountModule() {
  stopModule();
  releaseHBridgeResidualCharge(1000U);
}

void ShockModule::setBoostFrequency(uint32_t hz) {
  if (hz == 0U) {
    hz = 1U;
  }

  // 按开发板架构选择对应的 PWM 后端。
#if defined(ARDUINO_ARCH_AVR)
  // Keep the default Arduino PWM timer configuration on AVR.
  // AVR 上 analogWrite() 的 PWM 频率受定时器配置限制，这里不动态改定时器。
#warning "Boost PWM frequency on AVR uses the default analogWrite timer configuration; requested boostFreqHz is not applied."
  uint32_t pwmPeriodCycles = 0U;
  switch (digitalPinToTimer(pins_.boostPin)) {
    case TIMER0A:
    case TIMER0B:
      // Timer0 is configured as 8-bit fast PWM by the Arduino AVR core.
      pwmPeriodCycles = 64UL * 256UL;
      break;
    case TIMER1A:
    case TIMER1B:
    case TIMER1C:
    case TIMER2:
    case TIMER2A:
    case TIMER2B:
    case TIMER3A:
    case TIMER3B:
    case TIMER3C:
    case TIMER4A:
    case TIMER4B:
    case TIMER4C:
    case TIMER4D:
    case TIMER5A:
    case TIMER5B:
    case TIMER5C:
      // Other standard AVR PWM timers use 8-bit phase-correct PWM.
      pwmPeriodCycles = 64UL * 510UL;
      break;
    default:
      // No hardware PWM on this pin; keep software timing tied to the request.
      boost_.boostFreqHz = hz;
      boost_.boostPeriodUs = 1000000UL / hz;
      if (boost_.boostPeriodUs == 0U) {
        boost_.boostPeriodUs = 1U;
      }
      break;
  }

  if (pwmPeriodCycles > 0U) {
    boost_.boostFreqHz = F_CPU / pwmPeriodCycles;
    boost_.boostPeriodUs = clockCyclesToMicroseconds(pwmPeriodCycles);
    if (boost_.boostPeriodUs == 0U) {
      boost_.boostPeriodUs = 1U;
    }
  }
  analogWrite(pins_.boostPin, 0);
#elif defined(ARDUINO_ARCH_ESP8266)
  boost_.boostFreqHz = hz;
  boost_.boostPeriodUs = 1000000UL / hz;
  if (boost_.boostPeriodUs == 0U) {
    boost_.boostPeriodUs = 1U;
  }
  analogWrite(pins_.boostPin, 0);
  analogWriteFreq(hz);
  analogWriteRange(100U);
#elif defined(ARDUINO_ARCH_ESP32)
  boost_.boostFreqHz = hz;
  boost_.boostPeriodUs = 1000000UL / hz;
  if (boost_.boostPeriodUs == 0U) {
    boost_.boostPeriodUs = 1U;
  }
  ledcDetachPin(pins_.boostPin);
  ledcSetup(pins_.ledcChannel, hz, 8);
  ledcAttachPin(pins_.boostPin, pins_.ledcChannel);
  ledcWrite(pins_.ledcChannel, 0);
#else
#error "Unsupported architecture! This library only supports AVR, ESP8266, and ESP32."
#endif
}

void ShockModule::setBoostDuty(float dutyPercent) {
  const float duty = clampPercent(dutyPercent);

  // 将 [0, 100]% 转换为各平台 analogWrite/LEDC 的输出范围。
#if defined(ARDUINO_ARCH_AVR)
  analogWrite(pins_.boostPin, dutyFromPercent(duty, 255U));
#elif defined(ARDUINO_ARCH_ESP8266)
  analogWrite(pins_.boostPin, dutyFromPercent(duty, 100U));
#elif defined(ARDUINO_ARCH_ESP32)
  // ESP32 LEDC 的占空比范围由 setup 时的 bit 数决定，这里在setBoostFrequency中已经设置是 8 位（0-255）。
  ledcWrite(pins_.ledcChannel, dutyFromPercent(duty, 255U));
#else
#error "Unsupported architecture! This library only supports AVR, ESP8266, and ESP32."
#endif
}

void ShockModule::boostOnce() {
    // 打开升压 PWM，等待指定周期数后关闭。
  setBoostDuty(boost_.boostDutyPercent);
  for (uint16_t i = 0; i < boostCount_; ++i) {
    delayMicroseconds(boost_.boostPeriodUs); // 为什么原来要要+ 1U 微秒？--- IGNORE ---
  }
  setBoostDuty(0.0f);
}

void ShockModule::setboostConfig(const boostConfig_t& config) {
  boost_ = config;

  // 防止非法 0 值破坏后续时序计算。
  if (boost_.boostFreqHz == 0U) {
    boost_.boostFreqHz = 1U;
  }
  if (boost_.boostUnitsPerLevel == 0U) {
    boost_.boostUnitsPerLevel = 1U;
  }

  // 刷新实际升压脉冲数，随强度等级增加。
  boostCount_ = static_cast<uint16_t>(boost_.boostUnitsPerLevel * sense_.level);

  //主动更新频率设置，确保运行时修改配置后立即生效。
  setBoostFrequency(boost_.boostFreqHz);
  boost_.boostDutyPercent = clampPercent(boost_.boostDutyPercent);
}

void ShockModule::setHBridgeTrigger(bool direction) {
  // 正向：正端高、负端低；反向：正端低、负端高。
  digitalWrite(pins_.electrodePosPin, direction ? HIGH : LOW);
  digitalWrite(pins_.electrodeNegPin, direction ? LOW : HIGH);
}

void ShockModule::setHBridgeShort() {
  // H 桥短路：正负端同时高电平。
  digitalWrite(pins_.electrodePosPin, HIGH);
  digitalWrite(pins_.electrodeNegPin, HIGH);
}

void ShockModule::setHBridgeOpen() {
  // H 桥开路：正负端同时低电平。
  digitalWrite(pins_.electrodePosPin, LOW);
  digitalWrite(pins_.electrodeNegPin, LOW);
}

void ShockModule::releaseHBridgeResidualCharge(uint32_t durationUs) {
  setHBridgeShort();
  if (durationUs > 0U) {
    delayMicroseconds(durationUs);
  }
  setHBridgeOpen();
}

void ShockModule::triggerOnce(triggerMode_e triggerMode) {
  // 根据触发模式决定本次 H 桥输出极性。
  bool positiveFirst = true;
  switch (triggerMode) {
    case triggerMode_e::AcBidirectional:
      // 双向模式下，每次触发交替切换极性。
      nextPolarityHigh_ = !nextPolarityHigh_;
      positiveFirst = nextPolarityHigh_;
      break;
    case triggerMode_e::DcForward:
      positiveFirst = true;
      break;
    case triggerMode_e::DcReverse:
      positiveFirst = false;
      break;
    default:
      positiveFirst = true;
      break;
  }

  setHBridgeTrigger(positiveFirst);

  // 脉宽，触发持续时间
  delayMicroseconds(sense_.triggerWidthUs);

  // 触发结束后恢复空闲状态。
  setHBridgeOpen();
}

void ShockModule::setSenseParams(const SenseParams_t& params) {
  sense_ = params;

  // // 系统安全限制，先预留接口
  // if (sense_.level >= 0U) {
  //   sense_.level = MAX_LEVEL_LIMIT;
  // }

  // 刷新实际升压脉冲数，随强度等级增加。
  boostCount_ = static_cast<uint16_t>(boost_.boostUnitsPerLevel * sense_.level);
}

void ShockModule::setSenseFromLegacyArray(const int values[5]) {
  if (values == nullptr) {
    return;
  }

  SenseParams_t p = sense_;
  // 旧版数组字段顺序：
  // [0] 强度等级
  // [1] 触发脉冲宽度
  // [2] 触发间隔
  // [3] 触发次数
  // [4] 刺激单元间隔
  p.level = values[0] > 0 ? static_cast<uint8_t>(values[0]) : 0U;
  p.triggerWidthUs = values[1] > 0 ? static_cast<uint32_t>(values[1]) : 0U;
  p.triggerGapMs = values[2] > 0 ? static_cast<uint16_t>(values[2]) : 0U;
  p.triggerCount = values[3] > 0 ? static_cast<uint16_t>(values[3]) : 0U;
  p.senseDelayMs = values[4] > 0 ? static_cast<uint16_t>(values[4]) : 0U;
  setSenseParams(p);
}

float ShockModule::clampPercent(float value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  if (value > 100.0f) {
    return 100.0f;
  }
  return value;
}

uint32_t ShockModule::dutyFromPercent(float percent, uint32_t outMax) {
  return static_cast<uint32_t>((percent * static_cast<float>(outMax)) / 100.0f);
}

}  // namespace shock
