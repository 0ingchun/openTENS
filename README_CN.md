<p align="right">
  <a href="README.md">
    <img src="https://img.shields.io/badge/Switch to-English-blue" alt="Switch to English">
  </a>
</p>

# openTENS：开源电刺激系统（Electrical Stimulation System）

<p align="center">
  <img alt="banner" src="images/AC_TENS_version2_3D.png" width="600">
</p>
<p align="center">
  一个用于科研与可穿戴应用的 **可编程、安全、灵活** 的电刺激平台
</p>

---

## 👋 欢迎来到 openTENS！

首先，感谢你的到来！
欢迎你了解 **openTENS** —— 一个专为研究人员、设计者与开发者打造的 **开源电刺激平台**。

本 README 提供项目的核心信息，你可以直接跳转到以下章节：

---

## 🚩 我们在解决什么问题？

### 🔍 背景与痛点

* 大多数市售 **TENS/EMS 设备** 为封闭系统，用户无法修改波形或实现闭环控制
* 想研究不同刺激模式的科研人员必须从零搭电路 → 成本高、且存在安全风险
* 电刺激与 **GSR、EMG、IMU 等传感器融合** 时，缺乏统一硬件接口
* 可穿戴领域中，**纺织电极的舒适性与信号质量** 难以标准化评估

### 💡 openTENS 的解决方案

**openTENS** 的目标是成为电刺激领域的 **“Arduino 平台”**：

* 提供 **可编程的双极性刺激输出（H-Bridge）**
* 内置 **硬件限流与隔离机制**，确保使用安全
* 集成 **实时 GSR 测量模块 → 便于实现闭环控制**
* 支持 **导电纺织电极与多模态传感扩展**
* 开源，**固件 + PCB** 均可修改

借助 openTENS，你可以快速实现：

* 电刺激 + 情绪调节实验
* 基于纺织电极的康复训练系统
* 智能人机交互与情感计算研究

---

## 🧠 硬件设计

为了帮助你快速理解 **openTENS 的硬件结构与信号流程**，
下面展示了核心电路与模块设计原理图：

<p align="center">
  <img src="images/AC_TENS_version2_Schematic.png" width="650">
</p>

<p align="center">
  <em>图：openTENS 的电刺激电路v2电路原理图</em>
</p>

<p align="center">
  <img src="images/AC_TENS_version2_PCB.png" width="650">
</p>

<p align="center">
  <em>图：openTENS 的电刺激电路v2PCB设计图</em>
</p>

了解详情 👉 [硬件模块 使用指南](hardware_PCB/README.md)

---

## 📚 软件SDK库

若你想了解如何使用 SDK 以及内置库函数，请访问：
👉 [SDK & 软件驱动库 使用指南](software_SDK/README.md)

---

## 🚀 安装与快速上手 <a name="installation"></a>

> （本节将稍后更新硬件连接方法与 SDK 安装教程）

1. 克隆项目仓库

   ```bash
   git clone https://github.com/0ingchun/openTENS.git
   cd openTENS
   ```

2. 👉 [硬件模块 使用指南](hardware_PCB/README.md)

3. 👉 [SDK & 软件驱动库 使用指南](software_SDK/README.md)

---

## ⚠️ 安全说明

在组装或使用 openTENS 硬件前，请务必阅读
👉 [安全与免责声明（SAFETY_NOTICE.md）](SAFETY_NOTICE.md)

---

## ⚠️ 免责声明

- 軟體內所有網頁內容（包括可能存在的付費項目），均与整活者無任何關聯！

- 軟體圖標与開發設計歸项目设计者所有，有關一切禁止商用与公共場合傳播。

- 軟體內容僅供實踐學習使用，使用時請遵守當地法律法規，并自覺在24小時內刪除有關違規內容。

- 下載此项目或使用此项目下的硬件或軟體默認遵守以上內容

- 请遵守该项目的开源协议

---

## 👨‍🔬 谁在开发 openTENS？

openTENS 由以下成员团队发起：  
**Y. Liu** & **X. Teng**，西交利物浦大学（XJTLU）  
指导教师：**Prof. M. ten Bhömer**

本项目灵感源自我们的研究论文：  
[“H-Bridge Bipolar Stimulation System with Real-Time GSR Feedback”](https://ieeexplore.ieee.org/abstract/document/11120621)

我们希望将科研成果转化为一个开放式平台，使更多研究者能够安全地探索和验证 EMS 技术。  
如果本项目对您的研究有所帮助，欢迎引用（cite）我们的工作。  
Paper: **https://ieeexplore.ieee.org/abstract/document/11120621**

---

> ### **版權信息 Copyright information**

版權所有 (c) 2025 [0ingChun](https://github.com/0ingchun)，保留所有權力。

Copyright (c) 2025 [0ingChun](https://github.com/0ingchun), All rights reserved.

---

## 开源协议

> ### **開源協議 Open source license**

硬件原理图和 PCB 设计在 **Apache 2.0** 许可下发布。

Hardware schematics and PCB designs are released under the **Apache 2.0**.

固件和软件在 **Apache 2.0** 许可证下发布。

Firmware and software are released under the **Apache 2.0** License.

---

## 🧭 相关开源项目

[**dogoLab - 开源的带体动传感器的遥控电刺激设备**](https://github.com/0ingchun/dogoLab)
powered by Arduino & ESP32

<p align="center">
  <img src="images/dogoLab_shocker_c3_version1_3D.png" width="300">
</p>

---