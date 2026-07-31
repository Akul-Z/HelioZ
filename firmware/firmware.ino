// ==========================================
// 1️⃣ LIBRARIES
// ==========================================
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <TinyGPS++.h>
#include <WebServer.h>
#include <math.h>

// ==========================================
// 2️⃣ WIFI CREDENTIALS
// ==========================================
const char* ssid = "OnePlus 11 5G D362";
const char* password = "sohail123";

// ==========================================
// 3️⃣ FLASK SERVER URL
// ==========================================
const char* serverUrl = "http://10.98.21.47:5000/accident";

// ==========================================
// 4️⃣ PIN DEFINITIONS & THRESHOLDS
// ==========================================
#define MPU_SDA_PIN 21
#define MPU_SCL_PIN 22

#define GPS_RX_PIN 2   // Connect to GPS TX
#define GPS_TX_PIN 4   // Connect to GPS RX

#define BUZZER_PIN 25

#define BASE_IMPACT_THRESHOLD 1.2     // Base g-force threshold for demo
#define GYRO_ROTATION_THRESHOLD 80.0  // Deg/sec threshold for motion
#define ALERT_COOLDOWN_MS 30000       // 30 Seconds auto-recovery fallback

// ==========================================
// 5️⃣ GLOBAL VARIABLES & OBJECTS
// ==========================================
WebServer server(80);
MPU6050 mpu6050(Wire);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

float latitude = 0.0;
float longitude = 0.0;
bool gpsValid = false;

float accelerationX = 0.0, accelerationY = 0.0, accelerationZ = 0.0;
float gyroX = 0.0, gyroY = 0.0, gyroZ = 0.0;
float impact = 0.0;
float totalRotationSpeed = 0.0;
float dynamicThreshold = 1.2;

bool accidentDetected = false;
bool alertSent = false;       
bool buzzerState = false;

unsigned long lastResetTime = 0;
const unsigned long RE_TRIGGER_GRACE_PERIOD_MS = 3000; // 3s grace window after reset

unsigned long lastAlertTime = 0;
unsigned long previousPrintMillis = 0;
const long printInterval = 500;

// ==========================================
// FUNCTION DECLARATIONS
// ==========================================
void connectWiFi();
void checkWiFiConnection();
void initializeMPU();
void initializeGPS();
void readMPU();
void readGPS();
void detectAccident();
void sendAccident();
void activateBuzzer();
void stopBuzzer();
void resetAccident();
void printSensorData();
void handleRoot();
void handleDataJson();
void handleBuzzerOff();

// ==========================================
// 6️⃣ SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n🚀 --- STARTING HELIOZ SMART HELMET SYSTEM ---");

  // Configure Hardware Pins
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Hardware Buzzer Test at Power-On
  Serial.println("🔊 Testing Buzzer Hardware...");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(300);
  digitalWrite(BUZZER_PIN, LOW);

  // Network & Sensors
  connectWiFi();
  initializeMPU();
  initializeGPS();

  // Web Server Endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleDataJson);
  server.on("/buzzer/off", HTTP_POST, handleBuzzerOff);
  server.begin();

  Serial.println("🌐 Web Server Online & Endpoints Registered!");
  Serial.println("===========================================\n");
}

// ==========================================
// 7️⃣ MAIN LOOP
// ==========================================
void loop() {
  checkWiFiConnection();
  server.handleClient();

  readMPU();
  readGPS();
  detectAccident();

  // Alert Triggering
  if (accidentDetected && !alertSent) {
    sendAccident();
    activateBuzzer();
    alertSent = true;
    lastAlertTime = millis();
  }

  // Enforce hardware pin state based on buzzerState
  digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);

  // Auto-Recovery Fallback
  if (alertSent && (millis() - lastAlertTime > ALERT_COOLDOWN_MS)) {
    Serial.println("⏰ Cooldown elapsed. Resetting alert state...");
    resetAccident();
  }

  printSensorData();
}

// ==========================================
// 8️⃣ MODULAR FUNCTIONS
// ==========================================

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("📡 Connecting to WiFi: ");
  Serial.println(ssid);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("📍 ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi Disconnected! Re-initiating connection...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    delay(1000);
  }
}

void initializeMPU() {
  Serial.println("⚙️ Initializing MPU6050...");
  Wire.begin(MPU_SDA_PIN, MPU_SCL_PIN);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  Serial.println("✅ MPU6050 Calibrated Successfully!");
}

void initializeGPS() {
  Serial.println("⚙️ Initializing GPS Module...");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("✅ GPS Serial Stream Started!");
}

void readMPU() {
  mpu6050.update();
  
  accelerationX = mpu6050.getAccX();
  accelerationY = mpu6050.getAccY();
  accelerationZ = mpu6050.getAccZ();

  gyroX = mpu6050.getGyroX();
  gyroY = mpu6050.getGyroY();
  gyroZ = mpu6050.getGyroZ();

  impact = sqrt(accelerationX * accelerationX + 
                accelerationY * accelerationY + 
                accelerationZ * accelerationZ);

  totalRotationSpeed = sqrt(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);

  // Dynamic Threshold Logic: Threshold adapts with movement
  dynamicThreshold = BASE_IMPACT_THRESHOLD + (totalRotationSpeed / 200.0);
  if (dynamicThreshold > 4.0) dynamicThreshold = 4.0;
}

void readGPS() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    gpsValid = true;
  } else {
    latitude = 0.0;
    longitude = 0.0;
    gpsValid = false;
  }
}

void detectAccident() {
  // Ignore detection during 3s grace window after reset
  if (millis() - lastResetTime < RE_TRIGGER_GRACE_PERIOD_MS) {
    return;
  }

  bool highImpactDetected = (impact > dynamicThreshold);

  if (highImpactDetected) {
    if (!accidentDetected) {
      accidentDetected = true;
      Serial.println("\n💥 ACCIDENT DETECTED! Impact exceeded threshold!");
    }
  }
}

void sendAccident() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.setTimeout(5000);
    http.addHeader("Content-Type", "application/json");

    float sendLat = gpsValid ? latitude : 12.971598;
    float sendLon = gpsValid ? longitude : 77.594563;

    String jsonPayload = "{\"latitude\":" + String(sendLat, 6) + 
                         ",\"longitude\":" + String(sendLon, 6) + 
                         ",\"impact\":" + String(impact, 2) + "}";

    Serial.println("\n📤 Sending Accident Payload to Flask Server:");
    Serial.println(jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);
    http.end();
  }
}

void activateBuzzer() {
  buzzerState = true;
  digitalWrite(BUZZER_PIN, HIGH); // Force ON immediately
  Serial.println("🔔 Buzzer Alarm Activated!");
}

void stopBuzzer() {
  buzzerState = false;
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("🔕 Buzzer Deactivated.");
}

void resetAccident() {
  alertSent = false;
  accidentDetected = false;
  lastResetTime = millis(); // Mark reset timestamp
  stopBuzzer();
  Serial.println("🔄 System Reset to Normal Mode.");
}

void printSensorData() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousPrintMillis >= printInterval) {
    previousPrintMillis = currentMillis;

    Serial.printf("Impact: %.2f g | Thresh: %.2f g | Gyro: %.1f deg/s | Status: %s\n",
                  impact, dynamicThreshold, totalRotationSpeed,
                  accidentDetected ? "ACCIDENT!" : "NORMAL");
  }
}

// ==========================================
// 9️⃣ LIVE WEB DASHBOARD
// ==========================================

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>HelioZ Control Center</title>
<style>
body { font-family: Arial, sans-serif; text-align: center; background: #f4f4f4; padding: 20px; }
.card { background: white; padding: 15px; border-radius: 10px; margin: 12px auto; max-width: 360px; box-shadow: 0 2px 6px rgba(0,0,0,0.1); }
.val { font-size: 22px; font-weight: bold; color: #2c3e50; }
.status-alert { color: #e74c3c; font-size: 22px; font-weight: bold; }
.status-ok { color: #2ecc71; font-size: 22px; font-weight: bold; }
button.off-btn { padding: 15px 30px; font-size: 18px; background: #e74c3c; color: white; border: none; border-radius: 8px; cursor: pointer; font-weight: bold; }
button.off-btn:hover { background: #c0392b; }
button.map-btn { padding: 10px 20px; font-size: 15px; background: #3498db; color: white; border: none; border-radius: 6px; cursor: pointer; }
</style>
</head>
<body>

<h1>🚑 HelioZ Smart Helmet</h1>
<h3>ESP32 Control Panel</h3>

<p>Status: <span id="status" class="status-ok">LOADING...</span></p>

<div class="card">
  <h3>💥 Impact Force</h3>
  <div id="impact" class="val">0.00 g</div>
</div>

<div class="card">
  <h3>📊 Dynamic Threshold</h3>
  <div id="threshold" class="val">1.20 g</div>
</div>

<div class="card">
  <h3>📍 Coordinates</h3>
  <div id="coords" style="font-size: 17px;">--<br>--</div>
</div>

<div class="card">
  <a id="mapLink" href="#" target="_blank">
    <button type="button" class="map-btn">📍 Open Google Maps</button>
  </a>
</div>

<br>
<form action="/buzzer/off" method="POST">
  <button type="submit" class="off-btn">TURN OFF BUZZER</button>
</form>

<script>
async function updateTelemetry() {
  try {
    let res = await fetch('/data');
    let data = await res.json();

    document.getElementById('impact').innerText = data.impact.toFixed(2) + ' g';
    document.getElementById('threshold').innerText = data.threshold.toFixed(2) + ' g';
    document.getElementById('coords').innerHTML = data.lat.toFixed(6) + '<br>' + data.lon.toFixed(6);
    document.getElementById('mapLink').href = 'https://maps.google.com/?q=' + data.lat + ',' + data.lon;

    let statusEl = document.getElementById('status');
    if (data.accident) {
      statusEl.innerText = 'ACCIDENT DETECTED!';
      statusEl.className = 'status-alert';
    } else {
      statusEl.innerText = 'NORMAL';
      statusEl.className = 'status-ok';
    }
  } catch(e) {
    console.log(e);
  }
}

setInterval(updateTelemetry, 200);
updateTelemetry();
</script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleDataJson() {
  float sendLat = gpsValid ? latitude : 12.971598;
  float sendLon = gpsValid ? longitude : 77.594563;

  String json = "{";
  json += "\"impact\":" + String(impact, 2) + ",";
  json += "\"threshold\":" + String(dynamicThreshold, 2) + ",";
  json += "\"lat\":" + String(sendLat, 6) + ",";
  json += "\"lon\":" + String(sendLon, 6) + ",";
  json += "\"accident\":" + String(accidentDetected ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// Endpoint: Turn OFF Buzzer and Reset System
void handleBuzzerOff() {
  resetAccident();
  server.sendHeader("Location", "/");
  server.send(303);
}