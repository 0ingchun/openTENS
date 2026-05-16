#include "ShockModule.hpp"

#if defined(ARDUINO_ARCH_ESP32)
#include "esp32-hal-ledc.h"
#endif

namespace shock {

ShockModule::ShockModule(Pins pins, FixedConfig fixed)
    : pins_(pins), fixed_(fixed) {
  // Normalize constructor input so later logic can assume valid ranges.
  setFixedConfig(fixed_);
  // Keep default sense_ but run through normalization path.
  setSenseParams(sense_);
}

void ShockModule::begin() {
  // Output pins for PWM boost and two trigger legs.
  pinMode(pins_.boostPin, OUTPUT);
  pinMode(pins_.netPosPin, OUTPUT);
  pinMode(pins_.netNegPin, OUTPUT);

  // Configure PWM backend then clear residual charge once.
  setBoostFrequency(fixed_.boostFreqHz);
  releaseResidualCharge();
  stop();
}

void ShockModule::setFixedConfig(const FixedConfig& config) {
  fixed_ = config;

  // Guard against illegal zero values that would break timing math.
  if (fixed_.boostPeriodUs == 0U) {
    fixed_.boostPeriodUs = 1U;
  }
  if (fixed_.boostFreqHz == 0U) {
    fixed_.boostFreqHz = 1U;
  }
  if (fixed_.boostUnitsPerLevel == 0U) {
    fixed_.boostUnitsPerLevel = 1U;
  }

  fixed_.boostDutyPercent = clampPercent(fixed_.boostDutyPercent);
}

void ShockModule::setSenseParams(const SenseParams& params) {
  sense_ = params;

  // Keep a minimum non-zero level (matches original C behavior intent).
  if (sense_.level == 0U) {
    sense_.level = 1U;
  }
}

void ShockModule::setSenseFromLegacyArray(const int values[5]) {
  if (values == nullptr) {
    return;
  }

  SenseParams p = sense_;
  // Legacy field order:
  // [0] level
  // [1] triggerWidthUs
  // [2] triggerPeriodMs
  // [3] triggerCount
  // [4] sensePeriodMs
  p.level = values[0] > 0 ? static_cast<uint8_t>(values[0]) : 1U;
  p.triggerWidthUs = values[1] > 0 ? static_cast<uint32_t>(values[1]) : 0U;
  p.triggerPeriodMs = values[2] > 0 ? static_cast<uint16_t>(values[2]) : 0U;
  p.triggerCount = values[3] > 0 ? static_cast<uint16_t>(values[3]) : 0U;
  p.sensePeriodMs = values[4] > 0 ? static_cast<uint16_t>(values[4]) : 0U;
  setSenseParams(p);
}

void ShockModule::setBoostFrequency(uint32_t hz) {
  if (hz == 0U) {
    hz = 1U;
  }
  fixed_.boostFreqHz = hz;

  // Board-specific PWM backend.
#if defined(ARDUINO_ARCH_AVR)
  // On AVR, keeping default timer setup to avoid touching timer registers.
#elif defined(ARDUINO_ARCH_ESP8266)
  analogWrite(pins_.boostPin, 0);
  analogWriteFreq(hz);
  analogWriteRange(100);
#elif defined(ARDUINO_ARCH_ESP32)
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

  // Convert [0,100]% to backend-specific duty range.
#if defined(ARDUINO_ARCH_AVR)
  analogWrite(pins_.boostPin, dutyFromPercent(duty, 255U));
#elif defined(ARDUINO_ARCH_ESP8266)
  analogWrite(pins_.boostPin, dutyFromPercent(duty, 100U));
#elif defined(ARDUINO_ARCH_ESP32)
  ledcWrite(pins_.ledcChannel, dutyFromPercent(duty, 255U));
#else
#error "Unsupported architecture! This library only supports AVR, ESP8266, and ESP32."
#endif
}

void ShockModule::setHBridgeDischarge() {
  digitalWrite(pins_.netPosPin, HIGH);
  digitalWrite(pins_.netNegPin, HIGH);
}

void ShockModule::setHBridgeHighZ() {
  digitalWrite(pins_.netPosPin, LOW);
  digitalWrite(pins_.netNegPin, LOW);
}

void ShockModule::releaseResidualCharge(uint32_t durationUs) {
  setHBridgeDischarge();
  if (durationUs > 0U) {
    delayMicroseconds(durationUs);
  }
  setHBridgeHighZ();
}

bool ShockModule::startSenseUnit() {
  if (isBusy()) {
    return false;
  }

  triggerIndex_ = 0U;
  const uint32_t nowUs = micros();

  if (sense_.triggerCount == 0U) {
    enterSenseWait(nowUs);
    return true;
  }

  enterBoostOn(nowUs);
  return true;
}

bool ShockModule::startSenseUnit(const SenseParams& params) {
  setSenseParams(params);
  return startSenseUnit();
}

void ShockModule::update() {
  if (!isBusy()) {
    return;
  }

  const uint32_t nowUs = micros();
  if (!timeReached(nowUs, stateDeadlineUs_)) {
    return;
  }

  switch (state_) {
    case RuntimeState::BoostOn:
      setBoostDuty(0.0f);
      enterTriggerOn(nowUs);
      break;

    case RuntimeState::TriggerOn:
      setHBridgeHighZ();

      ++triggerIndex_;
      if (triggerIndex_ < sense_.triggerCount) {
        enterInterTriggerWait(nowUs);
      } else {
        enterSenseWait(nowUs);
      }
      break;

    case RuntimeState::InterTriggerWait:
      enterBoostOn(nowUs);
      break;

    case RuntimeState::SenseWait:
      state_ = RuntimeState::Idle;
      break;

    case RuntimeState::Idle:
    default:
      break;
  }
}

bool ShockModule::isBusy() const { return state_ != RuntimeState::Idle; }

void ShockModule::stop() {
  setBoostDuty(0.0f);
  setHBridgeHighZ();///？？？？

  triggerIndex_ = 0U;
  stateDeadlineUs_ = 0U;
  state_ = RuntimeState::Idle;
}

void ShockModule::boostOnce() {
  // Effective pulse count grows with level.
  boostCount_ = static_cast<uint16_t>(fixed_.boostUnitsPerLevel * sense_.level);

  // Enable boost PWM, wait enough periods, then disable PWM.
  setBoostDuty(fixed_.boostDutyPercent);
  for (uint16_t i = 0; i < boostCount_; ++i) {
    delayMicroseconds(fixed_.boostPeriodUs + 1U);
  }
  setBoostDuty(0.0f);
}

void ShockModule::triggerAC(bool positiveFirst) {
  // Differential polarity:
  // positiveFirst=true  => P=HIGH, N=LOW
  // positiveFirst=false => P=LOW,  N=HIGH
  digitalWrite(pins_.netPosPin, positiveFirst ? HIGH : LOW);
  digitalWrite(pins_.netNegPin, positiveFirst ? LOW : HIGH);

  delayMicroseconds(sense_.triggerWidthUs);

  // Return both trigger pins to idle state.
  setHBridgeHighZ();
}

void ShockModule::triggerDC(bool activeLevel) {
  digitalWrite(pins_.netNegPin, activeLevel ? HIGH : LOW);
  delayMicroseconds(sense_.triggerWidthUs);
  setHBridgeHighZ();
}

void ShockModule::runSenseUnit() {
  // Blocking compatibility wrapper on top of non-blocking state machine.
  if (!startSenseUnit()) {
    return;
  }
  while (isBusy()) {
    update();
  }
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

bool ShockModule::timeReached(uint32_t nowUs, uint32_t deadlineUs) {
  return static_cast<int32_t>(nowUs - deadlineUs) >= 0;
}

uint32_t ShockModule::msToUs(uint16_t ms) {
  return static_cast<uint32_t>(ms) * 1000UL;
}

bool ShockModule::resolvePositiveFirst() {
  bool positiveFirst = true;
  switch (sense_.outputMode) {
    case OutputMode::AcBidirectional:
      nextPolarityHigh_ = !nextPolarityHigh_;
      positiveFirst = nextPolarityHigh_;
      break;
    case OutputMode::AcForward:
      positiveFirst = true;
      break;
    case OutputMode::AcReverse:
      positiveFirst = false;
      break;
    default:
      positiveFirst = true;
      break;
  }
  return positiveFirst;
}

void ShockModule::enterBoostOn(uint32_t nowUs) {
  boostCount_ = static_cast<uint16_t>(fixed_.boostUnitsPerLevel * sense_.level);
  setBoostDuty(fixed_.boostDutyPercent);

  const uint32_t boostDurationUs =
      static_cast<uint32_t>(boostCount_) * (fixed_.boostPeriodUs + 1U);
  stateDeadlineUs_ = nowUs + boostDurationUs;
  state_ = RuntimeState::BoostOn;
}

void ShockModule::enterTriggerOn(uint32_t nowUs) {
  const bool positiveFirst = resolvePositiveFirst();
  digitalWrite(pins_.netPosPin, positiveFirst ? HIGH : LOW);
  digitalWrite(pins_.netNegPin, positiveFirst ? LOW : HIGH);

  stateDeadlineUs_ = nowUs + sense_.triggerWidthUs;
  state_ = RuntimeState::TriggerOn;
}

void ShockModule::enterInterTriggerWait(uint32_t nowUs) {
  stateDeadlineUs_ = nowUs + msToUs(sense_.triggerPeriodMs);
  state_ = RuntimeState::InterTriggerWait;
}

void ShockModule::enterSenseWait(uint32_t nowUs) {
  stateDeadlineUs_ = nowUs + msToUs(sense_.sensePeriodMs);
  state_ = RuntimeState::SenseWait;
}

}  // namespace shock
