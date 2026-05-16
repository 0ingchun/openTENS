#include <Arduino.h>

#include "ShockModule.hpp"

namespace {

constexpr uint8_t kLedPin = 13;
constexpr uint8_t kRepeatCount = 50;

enum class DemoPhase : uint8_t {
  Forward = 0,
  Reverse,
};

shock::SenseParams makeSensePattern1() {
  shock::SenseParams p;
  p.level = 12;
  p.triggerWidthUs = 75;
  p.triggerPeriodMs = 1;
  p.triggerCount = 50;
  p.sensePeriodMs = 1;
  p.outputMode = shock::OutputMode::AcBidirectional;
  return p;
}

shock::SenseParams makeSensePattern2() {
  shock::SenseParams p;
  p.level = 12;
  p.triggerWidthUs = 75;
  p.triggerPeriodMs = 1;
  p.triggerCount = 50;
  p.sensePeriodMs = 1;
  p.outputMode = shock::OutputMode::AcBidirectional;
  return p;
}

shock::Pins stimPins{
    6,  // boostPin
    7,  // netPosPin
    8,  // netNegPin
#if defined(ARDUINO_ARCH_ESP32)
    0,  // ledcChannel
#endif
};

shock::FixedConfig fixedCfg;
shock::ShockModule stim(stimPins, fixedCfg);

const shock::SenseParams kPattern1 = makeSensePattern1();
const shock::SenseParams kPattern2 = makeSensePattern2();

DemoPhase demoPhase = DemoPhase::Forward;
uint16_t phaseRunCount = 0U;

void startNextSenseUnit() {
  if (demoPhase == DemoPhase::Forward) {
    digitalWrite(kLedPin, HIGH);
    if (stim.startSenseUnit(kPattern1)) {
      ++phaseRunCount;
      if (phaseRunCount >= kRepeatCount) {
        demoPhase = DemoPhase::Reverse;
        phaseRunCount = 0U;
      }
    }
    return;
  }

  digitalWrite(kLedPin, LOW);
  if (stim.startSenseUnit(kPattern2)) {
    ++phaseRunCount;
    if (phaseRunCount >= kRepeatCount) {
      demoPhase = DemoPhase::Forward;
      phaseRunCount = 0U;
    }
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(kLedPin, OUTPUT);
  digitalWrite(kLedPin, LOW);

  stim.begin();
  startNextSenseUnit();
}

void loop() {
  stim.update();
  if (!stim.isBusy()) {
    startNextSenseUnit();
  }
}
