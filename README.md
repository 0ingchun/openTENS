# ⚡ openTENS: 开源电刺激系统

<p align="center">
  <img alt="banner" src="docs/opentens_banner.jpg" width="600">
</p>
<p align="center">
  为科研与可穿戴设备提供可编程、安全、灵活的电刺激开发平台
</p>

---

## 👋 欢迎使用 openTENS！

首先，欢迎你！🎉🎉🎉  
感谢关注 **openTENS** —— 一个为研究者、设计师和开发者打造的开源电刺激平台。

本文件（README）旨在为你提供项目的主要信息。  
你可以直接跳转到以下章节：

* [我们在做什么？为什么？](#我们在做什么)
* [谁在开发 openTENS？](#谁在开发)
* [我们需要什么？](#我们需要什么)
* [如何参与贡献？](#如何参与)
* [安装与使用](#安装)
* [联系我们](#联系我们)
* [更多资源](#更多资源)

---

## 💡 我们在做什么？

### 🧩 背景与问题

- 现有的 **商用电刺激（TENS/EMS）设备** 往往是封闭系统，无法修改波形或实现闭环控制  
- 研究人员想探索 **不同波形、不同刺激模式**，却需要自制电路，成本高且安全风险大  
- 想与 **传感器（GSR、EMG、IMU 等）** 结合做实验，却缺乏统一的开发接口  
- 可穿戴设备研究中，**织物电极的信号质量与舒适性问题** 仍然没有标准化平台可测试  

### 🚀 我们的解决方案

**openTENS** 致力于成为一个类似 Arduino 的开源电刺激平台：

- 提供 **可编程双极性电刺激输出（H-Bridge）**  
- 具备 **硬件限流 / 隔离保护** 的安全电路  
- 内置 **GSR 传感器接口** 实现闭环控制  
- 支持 **可穿戴织物电极** 与多模态传感扩展  
- 开源 **Python / Arduino SDK**，轻松集成到科研项目中  

使用 openTENS，你可以快速实现：
- 电刺激 + 情绪调节实验  
- 织物电极康复训练系统  
- 智能交互与情绪计算研究  

---

## 👨‍🔬 谁在开发 openTENS？

**openTENS** 由以下成员发起：

- **刘一博** & **滕孝鸣**，西交利物浦大学（XJTLU）  
  指导教师：**Prof. Martijn ten Bhömer**、**Dr. Suneel Kumar Kommuri**

项目灵感源自我们在 “H-Bridge Bipolar Stimulation System with Real-Time GSR Feedback” 的研究。  
我们希望将科研成果转化为开放平台，让更多人能安全地探索 EMS 技术。

---


## ⚙️ 安装与快速使用 <a name="安装"></a>


### 安装步骤
```bash
git clone https://github.com/yourusername/openTENS.git
cd openTENS
pip install -r requirements.txt
python examples/run_demo.py
