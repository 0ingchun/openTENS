
---

## 🧩 Data Structure

### `shockPluse_t`

Defines all runtime parameters for one stimulation cycle.

| Field | Type | Description |
|-------|------|-------------|
| `htim_Boost_L` | `TIM_HandleTypeDef*` | Timer handle for PWM generation |
| `Channel_Boost_L` | `uint32_t` | PWM channel |
| `GPIOx_Net_P / GPIOx_Net_N` | `GPIO_TypeDef*` | H-Bridge output pins |
| `boost_T` | `uint32_t` | Boost pulse duration (μs) |
| `boost_F` | `uint32_t` | Boost frequency (Hz) |
| `boost_Width` | `float` | PWM duty ratio (%) |
| `boost_Level` | `uint8_t` | Intensity level (0–15) |
| `trig_Width` | `uint32_t` | Trigger pulse width (μs) |
| `trig_T` | `float` | Trigger period (ms) |
| `trig_Count` | `uint16_t` | Number of trigger pulses per stimulation |
| `usense_T` | `float` | Delay between stimulations (ms) |

---

## ⚙️ Core Functions

### 🧾 Initialization and Configuration

#### `void shockIOPinConfig(shockPluse_t* s)`
> Configure the I/O pins and timer handles for stimulation output.

#### `void shockConstConfig(shockPluse_t* s)`
> Set default constant parameters (frequency, duty ratio, etc.).

#### `void shockAllInit(shockPluse_t* s)`
> Initialize all shock module components (timers, GPIO, constants).

---

### ⚡ PWM Control

#### `void pwmCalculatePSC_ARR(uint32_t freq, uint32_t* PSC, uint32_t* ARR)`
> Calculate prescaler and auto-reload values based on target PWM frequency.

#### `uint8_t shockBoostSetFreq(shockPluse_t* s, uint32_t freq)`
> Configure PWM frequency of the boost driver.

#### `void shockBoostSetDuty(shockPluse_t* s, float duty)`
> Set PWM duty cycle percentage.

---

### 🔁 Stimulation Sequences

#### `void shockBoostVol(shockPluse_t* s)`
> Generate the high-voltage charging pulse sequence.

#### `void shockTriggerAC(shockPluse_t* s, GPIO_PinState state)`
> Generate AC alternating stimulation waveform.

#### `void shockTriggerDC(shockPluse_t* s, GPIO_PinState state)`
> Generate unidirectional DC stimulation waveform.

#### `void shockPulseSenseUnit(shockPluse_t* s)`
> Execute one complete stimulation session (multi-pulse sequence).

---

### 🧮 Mapping & Utility

#### `long longMap(long value, long in_min, long in_max, long out_min, long out_max)`
> Linear integer mapping between two ranges.

#### `double doubleMap(double value, double in_min, double in_max, double out_min, double out_max)`
> Floating-point mapping with precision.

#### `void delay_us(__IO uint32_t delay)`
> Microsecond delay using SysTick counter.

---

### 🧠 Adaptive Feedback

#### `void shockAutoAdjust(uint32_t adcValue, shockPluse_t* s)`
> Automatically adjust stimulation intensity based on ADC feedback signal (e.g., GSR).

Parameters:
- `adcValue`: ADC reading from skin resistance or EMG sensor  
- `s`: pointer to current stimulation structure

Algorithm features:
- Moving average smoothing (`SAMPLE_SIZE = 10`)  
- Maps ADC value range `[0–4096]` to intensity range `[0–12]`  
- Dynamic adjustment using proportional factor `IMPACT_K` and bias `IMPACT_A`  

---

## 🧪 Example Usage

```c
#include "shockModule.h"

shockPluse_t stim;

int main(void)
{
    HAL_Init();
    shockAllInit(&stim);
    stim.boost_Level = 5;
    
    while (1)
    {
        uint32_t gsrValue = readGSR(); // example sensor input
        shockAutoAdjust(gsrValue, &stim);
        shockPulseSenseUnit(&stim);
    }
}

