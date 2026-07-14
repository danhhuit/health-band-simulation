#include <Arduino.h>

constexpr uint8_t STATUS_LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  pinMode(STATUS_LED_PIN, OUTPUT);
  delay(500);

  Serial.println();
  Serial.println("=================================");
  Serial.println("HEALTH BAND - WOKWI SIMULATOR");
  Serial.println("ESP32 da khoi dong thanh cong!");
  Serial.println("=================================");
}

void loop() {
  digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));

  Serial.print("Thoi gian hoat dong: ");
  Serial.print(millis() / 1000);
  Serial.println(" giay");

  delay(1000);
}
