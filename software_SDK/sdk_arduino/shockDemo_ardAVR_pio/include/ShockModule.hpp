#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace shock {

// 单个刺激通道的硬件引脚映射。
struct Pins_t {
  // 升压 PWM 输出引脚。
  uint8_t boostPin;
  // H 桥正向输出控制引脚。
  uint8_t electrodePosPin;
  // H 桥反向输出控制引脚。
  uint8_t electrodeNegPin;
#if defined(ARDUINO_ARCH_ESP32)
  // ESP32 LEDC PWM 通道。
  uint8_t ledcChannel = 0;
#endif
};

// 升压阶段使用的固定配置，通常启动时配置一次。
struct boostConfig_t {
  // 升压 PWM 频率，单位：Hz；AVR 上会按板型引脚映射换成实际值。
  uint32_t boostFreqHz = 25000;
  // 单个升压周期延时，单位：微秒；由 boostFreqHz 自动换算。
  uint32_t boostPeriodUs = 40;
  // 升压 PWM 占空比，范围：[0, 100]。
  float boostDutyPercent = 75.0f;
  // 每个强度等级对应的升压周期数。
  uint16_t boostUnitsPerLevel = 8;
};

// H 桥触发输出模式。
enum class triggerMode_e : uint8_t {
  // 每次触发交替切换正反极性。
  AcBidirectional = 0,
  // 每次触发都使用正向输出。
  DcForward = 1,
  // 每次触发都使用反向输出。
  DcReverse = 2,
};

// 运行时刺激参数，可按模式动态修改。
struct SenseParams_t {
  // 刺激强度等级，会影响升压周期数。
  uint8_t level = 1;
  // H 桥触发脉冲宽度，单位：微秒。
  uint32_t triggerWidthUs = 70;
  // 两次触发事件之间的间隔，单位：毫秒。
  uint16_t triggerGapMs = 1;
  // 一个刺激单元内的触发次数。
  uint16_t triggerCount = 10;
  // 一个刺激单元结束后的等待时间，单位：毫秒。
  uint16_t senseDelayMs = 1;
  // H 桥触发输出模式。
  triggerMode_e triggerMode = triggerMode_e::AcBidirectional;
};

// 面向对象的刺激模块驱动。
class ShockModule {
 public:
  // 构造函数：保存引脚映射和升压固定配置。
  explicit ShockModule(Pins_t pins, boostConfig_t boost = boostConfig_t());

  // 初始化 IO 和 PWM 后端，并保持输出关闭。
  void initModule();
  // 执行一个完整刺激单元：升压、触发、间隔循环。
  void runSenseUnit();
  // 关闭升压和 H 桥输出。
  void stopModule();
  // 安全卸载模块，目前等价于 stopModule()。
  void unmountModule();

  // 设置升压 PWM 频率。
  void setBoostFrequency(uint32_t hz);
  // 设置升压 PWM 占空比。
  void setBoostDuty(float dutyPercent);
  // 执行一次升压阶段。
  void boostOnce();

  // 更新升压固定参数，并做范围校正。
  void setboostConfig(const boostConfig_t& config);
  // 只读获取当前升压配置。
  const shock::boostConfig_t& getBoostConfig() const { return boost_; }

  // 执行一次 H 桥触发。
  void setHBridgeTrigger(bool direction);
  // H 桥短路：正负端同时拉高，用于释放残余电荷。
  void setHBridgeShort();
  // H 桥开路：正负端同时拉低，进入空闲状态。
  void setHBridgeOpen();
  // 保持 H 桥短路一段时间后恢复开路。
  void releaseHBridgeResidualCharge(uint32_t durationUs);
  // 按触发模式执行一次 H 桥触发。
  void triggerOnce(triggerMode_e triggerMode);

  // 更新运行时刺激参数，并做范围校正。
  void setSenseParams(const SenseParams_t& params);
  // 只读获取当前刺激参数。
  const SenseParams_t& senseParams() const { return sense_; }
  // 兼容旧版数组顺序：{强度, 脉宽, 触发间隔, 触发次数, 单元间隔}。
  void setSenseFromLegacyArray(const int values[5]);

 private:
  // 将百分比限制在 [0, 100]。
  static float clampPercent(float value);
  // 将百分比换算成 analogWrite/LEDC 使用的输出范围。
  static uint32_t dutyFromPercent(float percent, uint32_t outMax);

  // 构造时传入的引脚映射。
  Pins_t pins_;
  // 升压阶段使用的固定配置。
  boostConfig_t boost_;
  // 触发阶段使用的运行时配置。
  SenseParams_t sense_;

  // 当前运行使用的升压脉冲数缓存。
  uint16_t boostCount_ = 0;
  // 双向交流输出时的内部极性切换状态。
  bool nextPolarityHigh_ = false;
};

}  // namespace shock
