#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP085.h>
#include <Preferences.h>
#include <time.h>

// Dedicated UART2 keeps Wokwi serial output independent from ESP32 UART0/boot logs.
HardwareSerial WokwiSerial(2);
#define Serial WokwiSerial

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
constexpr uint8_t SERIAL_RX_PIN = 16;
constexpr uint8_t SERIAL_TX_PIN = 17;
constexpr uint8_t HEART_RATE_SENSOR_PIN = 34;
constexpr uint8_t SPO2_SENSOR_PIN = 35;
constexpr uint8_t BODY_TEMPERATURE_PIN = 5;
constexpr uint8_t LIGHT_SENSOR_PIN = 33;
constexpr uint8_t HAPTIC_PIN = 23;

constexpr unsigned long BLINK_FAST_MS = 200;
constexpr unsigned long BLINK_SLOW_MS = 1000;
constexpr unsigned long LIVE_TELEMETRY_INTERVAL_MS = 2000;
constexpr unsigned long ECO_TELEMETRY_INTERVAL_MS = 8000;
constexpr unsigned long RECONNECT_INTERVAL_MS = 5000;
constexpr unsigned long DIAGNOSTIC_INTERVAL_MS = 5000;
constexpr unsigned long STATUS_HEARTBEAT_INTERVAL_MS = 30000;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 250;
constexpr unsigned long MOTION_SAMPLE_INTERVAL_MS = 100;
constexpr unsigned long FALL_LATCH_MS = 3000;
constexpr unsigned long ENVIRONMENT_SAMPLE_INTERVAL_MS = 2000;
constexpr unsigned long CHECKPOINT_INTERVAL_MS = 30000;
// Demo thresholds are intentionally accelerated. A real wearable normally
// requires 10-15 minutes of stillness before confirming sleep.
constexpr unsigned long SLEEP_CANDIDATE_MS = 15000;
constexpr unsigned long SLEEP_CONFIRM_MS = 30000;
constexpr unsigned long DEEP_SLEEP_MS = 60000;
constexpr uint16_t MQTT_BUFFER_SIZE = 2300;

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
constexpr uint8_t WIFI_CH = 6;

const char* MQTT_HOST = "broker.emqx.io";
constexpr uint16_t MQTT_PORT = 1883;

const char* DEVICE_ID = "health-band-01";
const char* FW_VERSION = "0.6.0";

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
  MODE_LOW_BATTERY,
  MODE_SLEEP
};

enum UserProfile {
  PROFILE_STUDENT,
  PROFILE_OLDER_ADULT,
  PROFILE_ATHLETE,
  PROFILE_CHILD
};

enum PowerMode {
  POWER_LIVE,
  POWER_ECO
};

enum UserGender {
  GENDER_MALE,
  GENDER_FEMALE
};

enum WearMode {
  WEAR_AUTO,
  WEAR_FORCE_WORN,
  WEAR_FORCE_OFF_WRIST
};

enum SleepStage {
  SLEEP_NOT_TRACKED,
  SLEEP_AWAKE,
  SLEEP_CANDIDATE,
  SLEEP_LIGHT,
  SLEEP_DEEP
};

struct HealthData {
  int heartRate;
  int spo2;
  int systolic;
  int diastolic;
  bool fallDetected;
  const char* signalQuality;
  int heartRateRaw;
  int spo2Raw;
  float accelX;
  float accelY;
  float accelZ;
  float accelerationMagnitude;
  bool motionDetected;
};

struct EnvironmentData {
  float bodyTemperatureC;
  float ambientTemperatureC;
  int32_t pressurePa;
  float altitudeM;
  float ambientLightLux;
  int lightRaw;
  const char* displayMode;
};

struct GpsData {
  double latitude;
  double longitude;
  bool valid;
  unsigned long updatedAt;
  const char* source;
};

SimMode currentMode = MODE_NORMAL;
UserProfile currentProfile = PROFILE_STUDENT;
PowerMode selectedPowerMode = POWER_LIVE;
UserGender currentGender = GENDER_MALE;
WearMode selectedWearMode = WEAR_AUTO;
unsigned long sequenceNumber = 0;
unsigned long steps = 0;
int battery = 100;
HealthData latestData = {0, 0, 0, 0, false, "unknown", 0, 0, 0, 0, 0, 0, false};

unsigned long lastTelemetryMs = 0;
unsigned long lastBlinkMs = 0;
unsigned long lastReconnectMs = 0;
unsigned long lastButtonMs = 0;
unsigned long lastDiagnosticMs = 0;
unsigned long lastStatusMs = 0;
bool statusLedState = false;
bool mqttConnected = false;
bool oledReady = false;
bool mpuReady = false;
unsigned long lastStepDetectedMs = 0;
unsigned long lastMotionSampleMs = 0;
unsigned long lastSensorFallMs = 0;
float sensorAccelX = 0;
float sensorAccelY = 0;
float sensorAccelZ = 9.81f;
float sensorAccelerationMagnitude = 9.81f;
bool sensorMotionDetected = false;
unsigned long lastEnvironmentSampleMs = 0;
unsigned long hapticUntilMs = 0;
bool temperatureSensorReady = false;
bool bmpReady = false;
EnvironmentData environmentData = {36.6f, 28.0f, 101325, 0.0f, 500.0f, 0, "bright"};
GpsData gpsData = {0, 0, false, 0, "serial_nmea"};
String gpsInputLine;
bool isWearing = true;
bool vitalDataValid = true;
SleepStage currentSleepStage = SLEEP_AWAKE;
unsigned long sleepCandidateSinceMs = 0;
unsigned long sleepDetectedSinceMs = 0;
int sleepConfidence = 0;
bool sleepTimeWindowMatched = false;
bool stateRestored = false;
unsigned long checkpointSequence = 0;
unsigned long lastCheckpointMs = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_MPU6050 mpu;
OneWire oneWire(BODY_TEMPERATURE_PIN);
DallasTemperature bodyTemperatureSensors(&oneWire);
Adafruit_BMP085 bmp;
Preferences preferences;

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
    case MODE_SLEEP: return "sleep";
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
    case MODE_SLEEP: return "SLEEP";
    default: return "UNKNOWN";
  }
}

static const char* profileName(UserProfile profile) {
  switch (profile) {
    case PROFILE_STUDENT: return "student";
    case PROFILE_OLDER_ADULT: return "older_adult";
    case PROFILE_ATHLETE: return "athlete";
    case PROFILE_CHILD: return "child";
    default: return "student";
  }
}

static const char* powerModeName(PowerMode mode) {
  return mode == POWER_ECO ? "eco" : "normal";
}

static const char* genderName(UserGender gender) {
  return gender == GENDER_FEMALE ? "female" : "male";
}

static const char* wearModeName(WearMode mode) {
  switch (mode) {
    case WEAR_FORCE_WORN: return "worn";
    case WEAR_FORCE_OFF_WRIST: return "off_wrist";
    default: return "auto";
  }
}

static const char* sleepStageName(SleepStage stage) {
  switch (stage) {
    case SLEEP_AWAKE: return "awake";
    case SLEEP_CANDIDATE: return "candidate";
    case SLEEP_LIGHT: return "light";
    case SLEEP_DEEP: return "deep";
    default: return "not_tracked";
  }
}

static PowerMode effectivePowerMode() {
  return battery <= 20 ? POWER_ECO : selectedPowerMode;
}

static unsigned long telemetryIntervalMs() {
  return effectivePowerMode() == POWER_ECO
    ? ECO_TELEMETRY_INTERVAL_MS
    : LIVE_TELEMETRY_INTERVAL_MS;
}

static bool parseMode(const char* value, SimMode& result) {
  if (!value) return false;
  if (strcmp(value, "normal") == 0) result = MODE_NORMAL;
  else if (strcmp(value, "high_hr") == 0) result = MODE_HIGH_HR;
  else if (strcmp(value, "low_hr") == 0) result = MODE_LOW_HR;
  else if (strcmp(value, "low_spo2") == 0) result = MODE_LOW_SPO2;
  else if (strcmp(value, "fall") == 0) result = MODE_FALL;
  else if (strcmp(value, "low_battery") == 0) result = MODE_LOW_BATTERY;
  else if (strcmp(value, "sleep") == 0) result = MODE_SLEEP;
  else return false;
  return true;
}

static bool parseProfile(const char* value, UserProfile& result) {
  if (!value) return false;
  if (strcmp(value, "student") == 0) result = PROFILE_STUDENT;
  else if (strcmp(value, "older_adult") == 0) result = PROFILE_OLDER_ADULT;
  else if (strcmp(value, "athlete") == 0) result = PROFILE_ATHLETE;
  else if (strcmp(value, "child") == 0) result = PROFILE_CHILD;
  else return false;
  return true;
}

static bool parsePowerMode(const char* value, PowerMode& result) {
  if (!value) return false;
  if (strcmp(value, "normal") == 0) result = POWER_LIVE;
  else if (strcmp(value, "eco") == 0) result = POWER_ECO;
  else return false;
  return true;
}

static bool parseGender(const char* value, UserGender& result) {
  if (!value) return false;
  if (strcmp(value, "male") == 0) result = GENDER_MALE;
  else if (strcmp(value, "female") == 0) result = GENDER_FEMALE;
  else return false;
  return true;
}

static bool parseWearMode(const char* value, WearMode& result) {
  if (!value) return false;
  if (strcmp(value, "auto") == 0) result = WEAR_AUTO;
  else if (strcmp(value, "worn") == 0) result = WEAR_FORCE_WORN;
  else if (strcmp(value, "off_wrist") == 0) result = WEAR_FORCE_OFF_WRIST;
  else return false;
  return true;
}

static void saveCheckpoint(bool force = false) {
  unsigned long now = millis();
  if (!force && now - lastCheckpointMs < CHECKPOINT_INTERVAL_MS) return;
  lastCheckpointMs = now;
  checkpointSequence++;
  preferences.putBool("valid", true);
  preferences.putULong("steps", steps);
  preferences.putInt("battery", battery);
  preferences.putUChar("mode", static_cast<uint8_t>(currentMode));
  preferences.putUChar("profile", static_cast<uint8_t>(currentProfile));
  preferences.putUChar("gender", static_cast<uint8_t>(currentGender));
  preferences.putUChar("power", static_cast<uint8_t>(selectedPowerMode));
  preferences.putUChar("wear", static_cast<uint8_t>(selectedWearMode));
  preferences.putULong("checkpoint", checkpointSequence);
}

static void restoreCheckpoint() {
  preferences.begin("health-band", false);
  if (!preferences.getBool("valid", false)) return;
  steps = preferences.getULong("steps", 0);
  battery = constrain(preferences.getInt("battery", 100), 0, 100);
  currentMode = static_cast<SimMode>(
    constrain(static_cast<int>(preferences.getUChar("mode", MODE_NORMAL)),
              static_cast<int>(MODE_NORMAL), static_cast<int>(MODE_SLEEP))
  );
  currentProfile = static_cast<UserProfile>(
    constrain(static_cast<int>(preferences.getUChar("profile", PROFILE_STUDENT)),
              static_cast<int>(PROFILE_STUDENT), static_cast<int>(PROFILE_CHILD))
  );
  currentGender = static_cast<UserGender>(
    constrain(static_cast<int>(preferences.getUChar("gender", GENDER_MALE)),
              static_cast<int>(GENDER_MALE), static_cast<int>(GENDER_FEMALE))
  );
  selectedPowerMode = static_cast<PowerMode>(
    constrain(static_cast<int>(preferences.getUChar("power", POWER_LIVE)),
              static_cast<int>(POWER_LIVE), static_cast<int>(POWER_ECO))
  );
  selectedWearMode = static_cast<WearMode>(
    constrain(static_cast<int>(preferences.getUChar("wear", WEAR_AUTO)),
              static_cast<int>(WEAR_AUTO), static_cast<int>(WEAR_FORCE_OFF_WRIST))
  );
  checkpointSequence = preferences.getULong("checkpoint", 0);
  stateRestored = true;
}

static double nmeaCoordinateToDecimal(const char* coordinate, const char* hemisphere) {
  if (!coordinate || !hemisphere || strlen(coordinate) < 4) return 0;
  double raw = atof(coordinate);
  int degrees = static_cast<int>(raw / 100.0);
  double minutes = raw - degrees * 100.0;
  double decimal = degrees + minutes / 60.0;
  if (hemisphere[0] == 'S' || hemisphere[0] == 'W') decimal = -decimal;
  return decimal;
}

static bool parseGpsNmea(const char* sentence) {
  if (!sentence || strncmp(sentence, "$GPRMC", 6) != 0) return false;
  char copy[160];
  strlcpy(copy, sentence, sizeof(copy));
  char* fields[16] = {};
  size_t count = 0;
  char* token = strtok(copy, ",");
  while (token && count < 16) {
    fields[count++] = token;
    token = strtok(nullptr, ",");
  }
  if (count < 7 || strcmp(fields[2], "A") != 0) return false;
  gpsData.latitude = nmeaCoordinateToDecimal(fields[3], fields[4]);
  gpsData.longitude = nmeaCoordinateToDecimal(fields[5], fields[6]);
  gpsData.valid = true;
  gpsData.updatedAt = millis();
  gpsData.source = "serial_nmea";
  Serial.print("[GPS] Fix ");
  Serial.print(gpsData.latitude, 6);
  Serial.print(",");
  Serial.println(gpsData.longitude, 6);
  return true;
}

static void handleGpsSerialInput() {
  while (Serial.available()) {
    char value = static_cast<char>(Serial.read());
    if (value == '\r') continue;
    if (value == '\n') {
      gpsInputLine.trim();
      if (gpsInputLine.length()) {
        if (!parseGpsNmea(gpsInputLine.c_str())) {
          Serial.println("[GPS] Ignored input; expected valid $GPRMC sentence");
        }
      }
      gpsInputLine = "";
    } else if (gpsInputLine.length() < 150) {
      gpsInputLine += value;
    }
  }
}

static void startHaptic(unsigned long durationMs) {
  digitalWrite(HAPTIC_PIN, HIGH);
  hapticUntilMs = millis() + durationMs;
}

static void updateHaptic(unsigned long now) {
  if (hapticUntilMs > 0 && static_cast<long>(now - hapticUntilMs) >= 0) {
    digitalWrite(HAPTIC_PIN, LOW);
    hapticUntilMs = 0;
  }
}

static float calculateLux(int rawValue) {
  if (rawValue <= 0) return 100000.0f;
  if (rawValue >= 4094) return 0.1f;
  const float gamma = 0.7f;
  const float rl10 = 50.0f;
  float scaled = rawValue / 4095.0f * 1023.0f;
  float voltage = scaled / 1024.0f * 5.0f;
  float resistance = 2000.0f * voltage / (1.0f - voltage / 5.0f);
  float lux = pow(rl10 * 1000.0f * pow(10.0f, gamma) / resistance, 1.0f / gamma);
  return isfinite(lux) ? constrain(lux, 0.1f, 100000.0f) : 100000.0f;
}

static void applyDisplayContrast() {
  if (!oledReady) return;
  uint8_t contrast = environmentData.ambientLightLux < 100.0f ? 35 : 255;
  environmentData.displayMode = contrast < 100 ? "dim" : "bright";
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(contrast);
}

static void sampleEnvironmentSensors(unsigned long now) {
  if (now - lastEnvironmentSampleMs < ENVIRONMENT_SAMPLE_INTERVAL_MS) return;
  lastEnvironmentSampleMs = now;

  bodyTemperatureSensors.requestTemperatures();
  float bodyTemperature = bodyTemperatureSensors.getTempCByIndex(0);
  temperatureSensorReady = bodyTemperature != DEVICE_DISCONNECTED_C;
  if (temperatureSensorReady) environmentData.bodyTemperatureC = bodyTemperature;

  environmentData.lightRaw = analogRead(LIGHT_SENSOR_PIN);
  environmentData.ambientLightLux = calculateLux(environmentData.lightRaw);
  applyDisplayContrast();

  if (bmpReady) {
    environmentData.ambientTemperatureC = bmp.readTemperature();
    environmentData.pressurePa = bmp.readPressure();
    environmentData.altitudeM = bmp.readAltitude();
  }
}

static void sampleMotionSensor(unsigned long now) {
  if (!mpuReady || now - lastMotionSampleMs < MOTION_SAMPLE_INTERVAL_MS) return;
  lastMotionSampleMs = now;

  sensors_event_t acceleration;
  sensors_event_t gyroscope;
  sensors_event_t temperature;
  mpu.getEvent(&acceleration, &gyroscope, &temperature);
  sensorAccelX = acceleration.acceleration.x;
  sensorAccelY = acceleration.acceleration.y;
  sensorAccelZ = acceleration.acceleration.z;
  sensorAccelerationMagnitude = sqrt(
    sensorAccelX * sensorAccelX +
    sensorAccelY * sensorAccelY +
    sensorAccelZ * sensorAccelZ
  );

  const float gravityDelta = fabs(sensorAccelerationMagnitude - 9.80665f);
  sensorMotionDetected = gravityDelta >= 2.2f;
  const bool fallNow = sensorAccelerationMagnitude < 2.5f ||
                       sensorAccelerationMagnitude > 24.0f;
  if (fallNow) lastSensorFallMs = now;

  if (sensorMotionDetected && !fallNow &&
      now - lastStepDetectedMs >= 350) {
    steps++;
    lastStepDetectedMs = now;
  }
}

static HealthData readSensorData() {
  HealthData data = {80, 98, 118, 76, false, "good", 0, 0, 0, 0, 9.81f, 9.81f, false};
  data.heartRateRaw = analogRead(HEART_RATE_SENSOR_PIN);
  data.spo2Raw = analogRead(SPO2_SENSOR_PIN);
  data.heartRate = constrain(map(data.heartRateRaw, 0, 4095, 40, 180), 40, 180);
  data.spo2 = constrain(map(data.spo2Raw, 0, 4095, 80, 100), 80, 100);

  if (data.heartRateRaw < 80 || data.heartRateRaw > 4015 ||
      data.spo2Raw < 80 || data.spo2Raw > 4015) {
    data.signalQuality = "poor";
  } else if (data.heartRateRaw < 250 || data.spo2Raw < 250) {
    data.signalQuality = "medium";
  }

  if (mpuReady) {
    data.accelX = sensorAccelX;
    data.accelY = sensorAccelY;
    data.accelZ = sensorAccelZ;
    data.accelerationMagnitude = sensorAccelerationMagnitude;
    data.motionDetected = sensorMotionDetected;
    data.fallDetected = lastSensorFallMs > 0 &&
                        millis() - lastSensorFallMs <= FALL_LATCH_MS;
  } else {
    data.signalQuality = "poor";
  }
  return data;
}

static HealthData applyScenarioOverride(HealthData data) {
  switch (currentMode) {
    case MODE_NORMAL:
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
    case MODE_SLEEP:
      data.heartRate = randRange(55, 65);
      data.spo2 = randRange(94, 98);
      data.motionDetected = false;
      data.accelerationMagnitude = 9.81f;
      data.signalQuality = "good";
      break;
  }
  return data;
}

static void estimateBloodPressure(HealthData& data) {
  // This is a transparent classroom estimate, not a cuff measurement.
  int baseSystolic = 118;
  int baseDiastolic = 76;
  if (currentProfile == PROFILE_CHILD) {
    baseSystolic = 105;
    baseDiastolic = 68;
  } else if (currentProfile == PROFILE_OLDER_ADULT) {
    baseSystolic = 126;
    baseDiastolic = 78;
  } else if (currentProfile == PROFILE_ATHLETE) {
    baseSystolic = 114;
    baseDiastolic = 72;
  }
  int heartRateOffset = constrain((data.heartRate - 75) / 4, -10, 18);
  data.systolic = constrain(baseSystolic + heartRateOffset + randRange(-2, 2), 80, 180);
  data.diastolic = constrain(baseDiastolic + heartRateOffset / 2 + randRange(-2, 2), 50, 120);
}

static void resetSleepTracking() {
  currentSleepStage = isWearing ? SLEEP_AWAKE : SLEEP_NOT_TRACKED;
  sleepCandidateSinceMs = 0;
  sleepDetectedSinceMs = 0;
  sleepConfidence = 0;
}

static bool isSleepTimeWindow() {
  struct tm localTime;
  if (!getLocalTime(&localTime, 5)) return currentMode == MODE_SLEEP;
  return localTime.tm_hour >= 21 || localTime.tm_hour < 9;
}

static void updateWearAndSleep(HealthData& data, unsigned long now) {
  sleepTimeWindowMatched = false;
  bool automaticWearing =
    data.heartRateRaw >= 80 && data.heartRateRaw <= 4015 &&
    data.spo2Raw >= 80 && data.spo2Raw <= 4015 &&
    environmentData.bodyTemperatureC >= 30.0f;

  if (selectedWearMode == WEAR_FORCE_WORN) isWearing = true;
  else if (selectedWearMode == WEAR_FORCE_OFF_WRIST) isWearing = false;
  else isWearing = automaticWearing;

  vitalDataValid = isWearing && strcmp(data.signalQuality, "poor") != 0;
  if (!isWearing) {
    data.heartRate = 0;
    data.spo2 = 0;
    data.systolic = 0;
    data.diastolic = 0;
    data.fallDetected = false;
    resetSleepTracking();
    return;
  }

  sleepTimeWindowMatched = isSleepTimeWindow();
  const bool sleepEvidence =
    !data.motionDetected &&
    data.heartRate >= 45 && data.heartRate <= 70 &&
    data.spo2 >= 90 &&
    environmentData.bodyTemperatureC >= 30.0f &&
    (sleepTimeWindowMatched || currentMode == MODE_SLEEP);

  if (!sleepEvidence) {
    resetSleepTracking();
    return;
  }

  if (sleepCandidateSinceMs == 0) sleepCandidateSinceMs = now;
  unsigned long stillDuration = now - sleepCandidateSinceMs;
  if (stillDuration < SLEEP_CANDIDATE_MS) {
    currentSleepStage = SLEEP_CANDIDATE;
    sleepConfidence = 45;
  } else if (stillDuration < SLEEP_CONFIRM_MS) {
    currentSleepStage = SLEEP_CANDIDATE;
    sleepConfidence = 70;
  } else {
    if (sleepDetectedSinceMs == 0) {
      // Back-date to the beginning of the verified stillness period.
      sleepDetectedSinceMs = sleepCandidateSinceMs;
    }
    currentSleepStage = stillDuration >= DEEP_SLEEP_MS ? SLEEP_DEEP : SLEEP_LIGHT;
    sleepConfidence = currentSleepStage == SLEEP_DEEP ? 92 : 85;
  }
}

static HealthData readHealthData() {
  HealthData data = applyScenarioOverride(readSensorData());
  estimateBloodPressure(data);
  updateWearAndSleep(data, millis());
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
    startHaptic(1200);
  } else if (mode != MODE_NORMAL) {
    tone(BUZZER_PIN, 780, 180);
    startHaptic(350);
  } else {
    tone(BUZZER_PIN, 440, 80);
    startHaptic(120);
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
  display.print(isWearing ? String(latestData.heartRate) : "--");
  display.setTextSize(1);
  display.print(" BPM");

  display.setTextSize(2);
  display.setCursor(68, 15);
  display.print(isWearing ? String(latestData.spo2) : "--");
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
  if (effectivePowerMode() == POWER_ECO) display.print(" ECO");
  if (!isWearing) display.print(" OFF");
  else if (currentSleepStage == SLEEP_LIGHT || currentSleepStage == SLEEP_DEEP) display.print(" Zz");
  if (latestData.fallDetected) display.print("  ALERT");
  display.display();
}

static void publishEvent(const char* eventType,
                         const char* requestId,
                         const char* command,
                         const char* value,
                         const char* message) {
  if (!mqttClient.connected()) return;
  StaticJsonDocument<512> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["eventType"] = eventType;
  doc["requestId"] = requestId ? requestId : "";
  doc["command"] = command ? command : "";
  if (value && strlen(value)) doc["value"] = value;
  doc["activeMode"] = modeName(currentMode);
  doc["profile"] = profileName(currentProfile);
  doc["gender"] = genderName(currentGender);
  doc["wearing"] = isWearing;
  doc["wearMode"] = wearModeName(selectedWearMode);
  doc["sleepStage"] = sleepStageName(currentSleepStage);
  doc["powerMode"] = powerModeName(effectivePowerMode());
  doc["timestamp"] = millis();
  doc["message"] = message;
  char buffer[512];
  size_t length = serializeJson(doc, buffer, sizeof(buffer));
  mqttClient.publish(TOPIC_EVENT, reinterpret_cast<const uint8_t*>(buffer), length, false);
  Serial.print("[EVENT] ");
  Serial.println(buffer);
}

static void publishStatus(bool online) {
  StaticJsonDocument<512> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["online"] = online;
  doc["uptime"] = millis();
  doc["firmwareVersion"] = FW_VERSION;
  doc["activeMode"] = modeName(currentMode);
  doc["profile"] = profileName(currentProfile);
  doc["gender"] = genderName(currentGender);
  doc["wearing"] = isWearing;
  doc["wearMode"] = wearModeName(selectedWearMode);
  doc["sleepStage"] = sleepStageName(currentSleepStage);
  doc["stateRestored"] = stateRestored;
  doc["checkpointSequence"] = checkpointSequence;
  doc["powerMode"] = powerModeName(effectivePowerMode());
  doc["samplingIntervalMs"] = telemetryIntervalMs();
  char buffer[512];
  size_t length = serializeJson(doc, buffer, sizeof(buffer));
  mqttClient.publish(TOPIC_STATUS, reinterpret_cast<const uint8_t*>(buffer), length, true);
}

static void publishTelemetry() {
  bool previousFall = latestData.fallDetected;
  latestData = readHealthData();
  if (latestData.fallDetected && !previousFall) {
    tone(BUZZER_PIN, 1200, 700);
    startHaptic(1200);
  }
  if (currentMode != MODE_NORMAL && currentMode != MODE_FALL) {
    steps += randRange(0, 2);
  }
  if (sequenceNumber % 20 == 0 && battery > 0) battery--;
  if (currentMode == MODE_LOW_BATTERY) battery = constrain(battery, 12, 19);

  StaticJsonDocument<2048> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["timestamp"] = millis();
  doc["seq"] = sequenceNumber++;
  doc["heartRate"] = latestData.heartRate;
  doc["spo2"] = latestData.spo2;
  JsonObject bloodPressure = doc.createNestedObject("bloodPressure");
  bloodPressure["systolic"] = latestData.systolic;
  bloodPressure["diastolic"] = latestData.diastolic;
  bloodPressure["source"] = "simulated_estimate";
  bloodPressure["medicalGrade"] = false;
  doc["steps"] = steps;
  doc["fallDetected"] = latestData.fallDetected;
  doc["battery"] = battery;
  doc["signalQuality"] = latestData.signalQuality;
  doc["mode"] = modeName(currentMode);
  doc["profile"] = profileName(currentProfile);
  doc["gender"] = genderName(currentGender);
  doc["wearing"] = isWearing;
  doc["wearMode"] = wearModeName(selectedWearMode);
  doc["vitalDataValid"] = vitalDataValid;
  doc["powerMode"] = powerModeName(effectivePowerMode());
  doc["samplingIntervalMs"] = telemetryIntervalMs();
  doc["dataSource"] = currentMode == MODE_NORMAL ? "sensors" : "scenario_override";
  doc["sensorHealth"] = mpuReady ? "ok" : "degraded";
  doc["heartRateRaw"] = latestData.heartRateRaw;
  doc["spo2Raw"] = latestData.spo2Raw;
  JsonObject motion = doc.createNestedObject("motion");
  motion["accelX"] = serialized(String(latestData.accelX, 2));
  motion["accelY"] = serialized(String(latestData.accelY, 2));
  motion["accelZ"] = serialized(String(latestData.accelZ, 2));
  motion["magnitude"] = serialized(String(latestData.accelerationMagnitude, 2));
  motion["detected"] = latestData.motionDetected;
  doc["bodyTemperatureC"] = serialized(String(environmentData.bodyTemperatureC, 1));
  doc["ambientLightLux"] = serialized(String(environmentData.ambientLightLux, 1));
  doc["displayMode"] = environmentData.displayMode;
  JsonObject environment = doc.createNestedObject("environment");
  environment["temperatureC"] = serialized(String(environmentData.ambientTemperatureC, 1));
  environment["pressurePa"] = environmentData.pressurePa;
  environment["altitudeM"] = serialized(String(environmentData.altitudeM, 1));
  JsonObject location = doc.createNestedObject("location");
  location["latitude"] = serialized(String(gpsData.latitude, 6));
  location["longitude"] = serialized(String(gpsData.longitude, 6));
  location["valid"] = gpsData.valid;
  location["source"] = gpsData.source;
  location["updatedAt"] = gpsData.updatedAt;
  doc["hapticActive"] = hapticUntilMs > 0;
  JsonObject sleep = doc.createNestedObject("sleep");
  sleep["stage"] = sleepStageName(currentSleepStage);
  sleep["confidence"] = sleepConfidence;
  sleep["candidateSinceMs"] = sleepCandidateSinceMs;
  sleep["detectedStartMs"] = sleepDetectedSinceMs;
  sleep["durationMs"] = sleepDetectedSinceMs > 0 ? millis() - sleepDetectedSinceMs : 0;
  sleep["model"] = "sensor_fusion_demo";
  sleep["timeWindowMatched"] = sleepTimeWindowMatched;
  JsonObject recovery = doc.createNestedObject("recovery");
  recovery["stateRestored"] = stateRestored;
  recovery["checkpointSequence"] = checkpointSequence;
  recovery["checkpointIntervalMs"] = CHECKPOINT_INTERVAL_MS;

  char buffer[2048];
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
    Serial.print("[MQTT] Telemetry publish failed; payload=");
    Serial.print(length);
    Serial.print(" bytes, MQTT buffer=");
    Serial.println(mqttClient.getBufferSize());
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
    resetSleepTracking();
    playModeTone(currentMode);
    updateRgbState();
    renderDisplay();
    publishEvent("COMMAND_ACCEPTED", requestId, command, value, "Scenario changed");
    publishStatus(true);
    saveCheckpoint(true);
    return;
  }

  if (strcmp(command, "resetSteps") == 0) {
    steps = 0;
    renderDisplay();
    publishEvent("COMMAND_ACCEPTED", requestId, command, "", "Step counter reset");
    saveCheckpoint(true);
    return;
  }

  if (strcmp(command, "setProfile") == 0) {
    const char* value = doc["value"];
    UserProfile requestedProfile;
    if (!parseProfile(value, requestedProfile)) {
      publishEvent("COMMAND_REJECTED", requestId, command, value, "Unsupported profile");
      return;
    }
    currentProfile = requestedProfile;
    renderDisplay();
    publishEvent("COMMAND_ACCEPTED", requestId, command, value, "User profile changed");
    publishStatus(true);
    saveCheckpoint(true);
    return;
  }

  if (strcmp(command, "setGender") == 0) {
    const char* value = doc["value"];
    UserGender requestedGender;
    if (!parseGender(value, requestedGender)) {
      publishEvent("COMMAND_REJECTED", requestId, command, value, "Unsupported gender");
      return;
    }
    currentGender = requestedGender;
    publishEvent("COMMAND_ACCEPTED", requestId, command, value, "Gender metadata changed");
    publishStatus(true);
    saveCheckpoint(true);
    return;
  }

  if (strcmp(command, "setWearState") == 0) {
    const char* value = doc["value"];
    WearMode requestedWearMode;
    if (!parseWearMode(value, requestedWearMode)) {
      publishEvent("COMMAND_REJECTED", requestId, command, value, "Unsupported wear state");
      return;
    }
    selectedWearMode = requestedWearMode;
    resetSleepTracking();
    publishEvent("COMMAND_ACCEPTED", requestId, command, value, "Wear-state policy changed");
    publishStatus(true);
    saveCheckpoint(true);
    return;
  }

  if (strcmp(command, "setPowerMode") == 0) {
    const char* value = doc["value"];
    PowerMode requestedPowerMode;
    if (!parsePowerMode(value, requestedPowerMode)) {
      publishEvent("COMMAND_REJECTED", requestId, command, value, "Unsupported power mode");
      return;
    }
    selectedPowerMode = requestedPowerMode;
    lastTelemetryMs = 0;
    renderDisplay();
    publishEvent("COMMAND_ACCEPTED", requestId, command, value, "Power mode changed");
    publishStatus(true);
    saveCheckpoint(true);
    return;
  }

  if (strcmp(command, "ackAlert") == 0) {
    const char* value = doc["value"];
    if (!value || strlen(value) == 0) {
      publishEvent("COMMAND_REJECTED", requestId, command, "", "Missing alert code");
      return;
    }
    publishEvent("COMMAND_ACCEPTED", requestId, command, value, "Alert acknowledged by presenter");
    return;
  }

  if (strcmp(command, "emergencyAction") == 0) {
    const char* value = doc["value"];
    if (!value || (strcmp(value, "cancel") != 0 && strcmp(value, "send") != 0)) {
      publishEvent("COMMAND_REJECTED", requestId, command, value, "Unsupported emergency action");
      return;
    }
    if (strcmp(value, "cancel") == 0) {
      currentMode = MODE_NORMAL;
      playModeTone(currentMode);
      updateRgbState();
      renderDisplay();
    }
    publishEvent(
      "COMMAND_ACCEPTED",
      requestId,
      command,
      value,
      strcmp(value, "cancel") == 0
        ? "Simulated emergency cancelled"
        : "Simulated emergency notification sent"
    );
    publishStatus(true);
    saveCheckpoint(true);
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
  StaticJsonDocument<512> lastWill;
  lastWill["deviceId"] = DEVICE_ID;
  lastWill["online"] = false;
  lastWill["uptime"] = 0;
  lastWill["firmwareVersion"] = FW_VERSION;
  lastWill["activeMode"] = modeName(currentMode);
  lastWill["profile"] = profileName(currentProfile);
  lastWill["gender"] = genderName(currentGender);
  lastWill["wearing"] = false;
  lastWill["wearMode"] = wearModeName(selectedWearMode);
  lastWill["sleepStage"] = "not_tracked";
  lastWill["stateRestored"] = false;
  lastWill["checkpointSequence"] = checkpointSequence;
  lastWill["powerMode"] = powerModeName(effectivePowerMode());
  lastWill["samplingIntervalMs"] = telemetryIntervalMs();
  char lastWillBuffer[512];
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
    lastStatusMs = millis();
    publishEvent(
      stateRestored ? "DEVICE_RECOVERED" : "DEVICE_STARTED",
      "", "", "",
      stateRestored
        ? "Health Band restored the last local checkpoint"
        : "Health Band connected"
    );
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
  Serial.begin(115200, SERIAL_8N1, SERIAL_RX_PIN, SERIAL_TX_PIN);
  delay(300);
  Serial.println("\n=== HEALTH BAND SENSOR TWIN v0.6.0 ===");
  restoreCheckpoint();
  Serial.println(stateRestored
    ? "[RECOVERY] Restored profile, gender, steps, battery and operating mode from NVS"
    : "[RECOVERY] No previous checkpoint; using defaults");

  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(RGB_R_PIN, OUTPUT);
  pinMode(RGB_G_PIN, OUTPUT);
  pinMode(RGB_B_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FALL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(HEART_RATE_SENSOR_PIN, INPUT);
  pinMode(SPO2_SENSOR_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(HAPTIC_PIN, OUTPUT);
  digitalWrite(HAPTIC_PIN, LOW);

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  mpuReady = mpu.begin(0x68, &Wire);
  if (mpuReady) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("[SENSOR] MPU6050 ready");
  } else {
    Serial.println("[SENSOR] MPU6050 not detected; motion data degraded");
  }
  bmpReady = bmp.begin(BMP085_ULTRAHIGHRES);
  Serial.println(bmpReady
    ? "[SENSOR] BMP180 ready"
    : "[SENSOR] BMP180 not detected; environment data degraded");
  bodyTemperatureSensors.begin();
  bodyTemperatureSensors.setResolution(10);
  temperatureSensorReady = bodyTemperatureSensors.getDeviceCount() > 0;
  Serial.println(temperatureSensorReady
    ? "[SENSOR] DS18B20 ready"
    : "[SENSOR] DS18B20 not detected; body temperature degraded");
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
  if (!mqttClient.setBufferSize(MQTT_BUFFER_SIZE)) {
    Serial.println("[MQTT] ERROR: could not allocate 2300-byte MQTT buffer");
  } else {
    Serial.print("[MQTT] Buffer size: ");
    Serial.println(mqttClient.getBufferSize());
  }
  mqttClient.setKeepAlive(15);
  mqttClient.setSocketTimeout(10);
  parseGpsNmea("$GPRMC,123519,A,1045.7573,N,10639.6103,E,0.0,0.0,030826,,,A");
  lastEnvironmentSampleMs = millis() - ENVIRONMENT_SAMPLE_INTERVAL_MS;
  sampleEnvironmentSensors(millis());
  connectWiFi();
  configTime(7 * 3600, 0, "pool.ntp.org", "time.google.com");
  connectMqtt();
  renderDisplay();
}

void loop() {
  unsigned long now = millis();
  handleGpsSerialInput();
  updateHaptic(now);
  sampleEnvironmentSensors(now);

  if (!mqttClient.connected()) {
    mqttConnected = false;
    if (now - lastReconnectMs >= RECONNECT_INTERVAL_MS) {
      lastReconnectMs = now;
      if (WiFi.status() != WL_CONNECTED) connectWiFi();
      connectMqtt();
    }
  } else {
    mqttClient.loop();
    if (now - lastStatusMs >= STATUS_HEARTBEAT_INTERVAL_MS) {
      lastStatusMs = now;
      publishStatus(true);
    }
  }

  unsigned long blinkInterval = mqttConnected ? BLINK_SLOW_MS : BLINK_FAST_MS;
  if (now - lastBlinkMs >= blinkInterval) {
    lastBlinkMs = now;
    statusLedState = !statusLedState;
    digitalWrite(STATUS_LED_PIN, statusLedState);
  }

  handleFallButton(now);
  sampleMotionSensor(now);

  if (now - lastDiagnosticMs >= DIAGNOSTIC_INTERVAL_MS) {
    lastDiagnosticMs = now;
    Serial.print("[DIAG] WiFi=");
    Serial.print(WiFi.status() == WL_CONNECTED ? "connected" : "disconnected");
    Serial.print(" MQTT=");
    Serial.print(mqttClient.connected() ? "connected" : "disconnected");
    Serial.print(" RSSI=");
    Serial.print(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0);
    Serial.print(" mode=");
    Serial.print(modeName(currentMode));
    Serial.print(" interval=");
    Serial.print(telemetryIntervalMs());
    Serial.print(" source=");
    Serial.print(currentMode == MODE_NORMAL ? "sensors" : "scenario_override");
    Serial.print(" mpu=");
    Serial.print(mpuReady ? "ok" : "missing");
    Serial.println("ms");
  }

  if (now - lastTelemetryMs >= telemetryIntervalMs()) {
    lastTelemetryMs = now;
    publishTelemetry();
  }
  saveCheckpoint(false);
}
