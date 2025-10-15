# ⚡ openTENS Safety and Usage Disclaimer

> ⚠️ **Important:**  
> openTENS is an **open-source electrical stimulation toolkit** intended **for research, education, and prototyping purposes only**.  
> It is **NOT a certified medical device**, and **must not be used for medical treatment, diagnosis, or therapy**.  
> Improper use may result in injury, discomfort, or hardware damage.  
> Always operate within safe voltage and current limits.

---

## 🧩 Overview

**openTENS** is designed as an **open-source platform** to explore and prototype electrical stimulation systems safely.  
The project provides circuit designs, firmware, and examples to support research in **wearable technology**, **affective computing**, and **human-computer interaction**.

However, the hardware and firmware are provided **“as is”**, without any warranty or guarantee regarding safety or fitness for a particular purpose.

---

## ⚠️ Safety Warnings

1. **Human Safety Comes First**  
   Only use openTENS under supervision by individuals familiar with **electrical safety and physiology**.  
   Never apply stimulation above the safe current and voltage limits described below.

2. **Not for Clinical or Medical Use**  
   openTENS is not intended to replace or replicate certified medical-grade TENS/EMS equipment.  
   It must **not** be used on patients or for any therapeutic procedures.

3. **Isolation and Power Source**  
   Always power the circuit using an **isolated source (battery or isolated DC/DC converter)**.  
   Do not connect openTENS directly to a computer USB port while electrodes are attached to the body.

4. **Current Limiting and Overvoltage Protection**  
   Use hardware current limiters, resistors, and transient voltage suppressors (TVS) as part of your circuit design.  
   Verify that the current never exceeds safe levels.

5. **Body Contact and Electrodes**  
   - Use **biocompatible electrodes** (e.g., medical-grade hydrogel pads or conductive textiles).  
   - Avoid direct metal-to-skin contact.  
   - Keep electrode surfaces clean and replace them when degraded.

6. **Operating Limits (Recommended)**  
   | Parameter | Recommended Safe Range |
   |------------|------------------------|
   | Output Current (RMS) | ≤ 10 mA |
   | Output Voltage | ±50 V max |
   | Frequency Range | 1 – 100 Hz |
   | Pulse Width | ≤ 500 µs |
   | Duty Cycle | ≤ 20% (typical) |

7. **Individuals Who Should NOT Use openTENS**  
   - People with **pacemakers or implanted medical devices**  
   - Those with **cardiac, neurological, or skin disorders**  
   - **Children or untrained individuals**

8. **Testing on the Body**  
   - Always start at **minimum amplitude** and increase slowly.  
   - Test with **simulated loads (resistors)** before any human contact.  
   - Stop immediately if you experience pain, discomfort, or abnormal sensations.

---

## 🧠 Responsible Use

By using or modifying openTENS, you acknowledge that:

- You take **full responsibility** for any consequences of its use.  
- The authors, contributors, and affiliated institutions **shall not be held liable** for any injury, loss, or damage caused by misuse.  
- You will **not distribute or commercialize** openTENS as a medical product without meeting applicable regulations and certifications.  

---

## 🧾 Reference Standards (Recommended Reading)

- **IEC 60601-1:** Medical Electrical Equipment — General Requirements for Safety  
- **IEEE Std 602-2007:** Electrical Systems in Health Care Facilities  
- **ISO 14971:** Medical Devices — Application of Risk Management  

---

## 🧰 Additional Safety Resources

If you are developing or modifying openTENS hardware:
- Review **hardware current limit design** in `/docs/safety/` (if available).  
- Use **simulation tools (LTSpice, Proteus, etc.)** before prototyping.  
- Always measure actual output using an **oscilloscope with isolated probes**.

---

## 🪪 Disclaimer Summary

> This open-source project is provided **without any warranty**.  
> All users are responsible for their own safety and compliance with local laws.  
> By using openTENS, you agree to the terms stated in this document.

---

**Authors & Contributors:**  
Xiaoming Teng · Yibo Liu 
*(Xi’an Jiaotong-Liverpool University, School of Advanced Technology)*  
