#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace shock {

// Hardware pin mapping for one stimulation channel.
struct Pins {
  // PWM output pin for boost control.
  uint8_t boostPin;
  // H-bridge positive output control pin.
  uint8_t netPosPin;
  // H-bridge negative output control pin.
  uint8_t netNegPin;
#if defined(ARDUINO_ARCH_ESP32)
  // ESP32 LEDC channel used by boostPin PWM.
  uint8_t ledcChannel = 0;
#endif
};

// Mostly static parameters. Usually configured once at startup.
struct FixedConfig {
  // Boost PWM period in microseconds.
  uint32_t boostPeriodUs = 40;
  // Boost PWM frequency in Hz.
  uint32_t boostFreqHz = 25000;
  // Boost PWM duty in percent [0, 100].
  float boostDutyPercent = 75.0f;
  // Number of boost periods per level unit.
  uint16_t boostUnitsPerLevel = 8;
};

// AC trigger output mode.
enum class OutputMode : uint8_t {
  AcBidirectional = 0,  // Alternate polarity every trigger
  AcForward = 1,        // Always P=HIGH, N=LOW during trigger pulse
  AcReverse = 2,        // Always P=LOW, N=HIGH during trigger pulse
};

// Runtime stimulation parameters. Can be changed per pattern.
struct SenseParams {
  // Intensity level, affects boost pulse count.
  uint8_t level = 1;
  // Output trigger pulse width in microseconds.
  uint32_t triggerWidthUs = 70;
  // Interval between two trigger events in milliseconds.
  uint16_t triggerPeriodMs = 1;
  // Number of trigger events in one sense unit.
  uint16_t triggerCount = 10;
  // Delay after one sense unit is finished in milliseconds.
  uint16_t sensePeriodMs = 1;
  // Output mode for AC trigger pulse.
  OutputMode outputMode = OutputMode::AcBidirectional;
};

// Object-oriented stim driver:
// 1) configure pins and params
// 2) begin()
// 3) setSenseParams(...)
// 4) runSenseUnit()
class ShockModule {
 public:
  explicit ShockModule(Pins pins, FixedConfig fixed = FixedConfig());

  // Configure GPIO/PWM and force boost output off.
  void begin();

  // Update fixed parameters with value sanitization.
  void setFixedConfig(const FixedConfig& config);
  const FixedConfig& fixedConfig() const { return fixed_; }

  // Update runtime sense parameters.
  void setSenseParams(const SenseParams& params);
  const SenseParams& senseParams() const { return sense_; }

  // Legacy order mapping:
  // { level, triggerWidthUs, triggerPeriodMs, triggerCount, sensePeriodMs }
  void setSenseFromLegacyArray(const int values[5]);

  void setBoostFrequency(uint32_t hz);
  void setBoostDuty(float dutyPercent);
  // Force H-bridge control pins HIGH/HIGH (discharge mode).
  void setHBridgeDischarge();
  // Force H-bridge control pins LOW/LOW (high-impedance mode in this design).
  void setHBridgeHighZ();
  // One-shot residual charge release: discharge for durationUs, then high-Z.
  void releaseResidualCharge(uint32_t durationUs = 200U);

  // Start one sense unit and return immediately.
  // Returns false if a sense unit is already running.
  bool startSenseUnit();
  bool startSenseUnit(const SenseParams& params);
  // Advance non-blocking state machine. Call this frequently in loop().
  void update();
  bool isBusy() const;
  // Stop current output immediately and return to idle.
  void stop();

  // Run boost phase once: duty on -> wait N periods -> duty off.
  void boostOnce();
  // Biphasic trigger via two output pins.
  void triggerAC(bool positiveFirst);
  // Monophasic trigger on negative pin only.
  void triggerDC(bool activeLevel);
  // One full sense unit: repeated (boost + AC trigger) then final delay.
  void runSenseUnit();

 private:
  enum class RuntimeState : uint8_t {
    Idle = 0,
    BoostOn,
    TriggerOn,
    InterTriggerWait,
    SenseWait,
  };

  static float clampPercent(float value);
  static uint32_t dutyFromPercent(float percent, uint32_t outMax);
  static bool timeReached(uint32_t nowUs, uint32_t deadlineUs);
  static uint32_t msToUs(uint16_t ms);

  bool resolvePositiveFirst();
  void enterBoostOn(uint32_t nowUs);
  void enterTriggerOn(uint32_t nowUs);
  void enterInterTriggerWait(uint32_t nowUs);
  void enterSenseWait(uint32_t nowUs);

  // Construction-time pin map.
  Pins pins_;
  // Fixed config used by boost stage.
  FixedConfig fixed_;
  // Runtime config used by trigger stage.
  SenseParams sense_;

  // Cached boost pulse count for current run.
  uint16_t boostCount_ = 0;
  // Current trigger index inside one sense unit.
  uint16_t triggerIndex_ = 0;
  // State machine runtime state.
  RuntimeState state_ = RuntimeState::Idle;
  // State transition deadline in microseconds (micros() timebase).
  uint32_t stateDeadlineUs_ = 0;
  // Internal AC polarity toggle state.
  bool nextPolarityHigh_ = false;
};

}  // namespace shock
