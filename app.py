from flask import Flask, request, jsonify, render_template_string
from twilio.rest import Client
import os
from dotenv import load_dotenv
from pathlib import Path
import requests
from gemma import generate_report

app = Flask(__name__)

# -----------------------------
# Explicit .env Path Loading
# -----------------------------
env_path = Path(__file__).parent / ".env"

print("Loading:", env_path)
print("Exists :", env_path.exists())

loaded = load_dotenv(dotenv_path=env_path)

print("Loaded :", loaded)

# -----------------------------
# Twilio Configuration
# -----------------------------
TWILIO_ACCOUNT_SID = os.getenv("TWILIO_ACCOUNT_SID")
TWILIO_AUTH_TOKEN = os.getenv("TWILIO_AUTH_TOKEN")
TWILIO_PHONE = os.getenv("TWILIO_PHONE")
EMERGENCY_PHONE = os.getenv("EMERGENCY_PHONE")

# Twilio Client Initialization & Verification
print("\n========== TWILIO CONFIG ==========")
print("SID Loaded       :", TWILIO_ACCOUNT_SID)
print("Phone Loaded     :", TWILIO_PHONE)
print("Emergency Phone  :", EMERGENCY_PHONE)
print("Token Loaded     :", "YES" if TWILIO_AUTH_TOKEN else "NO")

if not all([TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN, TWILIO_PHONE, EMERGENCY_PHONE]):
    print("\n❌ ERROR: One or more Twilio environment variables are missing!")
    client = None
else:
    client = Client(TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN)
    print("✅ Twilio Client Initialized Successfully")

# -----------------------------
# ESP32 IP
# -----------------------------
ESP32_IP = "http://192.168.1.50"   # Change this to your actual ESP32 IP

# -----------------------------
# Dashboard Data
# -----------------------------
latest_data = {
    "latitude": 0.0,
    "longitude": 0.0,
    "impact": 0.0,
    "ai_report": "No accident detected yet."
}


@app.route("/")
def home():
    html_template = """
    <!DOCTYPE html>
    <html>
    <head>
        <title>HelioZ Dashboard</title>
        <style>
            body { font-family: Arial, sans-serif; margin: 30px; }
            .card { background: #f4f4f4; padding: 20px; border-radius: 8px; margin-bottom: 20px; }
            button {
                padding: 12px 24px;
                font-size: 16px;
                background: #e74c3c;
                color: white;
                border: none;
                border-radius: 5px;
                cursor: pointer;
            }
            button:hover {
                background: #c0392b;
            }
        </style>
    </head>
    <body>
        <h1>HelioZ Emergency Control Center 🚀</h1>

        <div class="card">
            <h3>Latest Accident Telemetry</h3>
            <p><b>Latitude:</b> {{data.latitude}}</p>
            <p><b>Longitude:</b> {{data.longitude}}</p>
            <p><b>Impact:</b> {{data.impact}} g</p>
            <p><b>AI Report:</b> {{data.ai_report}}</p>
        </div>

        <button onclick="turnOffBuzzer()">Turn OFF Buzzer</button>

        <script>
        async function turnOffBuzzer() {
            try {
                let response = await fetch("/buzzer/off", {
                    method: "POST"
                });
                let result = await response.json();
                alert(result.status);
            } catch (error) {
                alert(error);
            }
        }
        </script>
    </body>
    </html>
    """
    return render_template_string(html_template, data=latest_data)


# -----------------------------
# Send SMS / WhatsApp
# -----------------------------
def send_sms(latitude, longitude, impact, ai_report):

    if client is None:
        raise Exception("Twilio client not initialized.")

    try:
        sms = client.messages.create(
            from_=TWILIO_PHONE,
            to=EMERGENCY_PHONE,
            content_sid="HXfe5ab5f00277942d4d4200328b4d403c",
            content_variables="{}"
        )

        print("\n✅ MESSAGE SENT")
        print("SID:", sms.sid)
        print("Status:", sms.status)

    except Exception as e:
        print("\n========== TWILIO FULL ERROR ==========")
        print(type(e))
        print(e)
        raise


# -----------------------------
# Accident Endpoint
# -----------------------------
@app.route("/accident", methods=["POST"])
def accident():
    global latest_data

    data = request.get_json() or {}

    latitude = data.get("latitude", 0.0)
    longitude = data.get("longitude", 0.0)
    impact = data.get("impact", 0.0)

    # Try Gemini/Gemma
    try:
        ai_report = generate_report(latitude, longitude, impact)
    except Exception as e:
        print("\nGemini Error:", e)
        ai_report = (
            "Possible road accident detected. "
            "Immediate medical assistance is recommended."
        )

    latest_data = {
        "latitude": latitude,
        "longitude": longitude,
        "impact": impact,
        "ai_report": ai_report
    }

    print("\n========== ACCIDENT DETECTED ==========")
    print("Latitude :", latitude)
    print("Longitude:", longitude)
    print("Impact   :", impact)

    print("\n========== AI REPORT ==========")
    print(ai_report)

    # Send WhatsApp Alert
    try:
        send_sms(latitude, longitude, impact, ai_report)
    except Exception as e:
        print("\nTwilio Error:", e)

    return jsonify({
        "status": "Success",
        "latitude": latitude,
        "longitude": longitude,
        "impact": impact,
        "ai_report": ai_report
    })


# -----------------------------
# Turn OFF Buzzer
# -----------------------------
@app.route("/buzzer/off", methods=["POST"])
def buzzer_off():
    try:
        esp_url = f"{ESP32_IP}/buzzer/off"
        print(f"\nSending request to {esp_url}")

        response = requests.post(esp_url, timeout=3)

        if response.status_code == 200:
            return jsonify({
                "status": "Buzzer Turned OFF",
                "esp_response": response.json()
            })
        else:
            return jsonify({
                "status": "ESP32 Error"
            }), 500

    except Exception as e:
        return jsonify({
            "status": "Failed to reach ESP32",
            "error": str(e)
        }), 500


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)