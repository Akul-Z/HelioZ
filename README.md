i# 🪖 IoT Smart Helmet: Automatic Accident Detection & AI Incident Reporting

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

```text
[ Onboard Sensors ]
  ├── MPU6050 (G-Force & Rotation)
  └── NEO-6M GPS (Latitude & Longitude)
        │
        ▼
[ ESP32 Microcontroller ]
  ├── Hardware Processing & Crash Detection Logic
  └── 15-Second Manual Override Switch (Cancel Button & Buzzer)
        │
        ├──► [ GSM Module (SIM800L) ] ────► Direct Emergency SMS Alerts
        │
        └──► [ Wi-Fi / Cellular Data ] ───► [ FastAPI Cloud Backend ]
                                                   │
                                                   ▼
                                         [ OpenAI API Integration ]
                                                   │
                                                   ▼
                                         [ AI Incident Summary ]

📂 Repository Structure
smart-helmet-system/
├── firmware/
│   ├── main_helmet_esp32.ino    # ESP32 sensor integration & main loop
│   ├── config.h                 # Wi-Fi, GSM, and API key configurations
│   └── libraries/               # Required local driver dependencies
├── server/
│   ├── app.py                   # FastAPI REST server & webhook endpoint
│   ├── ai_generator.py          # OpenAI API wrapper for crash summary generation
│   ├── requirements.txt         # Python package dependencies
│   └── .env.example             # Environment variable template
├── hardware/
│   ├── schematic_diagram.png    # Circuit wiring diagram
│   └── pinout_guide.md          # ESP32 pin configuration documentation
├── docs/
│   └── API_SPECIFICATION.md     # JSON payload & alert response schema
├── .gitignore
├── LICENSE
└── README.md
