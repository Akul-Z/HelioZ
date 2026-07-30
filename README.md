# HelioZ

```markdown
# 🪖 IoT Smart Helmet: Automatic Accident Detection & AI Incident Reporting

An affordable, IoT-enabled smart helmet designed to drastically reduce emergency response times for two-wheeler accidents. Utilizing onboard sensors and an **ESP32** microcontroller, the helmet automatically detects severe crashes, captures live **GPS coordinates**, transmits instant **SMS alerts**, and triggers an **AI-assisted incident summary report** (estimating impact severity and scenario details) for first responders.

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Hardware: ESP32](https://img.shields.io/badge/Hardware-ESP32%20%7C%20NEO--6M%20%7C%20MPU6050-blue)
![Backend: Python / FastAPI](https://img.shields.io/badge/Backend-FastAPI%20%7C%20OpenAI%20API-green)

---

## 📌 Problem Statement

Road traffic accidents involving two-wheeler riders frequently result in delayed emergency response. In many cases, the rider becomes unconscious or incapacitated, leading to missed alerts and prolonged wait times before medical personnel reach the scene. Standard helmets only offer passive physical protection—they lack automatic crash detection, location sharing, and contextual emergency reporting needed to accelerate rescue operations.

## 🎯 Goal & Key Features

* **Automatic Accident Sensing:** Detects real-time crashes using threshold algorithms combining high G-force impacts ($>4.5g$) and post-collision helmet rotation ($>120^\circ$).
* **False-Alarm Mitigation:** Provides a **15-second manual override window** (via a helmet-mounted cancel button and buzzer) to cancel false triggers before alerts fire.
* **Live Location Dispatch:** Obtains exact satellite GPS coordinates and sends automated SMS alerts containing a Google Maps location link to pre-configured emergency contacts.
* **AI-Assisted Incident Summary:** Sends raw crash telemetry (peak G-force, angular velocity, impact axis, and location data) to a lightweight backend. Generates a structured crash severity report (e.g., *High Likelihood of Head Trauma / Rollover Collision*) to brief first responders before they arrive on scene.

---

## 🏗️ System Architecture


```

[ MPU6050 IMU ] ──────┐
[ Piezo Impact ] ─────┼──(I2C/Digital)──> [ ESP32 Microcontroller ]
[ NEO-6M GPS  ] ──────┘                          │
├──(GSM SIM800L)──> [ Emergency Contacts SMS ]
│
(Wi-Fi / Cellular API)
│
▼
[ Cloud Server / FastAPI ]
│
(OpenAI LLM API)
│
▼
[ AI Crash Severity Report ]

```

---

## 🔌 Hardware Pinout Connection

| Component | Module Pin | ESP32 Pin | Function / Description |
| :--- | :--- | :--- | :--- |
| **MPU6050** | `SCL` / `SDA` | `GPIO 22` / `GPIO 21` | $I^2C$ Accelerometer + Gyroscope |
| **NEO-6M GPS** | `TX` / `RX` | `GPIO 16` (RX2) / `GPIO 17` (TX2) | Satellite Location Tracking |
| **SIM800L GSM**| `TX` / `RX` | `GPIO 4` (RX1) / `GPIO 2` (TX1) | Cellular SMS Dispatch |
| **Cancel Button**| `OUT` | `GPIO 13` (Pull-Up) | 15s False Alarm Abort Switch |
| **Buzzer** | `Positive` | `GPIO 12` | Audible Countdown Alarm |

---

## 📂 Repository Structure


```

├── firmware/
│   └── main_helmet_esp32.ino    # Arduino C++ sketch for ESP32 hardware & logic
├── server/
│   ├── app.py                   # FastAPI cloud backend for AI reporting
│   └── requirements.txt         # Python dependencies
├── hardware/
│   └── wiring_diagram.png       # Schematic wiring diagram
├── README.md
└── LICENSE

```

---

## ⚡ Quickstart & Installation

### 1. Firmware Flashing (ESP32)
1. Open `firmware/main_helmet_esp32.ino` in the **Arduino IDE**.
2. Install the required libraries via **Sketch → Include Library → Manage Libraries**:
   * `Adafruit MPU6050`
   * `TinyGPS++`
   * `HardwareSerial`
3. Configure your target Emergency Contact numbers and Cloud Server Endpoint inside the code:
   ```cpp
   const char* EMERGENCY_PHONE_NUMBER = "+1234567890";
   const char* BACKEND_SERVER_URL     = "[https://your-api-server.com/api/report-accident](https://your-api-server.com/api/report-accident)";

```

4. Select board **ESP32 Dev Module** and flash the sketch.

### 2. AI Reporting Backend Setup

1. Navigate to the `server/` directory:
```bash
cd server

```


2. Install Python dependencies:
```bash
pip install -r requirements.txt

```


3. Create a `.env` file containing your API credentials:
```env
OPENAI_API_KEY=your_openai_api_key_here

```


4. Launch the API server:
```bash
uvicorn app:app --host 0.0.0.0 --port 8000

```



---

## 📋 Sample AI Incident Report Output

When an accident payload is posted to the backend server, the integrated AI model evaluates telemetry patterns and outputs a brief report:

```json
{
  "incident_id": "ACC-2026-0891",
  "status": "CRITICAL_ALERT",
  "timestamp": "2026-07-30T22:09:00Z",
  "location": {
    "latitude": 12.9716,
    "longitude": 77.5946,
    "google_maps_link": "[https://maps.google.com/?q=12.9716,77.5946](https://maps.google.com/?q=12.9716,77.5946)"
  },
  "ai_incident_summary": {
    "severity_level": "HIGH",
    "impact_g_force": "7.2g",
    "collision_type": "High-Impact Lateral Collision with Rollover",
    "recommended_triage_briefing": "Rider sustained severe impact force exceeding 7g followed by full 180° helmet inversion. High risk of traumatic head/neck injury and loss of consciousness. Immediate dispatch of ALS (Advanced Life Support) recommended."
  }
}

```

---

## 🛡️ License

This project is licensed under the MIT License - see the [LICENSE](https://www.google.com/search?q=LICENSE) file for details.

```

<ElicitationsGroup message="Where would you like to focus next to finalize your project setup?">
  <Elicitation label="Generate full ESP32 Arduino firmware code" query="Provide the complete C++ code for the ESP32 firmware including MPU6050, GPS, SIM800L, and the 15-second cancel button timer."/>
  <Elicitation label="Build the Python FastAPI backend for AI summaries" query="Write the Python FastAPI server code that receives sensor JSON from the ESP32 and uses the OpenAI API to generate the AI incident summary."/>
</ElicitationsGroup>

```
