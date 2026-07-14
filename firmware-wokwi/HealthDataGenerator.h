#ifndef HEALTH_DATA_GENERATOR_H
#define HEALTH_DATA_GENERATOR_H

#include <Arduino.h>

/**
 * @brief Định nghĩa cấu trúc dữ liệu Telemetry chuẩn cho Vòng tay sức khỏe (Đồ án 31)
 * Nhánh thực hiện: Hồng Vỹ (feature/vy-health-data)
 */
struct HealthData {
  String deviceId;       // Mã thiết bị ảo (VD: "health-band-01")
  unsigned long timestamp; // millis() từ lúc khởi động
  unsigned long seq;     // Số thứ tự bản tin (Sequence Number)
  int heartRate;         // Nhịp tim (40 - 200 BPM)
  int spo2;              // Nồng độ oxy trong máu (70 - 100 %)
  unsigned long steps;   // Tổng số bước chân
  bool fallDetected;     // Cờ phát hiện té ngã
  int battery;           // Dung lượng pin (0 - 100 %)
  String signalQuality;  // Chất lượng tín hiệu ("good", "medium", "poor")
};

/**
 * @brief Lớp bộ sinh dữ liệu mô phỏng cho ESP32 trên Wokwi
 */
class HealthDataGenerator {
private:
  String _deviceId;
  unsigned long _seq;
  int _currentHeartRate;
  int _currentSpO2;
  unsigned long _steps;
  int _battery;
  unsigned long _lastBatteryDepletionTime;

  // Giới hạn hợp lệ theo Spec V02
  const int MIN_HR = 40;
  const int MAX_HR = 200;
  const int MIN_SPO2 = 70;
  const int MAX_SPO2 = 100;

  enum SimulationMode {
    MODE_NORMAL = 0,
    MODE_HIGH_HR = 1,
    MODE_LOW_SPO2 = 2,
    MODE_LOW_BATTERY = 3,
    MODE_CRITICAL = 4
  };

  SimulationMode _mode;

public:
  HealthDataGenerator(const String& deviceId = "health-band-01") {
    _deviceId = deviceId;
    _seq = 0;
    _currentHeartRate = 78; // Nhịp tim nghỉ ngơi bình thường
    _currentSpO2 = 98;      // SpO2 bình thường
    _steps = 0;
    _battery = 100;
    _lastBatteryDepletionTime = 0;
    _mode = MODE_NORMAL;
  }

  void setSimulationMode(int mode) {
    _mode = (SimulationMode)mode;
  }

  void incrementSteps(unsigned long count = 1) {
    _steps += count;
  }

  void triggerStepButton() {
    _steps += 1;
  }

  void setBattery(int level) {
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    _battery = level;
  }

  /**
   * @brief Sinh ra 1 gói dữ liệu HealthData mới, tự động điều chỉnh dao động hợp lệ
   * @param fallPressed Trạng thái nút nhấn té ngã (true nếu đang nhấn hoặc vừa phát hiện ngã)
   */
  HealthData generate(bool fallPressed = false) {
    HealthData data;
    data.deviceId = _deviceId;
    data.timestamp = millis();
    _seq++;
    data.seq = _seq;
    data.steps = _steps;
    data.fallDetected = fallPressed;

    // Cập nhật pin (giảm 1% sau mỗi 60 giây mô phỏng)
    if (millis() - _lastBatteryDepletionTime > 60000 && _battery > 0) {
      _battery--;
      _lastBatteryDepletionTime = millis();
    }

    // Xử lý chế độ mô phỏng (Simulation Mode)
    switch (_mode) {
      case MODE_HIGH_HR:
        _currentHeartRate = random(125, 155); // Nhịp tim cao > 120
        _currentSpO2 = random(95, 99);
        break;

      case MODE_LOW_SPO2:
        _currentHeartRate = random(80, 105);
        _currentSpO2 = random(85, 92);        // SpO2 thấp < 94%
        break;

      case MODE_LOW_BATTERY:
        _battery = random(10, 18);            // Pin yếu <= 20%
        _currentHeartRate = random(70, 85);
        _currentSpO2 = random(96, 99);
        break;

      case MODE_CRITICAL:
        _currentHeartRate = random(140, 170); // Nhịp tim cực cao
        _currentSpO2 = random(80, 88);        // SpO2 nguy cấp
        _battery = random(5, 15);             // Pin rất yếu
        break;

      case MODE_NORMAL:
      default:
        // Dao động ngẫu nhiên nhẹ theo bước đi bộ ngẫu nhiên (Random Walk [-2, +2])
        _currentHeartRate += random(-2, 3);
        if (_currentHeartRate < 65) _currentHeartRate = 65;
        if (_currentHeartRate > 95) _currentHeartRate = 95;

        // SpO2 dao động nhẹ trong khoảng bình thường [96, 100]
        if (random(0, 10) > 6) {
          _currentSpO2 += random(-1, 2);
          if (_currentSpO2 < 96) _currentSpO2 = 96;
          if (_currentSpO2 > 100) _currentSpO2 = 100;
        }
        break;
    }

    // Đảm bảo không bao giờ vượt biên tuyệt đối [MIN, MAX]
    if (_currentHeartRate < MIN_HR) _currentHeartRate = MIN_HR;
    if (_currentHeartRate > MAX_HR) _currentHeartRate = MAX_HR;
    if (_currentSpO2 < MIN_SPO2) _currentSpO2 = MIN_SPO2;
    if (_currentSpO2 > MAX_SPO2) _currentSpO2 = MAX_SPO2;

    data.heartRate = _currentHeartRate;
    data.spo2 = _currentSpO2;
    data.battery = _battery;

    // Đánh giá chất lượng tín hiệu mô phỏng
    if (_mode == MODE_CRITICAL || fallPressed) {
      data.signalQuality = "poor";
    } else if (_mode != MODE_NORMAL || _battery < 15) {
      data.signalQuality = "medium";
    } else {
      data.signalQuality = "good";
    }

    return data;
  }

  /**
   * @brief Chuyển đổi struct HealthData thành chuỗi JSON hợp lệ để publish qua MQTT
   */
  static String toJSON(const HealthData& data) {
    String json = "{";
    json += "\"deviceId\":\"" + data.deviceId + "\",";
    json += "\"timestamp\":" + String(data.timestamp) + ",";
    json += "\"seq\":" + String(data.seq) + ",";
    json += "\"heartRate\":" + String(data.heartRate) + ",";
    json += "\"spo2\":" + String(data.spo2) + ",";
    json += "\"steps\":" + String(data.steps) + ",";
    json += "\"fallDetected\":" + String(data.fallDetected ? "true" : "false") + ",";
    json += "\"battery\":" + String(data.battery) + ",";
    json += "\"signalQuality\":\"" + data.signalQuality + "\"";
    json += "}";
    return json;
  }
};

#endif // HEALTH_DATA_GENERATOR_H
