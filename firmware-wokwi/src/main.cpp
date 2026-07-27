#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Health Band digital-twin firmware
// Architecture: Device -> MQTT -> Node-RED -> Interactive application

constexpr uint8_t STATUS_LED_PIN = 2;
constexpr uint8_t RGB_R_PIN = 25;
constexpr uint8_t RGB_G_PIN = 26;
constexpr uint8_t RGB_B_PIN = 27;
constexpr uint8_t BUZZER_PIN = 18;
constexpr uint8_t FALL_BUTTON_PIN = 19;
constexpr uint8_t OLED_SDA_PIN = 21;
constexpr uint8_t OLED_SCL_PIN = 22;

constexpr unsigned long BLINK_FAST_MS = 200;
constexpr unsigned long BLINK_SLOW_MS = 1000;
constexpr unsigned long TELEMETRY_INTERVAL_MS = 2000;
constexpr unsigned long RECONNECT_INTERVAL_MS = 5000;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 250;

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
constexpr uint8_t WIFI_CH = 6;

const char* MQTT_HOST = "broker.emqx.io";
constexpr uint16_t MQTT_PORT = 1883;

const char* DEVICE_ID = "health-band-01";
const char* FW_VERSION = "0.2.0";

const char* TOPIC_TELEMETRY = "iot31/nhom-thanh-danh/health-band/telemetry";
const char* TOPIC_STATUS = "iot31/nhom-thanh-danh/health-band/status";
const char* TOPIC_COMMAND = "iot31/nhom-thanh-danh/health-band/command";
const char* TOPIC_EVENT = "iot31/nhom-thanh-danh/health-band/event";

enum SimMode {
  MODE_NORMAL,
  MODE_HIGH_HR,
  MODE_LOW_HR,
  MODE_LOW_SPO2,
  MODE_FALL,
  MODE_LOW_BATTERY
};

struct HealthData {
  int heartRate;
  int spo2;
  bool fallDetected;
  const char* signalQuality;
};

SimMode currentMode = MODE_NORMAL;
unsigned long sequenceNumber = 0;
unsigned long steps = 0;
int battery = 100;
HealthData latestData = {0, 0, false, "unknown"};

unsigned long lastTelemetryMs = 0;
unsigned long lastBlinkMs = 0;
unsigned long lastReconnectMs = 0;
unsigned long lastButtonMs = 0;
bool statusLedState = false;
bool mqttConnected = false;
bool oledReady = false;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

static int randRange(int minimum, int maximum) {
  return minimum + random(maximum - minimum + 1);
}

static const char* modeName(SimMode mode) {
  switch (mode) {
    case MODE_NORMAL: return "normal";
    case MODE_HIGH_HR: return "high_hr";
    case MODE_LOW_HR: return "low_hr";
    case MODE_LOW_SPO2: return "low_spo2";
    case MODE_FALL: return "fall";
    case MODE_LOW_BATTERY: return "low_battery";
    default: return "unknown";
  }
}

static const char* modeShortName(SimMode mode) {
  switch (mode) {
    case MODE_NORMAL: return "NORMAL";
    case MODE_HIGH_HR: return "HIGH HR";
    case MODE_LOW_HR: return "LOW HR";
    case MODE_LOW_SPO2: return "LOW O2";
    case MODE_FALL: return "FALL!";
    case MODE_LOW_BATTERY: return "LOW BAT";
    default: return "UNKNOWN";
  }
}

static bool parseMode(const char* value, SimMode& result) {
  if (!value) return false;
  if (strcmp(value, "normal") == 0) result = MODE_NORMAL;
  else if (strcmp(value, "high_hr") == 0) result = MODE_HIGH_HR;
  else if (strcmp(value, "low_hr") == 0) result = MODE_LOW_HR;
  else if (strcmp(value, "low_spo2") == 0) result = MODE_LOW_SPO2;
  else if (strcmp(value, "fall") == 0) result = MODE_FALL;
  else if (strcmp(value, "low_battery") == 0) result = MODE_LOW_BATTERY;
  else return false;
  return true;
}

static HealthData generateHealthData() {
  HealthData data = {80, 98, false, "medium"};
  switch (currentMode) {
    case MODE_NORMAL:
      data.heartRate = randRange(70, 90);
      data.spo2 = randRange(97, 99);
      break;
    case MODE_HIGH_HR:
      data.heartRate = randRange(125, 145);
      data.spo2 = randRange(95, 97);
      break;
    case MODE_LOW_HR:
      data.heartRate = randRange(40, 48);
      data.spo2 = randRange(96, 98);
      data.signalQuality = "good";
      break;
    case MODE_LOW_SPO2:
      data.heartRate = randRange(80, 95);
      data.spo2 = randRange(85, 92);
      break;
    case MODE_FALL:
      data.heartRate = randRange(100, 130);
      data.spo2 = randRange(93, 95);
      data.fallDetected = true;
      break;
    case MODE_LOW_BATTERY:
      data.heartRate = randRange(75, 85);
      data.spo2 = randRange(97, 98);
      break;
  }
  return data;
}

static void setRgb(bool red, bool green, bool blue) {
  digitalWrite(RGB_R_PIN, red ? HIGH : LOW);
  digitalWrite(RGB_G_PIN, green ? HIGH : LOW);
  digitalWrite(RGB_B_PIN, blue ? HIGH : LOW);
}

static void updateRgbState() {
  if (!mqttConnected) {
    setRgb(false, false, true);
    return;
  }
  if (currentMode == MODE_FALL) {
    setRgb(true, false, false);
  } else if (currentMode == MODE_NORMAL) {
    setRgb(false, true, false);
  } else {
    setRgb(true, true, false);
  }
}

static void playModeTone(SimMode mode) {
  if (mode == MODE_FALL) {
    tone(BUZZER_PIN, 1200, 700);
  } else if (mode != MODE_NORMAL) {
    tone(BUZZER_PIN, 780, 180);
  } else {
    tone(BUZZER_PIN, 440, 80);
  }
}

static void renderDisplay() {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("HEALTH BAND  ");
  display.println(mqttConnected ? "MQTT" : "OFF");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 15);
  display.print(latestData.heartRate);
  display.setTextSize(1);
  display.print(" BPM");

  display.setTextSize(2);
  display.setCursor(68, 15);
  display.print(latestData.spo2);
  display.setTextSize(1);
  display.print("%");

  display.setTextSize(1);
  display.setCursor(0, 39);
  display.print("Steps ");
  display.print(steps);
  display.setCursor(72, 39);
  display.print("Bat ");
  display.print(battery);
  display.print("%");

  display.fillRect(0, 51, 128, 13, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(3, 54);
  display.print(modeShortName(currentMode));
  if (latestData.fallDetected) display.print("  ALERT");
  display.display();
}

static void publishEvent(const char* eventType,
                         const char* requestId,
                         const char* command,
                         const char* value,
                         const char* message) {
  if (!mqttClient.connected()) return;
  StaticJsonDocument<384> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["eventType"] = eventType;
  doc["requestId"] = requestId ? requestId : "";
  doc["command"] = command ? command : "";
  if (value && strlen(value)) doc["value"] = value;
  doc["activeMode"] = modeName(currentMode);
  doc["timestamp"] = millis();
  doc["message"] = message;
  char buffer[384];
  size_t length = serializeJson(doc, buffer, sizeof(buffer));
  mqttClient.publish(TOPIC_EVENT, reinterpret_cast<const uint8_t*>(buffer), length, false);
  Serial.print("[EVENT] ");
  Serial.println(buffer);
}

static void publishStatus(bool online) {
  StaticJsonDocument<192> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["online"] = online;
  doc["uptime"] = millis();
  doc["firmwareVersion"] = FW_VERSION;
  doc["activeMode"] = modeName(currentMode);
  char buffer[192];
  size_t length = serializeJson(doc, buffer, sizeof(buffer));
  mqttClient.publish(TOPIC_STATUS, reinterpret_cast<const uint8_t*>(buffer), length, true);
}

static void publishTelemetry() {
  latestData = generateHealthData();
  if (currentMode != MODE_FALL) steps += randRange(0, 2);
  if (sequenceNumber % 20 == 0 && battery > 0) battery--;
  if (currentMode == MODE_LOW_BATTERY) battery = constrain(battery, 12, 19);

  StaticJsonDocument<320> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["timestamp"] = millis();
  doc["seq"] = sequenceNumber++;
  doc["heartRate"] = latestData.heartRate;
  doc["spo2"] = latestData.spo2;
  doc["steps"] = steps;
  doc["fallDetected"] = latestData.fallDetected;
  doc["battery"] = battery;
  doc["signalQuality"] = latestData.signalQuality;
  doc["mode"] = modeName(currentMode);

  char buffer[320];
  size_t length = serializeJson(doc, buffer, sizeof(buffer));
  bool published = mqttClient.publish(
    TOPIC_TELEMETRY,
    reinterpret_cast<const uint8_t*>(buffer),
    length,
    false
  );

  if (published) {
    Serial.print("[TELEMETRY] ");
    Serial.println(buffer);
  } else {
    Serial.println("[MQTT] Telemetry publish failed");
  }
  updateRgbState();
  renderDisplay();
}

static void handleCommand(const JsonDocument& doc) {
  const char* requestId = doc["requestId"] | "";
  const char* command = doc["command"];
  if (!command) {
    publishEvent("COMMAND_REJECTED", requestId, "", "", "Missing command");
    return;
  }

  if (strcmp(command, "setMode") == 0) {
    const char* value = doc["value"];
    SimMode requestedMode;
    if (!parseMode(value, requestedMode)) {
      publishEvent("COMMAND_REJECTED", requestId, command, value, "Unsupported mode");
      return;
    }
    currentMode = requestedMode;
    playModeTone(currentMode);
    updateRgbState();
    renderDisplay();
    publishEvent("COMMAND_ACCEPTED", requestId, command, value, "Scenario changed");
    return;
  }

  if (strcmp(command, "resetSteps") == 0) {
    steps = 0;
    renderDisplay();
    publishEvent("COMMAND_ACCEPTED", requestId, command, "", "Step counter reset");
    return;
  }

  publishEvent("COMMAND_REJECTED", requestId, command, "", "Unsupported command");
}

static void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print("[MQTT] Invalid command JSON: ");
    Serial.println(error.c_str());
    publishEvent("COMMAND_REJECTED", "", "", "", "Invalid command JSON");
    return;
  }
  Serial.print("[COMMAND] ");
  serializeJson(doc, Serial);
  Serial.println();
  handleCommand(doc);
}

static void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CH);
  Serial.print("[WiFi] Connecting");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(200);
    Serial.print(".");
    attempts++;
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " connected" : " failed");
}

static bool connectMqtt() {
  String clientId = String(DEVICE_ID) + "-" + String(millis() % 100000);
  StaticJsonDocument<192> lastWill;
  lastWill["deviceId"] = DEVICE_ID;
  lastWill["online"] = false;
  lastWill["uptime"] = 0;
  lastWill["firmwareVersion"] = FW_VERSION;
  lastWill["activeMode"] = modeName(currentMode);
  char lastWillBuffer[192];
  serializeJson(lastWill, lastWillBuffer, sizeof(lastWillBuffer));

  bool connected = mqttClient.connect(
    clientId.c_str(),
    TOPIC_STATUS,
    1,
    true,
    lastWillBuffer
  );

  if (connected) {
    mqttConnected = true;
    mqttClient.subscribe(TOPIC_COMMAND, 1);
    publishStatus(true);
    publishEvent("DEVICE_STARTED", "", "", "", "Health Band connected");
    Serial.println("[MQTT] Connected and subscribed");
  } else {
    mqttConnected = false;
    Serial.print("[MQTT] Connection failed, state=");
    Serial.println(mqttClient.state());
  }
  updateRgbState();
  renderDisplay();
  return connected;
}

static void handleFallButton(unsigned long now) {
  if (digitalRead(FALL_BUTTON_PIN) != LOW) return;
  if (now - lastButtonMs < BUTTON_DEBOUNCE_MS) return;
  lastButtonMs = now;
  currentMode = MODE_FALL;
  playModeTone(currentMode);
  updateRgbState();
  renderDisplay();
  publishEvent("LOCAL_FALL_BUTTON", "wokwi-button", "setMode", "fall", "Physical FALL button pressed");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== HEALTH BAND DIGITAL TWIN v0.2.0 ===");

  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(RGB_R_PIN, OUTPUT);
  pinMode(RGB_G_PIN, OUTPUT);
  pinMode(RGB_B_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FALL_BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  oledReady = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (oledReady) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(8, 22);
    display.println("HEALTH BAND");
    display.setCursor(17, 36);
    display.println("Starting...");
    display.display();
  }

  randomSeed(esp_random());
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setKeepAlive(15);
  connectWiFi();
  connectMqtt();
  renderDisplay();
}

void loop() {
  unsigned long now = millis();

  if (!mqttClient.connected()) {
    mqttConnected = false;
    if (now - lastReconnectMs >= RECONNECT_INTERVAL_MS) {
      lastReconnectMs = now;
      if (WiFi.status() != WL_CONNECTED) connectWiFi();
      connectMqtt();
    }
  } else {
    mqttClient.loop();
  }

  unsigned long blinkInterval = mqttConnected ? BLINK_SLOW_MS : BLINK_FAST_MS;
  if (now - lastBlinkMs >= blinkInterval) {
    lastBlinkMs = now;
    statusLedState = !statusLedState;
    digitalWrite(STATUS_LED_PIN, statusLedState);
  }

  handleFallButton(now);

  if (now - lastTelemetryMs >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = now;
    publishTelemetry();
  }
}
