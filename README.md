# 💧 Smart Water Treatment System (ESP8266 + Fuzzy Logic + Blynk)

This is a smart IoT-based water treatment system that monitors **turbidity** and **pH levels** in real time. It uses **fuzzy logic** to control a **servo motor valve** to treat water automatically. Data is displayed on an **LCD I2C 16x2** and sent to the **Blynk** app for remote monitoring.

---

## 🔧 Features
- 🔍 Real-time monitoring of water clarity and pH.
- 🧠 Intelligent decision-making using fuzzy logic.
- ⚙️ Automatic valve control using a servo motor.
- 📱 Blynk IoT integration for mobile notifications and monitoring.
- 📺 LCD I2C for on-device sensor display.

---

## 📦 Hardware Required

| Component           | Quantity | Notes                                      |
|--------------------|----------|--------------------------------------------|
| ESP8266 (NodeMCU)  | 1        | Main microcontroller with WiFi capability  |
| Turbidity Sensor   | 2        | To measure water clarity (NTU)             |
| pH Sensor          | 1        | To measure water pH                        |
| ADS1115            | 1        | 16-bit ADC module for analog inputs        |
| Servo Motor        | 1        | Controls water valve (open/close)          |
| LCD I2C 16x2       | 1        | Displays pH and turbidity values           |
| Jumper Wires       | -        |                                              |
| Breadboard/PCB     | 1        | Optional depending on your build           |
| Power Supply 5V    | 1        | External power for sensors & servo         |

---

## 🔌 Wiring Overview

| Sensor / Module     | ESP8266 Pin | Description               |
|---------------------|-------------|---------------------------|
| ADS1115 (I2C)       | SDA → D2    | I2C Data Line             |
|                     | SCL → D1    | I2C Clock Line            |
| LCD I2C             | SDA → D2    | Shared with ADS1115 SDA   |
|                     | SCL → D1    | Shared with ADS1115 SCL   |
| Servo Motor         | PWM → D3    | GPIO0                     |
| Turbidity Sensor 1  | AIN0        | Connected to ADS1115 AIN0 |
| Turbidity Sensor 2  | AIN1        | Connected to ADS1115 AIN1 |
| pH Sensor           | AIN3        | Connected to ADS1115 AIN3 |

---

## 📲 Blynk Integration

Widgets setup:
- Gauge for Turbidity 1 (`V0`)
- Gauge for Turbidity 2 (`V1`)
- Gauge for pH Value (`V2`)
- Label or Terminal for status display (`V3`, `V4`)
- Event named `notifikasi_keruh` for turbidity alert

---

## ⚙️ System Workflow

1. Sensors read pH and turbidity values.
2. Data is digitized via ADS1115 and sent to the ESP8266.
3. Fuzzy logic processes input to decide valve action:
   - Water Clarity: Clear / Normal / Murky
   - pH Levels: Acidic / Neutral / Alkaline
4. Servo motor opens/closes the valve accordingly.
5. Data is shown on the LCD and sent to the Blynk app.
6. If water is **murky**, a Blynk event sends an alert.

---

## 🧠 Fuzzy Logic Design

### Inputs
- **Turbidity (NTU)**  
  - Clear: 0–25  
  - Normal: 25–85  
  - Murky: 85–100

- **pH Value**  
  - Acidic: 0–7  
  - Neutral: 6.5–7.5  
  - Alkaline: 7.5–14

### Output
- **Servo Motor**  
  - Open Valve: 0°  
  - Close Valve: 180°

---

## 📥 Required Libraries

Install these via Arduino IDE Library Manager:
- `ESP8266WiFi`
- `Blynk` (or `BlynkSimpleEsp8266`)
- `Adafruit_ADS1X15`
- `Servo`
- `LiquidCrystal_I2C`
- `Fuzzy` (EFLL - Embedded Fuzzy Logic Library)

---

## ▶️ Upload Instructions

1. Open the sketch in Arduino IDE.
2. Select board: `NodeMCU 1.0 (ESP-12E Module)`
3. Install all required libraries.
4. Replace the following with your own:
   ```cpp
   char ssid[] = "Your_WiFi_Name";
   char pass[] = "Your_WiFi_Password";
   char auth[] = "Your_Blynk_Auth_Token";
