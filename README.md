# 🃏 Joker: IoT-based Smart Joke Machine

[![ESP32](https://img.shields.io/badge/Device-ESP32-red)]()
[![PlatformIO](https://img.shields.io/badge/IDE-PlatformIO-orange)]()
[![Make.com](https://img.shields.io/badge/Cloud-Make.com-purple)]()
[![Google Sheets](https://img.shields.io/badge/Database-Google%20Sheets-green)]()

**Joker** is a cloud-connected interactive IoT device designed to deliver humor tangibly. Unlike passive screens, Joker interacts with users through physical inputs, provides real-time bilingual content (English/Korean), and visualizes reactions using motion and sound.

This project overcomes the hardware limitations of the ESP32 (e.g., memory constraints with HTTPS) by implementing a robust **Cloud-Offloaded Proxy Architecture**.

---

## 🚀 Key Features

* **Category Selection:** Users can select joke categories (Programming, Pun, Dark, etc.) via a 4x4 Keypad.
* **Cloud-Powered Translation:** Displays the original English joke on the LCD while simultaneously streaming a Korean translation, enabled by a server-side proxy.
* **Interactive Rating System:** Users rate jokes (1-5 stars), triggering adaptive buzzer sounds (Celebratory vs. Disappointed tones).

---

## 🏗️ System Architecture

To handle heavy SSL/TLS handshakes and complex JSON parsing on a resource-constrained ESP32, we designed a **Middleware Proxy** using Make.com.

![System Diagram](static/images/final_diagram.png)

1.  **Request:** ESP32 sends a lightweight HTTP request to the Make.com Webhook.
2.  **Process (Cloud):**
    * Make.com fetches a random joke from an external API.
    * Translates the text using Google Translate API.
    * Formats the data into a simple string (e.g., `Joke ||| Translation`).
3.  **Response:** The ESP32 receives the optimized string, avoiding stack overflow issues.
4.  **Logging:** User ratings are sent back to the cloud and logged into Google Sheets for ranking analysis.

---

## 🛠️ Hardware & Tech Stack

### Hardware Components
* **MCU:** ESP32 Development Board
* **Display:** ILI9341 TFT LCD (SPI)
* **Input:** 4x4 Matrix Keypad
* **Actuators:** Passive Buzzer

### Software Stack
* **Firmware:** C++ (Arduino Framework via PlatformIO)
* **Simulation:** Wokwi Simulator (VSCode Extension)
* **Middleware:** Make.com (formerly Integromat)
* **Database:** Google Sheets & Google Apps Script

---

## ⚠️ Technical Challenges & Optimization

### LDR Auto-Brightness in Simulation
We initially implemented an auto-brightness feature using an LDR sensor and PWM control. However, it was disabled in the final simulation build due to the following reasons:

1.  **Simulation Lag:** Processing high-frequency PWM interrupts (5000Hz) alongside LCD rendering caused the simulation speed to drop below 50% of real-time.
2.  **SSL Timeouts:** The simulation lag led to clock desynchronization, causing **Network Error -80** during the SSL handshake with the cloud server.
3.  **Decision:** To prioritize network stability and seamless UI demonstration, the LDR feature was deprecated in the simulation code (though the logic remains valid for physical hardware).

> *This decision was made to ensure the stability of the core network features during the presentation.*

---

## 👥 Team & Roles

| Member | Role | Key Contributions |
|:---:|:---:|:---|
| **김주현** | **System Architect** | • Designed Make.com Cloud Proxy Architecture<br>• Implemented Translation & Logging Logic<br>• Developed Bi-directional Google Sheets Pipeline • Designed Circuit • Developed Category Selection Logic|
| **강정후** | **Hardware** | • Designed Circuit & Wiring<br>• Engineered Adaptive Buzzer Feedback |
| **김태연** | **Software**| • Implemented Keypad State Machine<br>• Developed Ranking System |

김주현
github: https://github.com/joohyun365/IoT-Project-Joker-B
homepage: https://joohyun365.github.io/IoT-Project-Joker-B/

강정후
github: https://github.com/kjhu1211-cpu/IoT-000B
homepage: https://kjhu1211-cpu.github.io/IoT-000B/

김태연
github: https://github.com/leo3898/IoT-000B
homepage: https://leo3898.github.io/IoT_000B/
