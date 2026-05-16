#include <Arduino.h>

#include "ShockModule.hpp"

namespace {

constexpr uint8_t kLedPin = 13;
constexpr uint8_t kRepeatCount = 100;

shock::SenseParams_t makeSensePattern1() {
  shock::SenseParams_t p;
  p.level = 3;
  p.triggerWidthUs = 70;
  p.triggerGapMs = 1;
  p.triggerCount = 10;
  p.senseDelayMs = 1;
  p.triggerMode = shock::triggerMode_e::DcForward;
  return p;
}

shock::SenseParams_t makeSensePattern2() {
  shock::SenseParams_t p;
  p.level = 3;
  p.triggerWidthUs = 70;
  p.triggerGapMs = 1;
  p.triggerCount = 10;
  p.senseDelayMs = 1;
  p.triggerMode = shock::triggerMode_e::DcReverse;
  return p;
}

shock::Pins_t stimPins{
    6,  // boostPin
    7,  // electrodePosPin
    8,  // electrodeNegPin
#if defined(ARDUINO_ARCH_ESP32)
    0,  // ledcChannel
#endif
};

shock::boostConfig_t fixedCfg;
shock::ShockModule stim(stimPins, fixedCfg);

const shock::SenseParams_t kPattern1 = makeSensePattern1();
const shock::SenseParams_t kPattern2 = makeSensePattern2();

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(kLedPin, OUTPUT);
  digitalWrite(kLedPin, LOW);

  stim.initModule();
}

void loop() {
  // 10 times forward DC output.
  digitalWrite(kLedPin, HIGH);
  stim.setSenseParams(kPattern1);
  for (uint8_t i = 0; i < kRepeatCount; ++i) {
    stim.runSenseUnit();
  }

  // 10 times reverse DC output.
  digitalWrite(kLedPin, LOW);
  stim.setSenseParams(kPattern2);
  for (uint8_t i = 0; i < kRepeatCount; ++i) {
    stim.runSenseUnit();
  }

}
