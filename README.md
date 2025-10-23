# ⚙️ Project 3 — PID Control Loop Simulation (ESP32-C3)

This project implements a **real-time PID control loop** on an **ESP32-C3 DevKitM-1**.  
It simulates a single-axis “drone pitch” system, runs at a fixed control rate (100 Hz), and drives the on-board LED as a mock motor output.  
You can live-tune `Kp`, `Ki`, `Kd`, and the target angle in real time via the Serial Monitor.

---

## 🎯 Project Goals

- Understand the structure of a **flight-controller stabilization loop**  
- Learn how to implement **P, I, D** terms in code  
- Maintain **precise timing** for control loops using FreeRTOS  
- Observe the effects of gain tuning, overshoot, and damping  
- Map controller output to a physical actuator (PWM brightness)

---

## 🧩 Hardware Requirements

| Component | Purpose |
|------------|----------|
| **ESP32-C3 DevKitM-1** | Main MCU |
| **USB Cable** | Serial + Power |
| *(Optional)* External LED or motor driver | For visual feedback |

> The on-board LED on **GPIO 2** is used as the actuator by default.

---

## 💻 Code Overview

### Loop Frequency  
Runs at **100 Hz (10 ms dt)** using `vTaskDelayUntil()` for deterministic timing.

### PID Core  
```cpp
u = Kp * e + Ki * ∫e dt + Kd * de/dt
