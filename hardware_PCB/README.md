openTENS

## PCB & Schematic

We recommand **AC_TENS_version2** for a shock moudle and **adapterBoard_version1** to adapte your breadboard.

You can just donwload PCB Greber or check project files in easyEDA.

---

## AC_TENS 电刺激模块

### 📍 Pinout Overview（引脚定义）

| 引脚名称 | 类型 | 功能说明 | 连接 |
|---------|------|-----------|-----------|
| VCC_3V3 | 电源 | 系统供电输入 | 3.3V~5V |
| GND     | 地   | 公共地线 | 0V |
| BOOST_L | Boost Enable | 电刺激升压控制引脚 | PWM |
| CTRL_B-I | Input | 交流电刺激脉冲控制 | GPIO |
| CTRL_A-I | Input | 交流电刺激脉冲控制 | GPIO |
| 1-O   | Output | 交流电脉冲输出 | 理疗电极 |
| 2-O   | Output | 交流电脉冲输出 | 理疗电极 |

<p align="center">
  <img src="../images/AC_TENS_version2_3D.png" alt="Hardware Schematic" width="650">
</p>

<p align="center">
  <em>图：openTENS AC_TENS_version2</em>
</p>

---

## adapterBoard 2.54mm针脚to2.5mm耳机孔转接板

### 📍 Pinout Overview（引脚定义）

| 引脚名称 | 类型 | 功能说明 |
|---------|------|-----------|
| 2.54mm | 2.54排针脚 | 可连接排针，面包板，电刺激模块 |
| 2.5mm     | 2.5mm耳机孔   | 可连接标准理疗仪电极线 |

<p align="center">
  <img src="../images/adapterBoard_version1_3D.png" alt="Hardware PCB" width="650">
</p>

<p align="center">
  <em>图：openTENS adapterBoard_version1</em>
</p>

---

AD TIME~

## 🧭 相关开源项目

[**dogoLab - 开源的带体动传感器的遥控电刺激设备**](https://github.com/0ingchun/dogoLab)
powered by Arduino & ESP32

<p align="center">
  <img src="../images/dogoLab_shocker_c3_version1_3D.png" alt="Hardware PCB" width="650">
</p>

---