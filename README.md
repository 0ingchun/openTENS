<p align="right">
  <a href="README_CN.md">
    <img src="https://img.shields.io/badge/Switch to-中文文档-blue" alt="Switch to Chinese">
  </a>
</p>

# openTENS: Open-Source Electrical Stimulation System

<p align="center">
  <img alt="banner" src="images/AC_TENS_version2_3D.png" width="600">
</p>
<p align="center">
  A **programmable**, **safe**, and **flexible** electrical stimulation platform  
  designed for research and wearable applications.
</p>

---

## 👋 Welcome to openTENS!

Thank you for your interest in **openTENS** —  
an open-source electrical stimulation platform designed for **researchers, developers, and hardware designers**.

This README gives you an overview of the project.  
You may jump directly to the major sections listed below:

---

## 🚩 What Problems Are We Solving?

### 🔍 Background & Challenges

- Most commercial **TENS/EMS devices** are closed systems — waveform modification and closed-loop control are not allowed  
- Researchers exploring **custom stimulation modes** must build circuits from scratch → high cost & safety risks  
- Lack of **standardized hardware interfaces** for integrating stimulation with **GSR, EMG, IMU,** and other sensors  
- In wearable applications, the **comfort and signal quality** of textile electrodes are difficult to test and evaluate

### 💡 openTENS — Our Solution

The goal of **openTENS** is to become the **“Arduino Platform” for electrical stimulation**:

- Provides **programmable bipolar stimulation output (H-Bridge)**
- **Hardware current limiting & isolation** for user safety
- Built-in **real-time GSR sensing** for closed-loop control
- Supports **conductive textile electrodes** and sensor extensions
- Fully open-source — **firmware + PCB design**

With openTENS, you can quickly prototype:

- Electrical stimulation + emotion regulation experiments  
- Rehabilitation training systems with textile electrodes  
- Human-computer interaction (HCI) and affective computing studies  

---

## 🧠 Hardware Design Overview

To better understand how **openTENS** works at the hardware level,  
the diagrams below show the **core circuit design** and hardware flow:

<p align="center">
  <img src="images/AC_TENS_version2_Schematic.png" alt="Hardware Schematic" width="650">
</p>

<p align="center">
  <em>Figure: Core schematic of the AC_TENS v2 stimulation circuit</em>
</p>

<p align="center">
  <img src="images/AC_TENS_version2_PCB.png" alt="Hardware PCB" width="650">
</p>

<p align="center">
  <em>Figure: PCB layout design of openTENS v2</em>
</p>

👉 Learn more: [Hardware PCB Module Guide](hardware_PCB/README.md)

---

## 📚 Software SDK

To use the SDK and driver libraries for openTENS, please visit:  
👉 [SDK & Software Library Guide](software_SDK/README.md)

---

## 🚀 Installation & Quick Start <a name="installation"></a>

> (Hardware setup and SDK tutorials will be added in the next update)

1. Clone this repository:

   ```bash
   git clone https://github.com/0ingchun/openTENS.git
   cd openTENS
   ```

2. 👉 Read the **Hardware PCB Guide**:  
   `hardware_PCB/README.md`

3. 👉 Read the **SDK & Software Library Guide**:  
   `software_SDK/README.md`

---

## ⚠️ Safety Notice

Before assembling or using any openTENS hardware,  
please read the safety guidelines:  
👉 [Safety Notice & Disclaimer](SAFETY_NOTICE.md)

---

## 👨‍🔬 Who Is Developing openTENS?

openTENS was initiated by:  
**Y. Liu** & **X. Teng** — Xi’an Jiaotong-Liverpool University (XJTLU)  
Supervising Professor: **Prof. M. ten Bhömer**

This project was inspired by our research paper:  
[“H-Bridge Bipolar Stimulation System with Real-Time GSR Feedback”](https://ieeexplore.ieee.org/abstract/document/11120621)

Our ambition is to transform research results into an **open, safe, and reusable platform**  
for researchers to explore and validate EMS technologies.

If openTENS contributes to your work, please consider citing our paper:  
Paper link: **https://ieeexplore.ieee.org/abstract/document/11120621**

---

## 🧭 Related Open-Source Project

[**dogoLab – Open-source remote EMS device with motion sensors**](https://github.com/0ingchun/dogoLab)  
powered by Arduino & ESP32

<p align="center">
  <img src="images/dogoLab_shocker_c3_version1_3D.png" alt="Hardware PCB" width="650">
</p>

---
