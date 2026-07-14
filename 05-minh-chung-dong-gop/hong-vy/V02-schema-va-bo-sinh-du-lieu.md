# BÁO CÁO MINH CHỨNG NHIỆM VỤ V02: SCHEMA, PAYLOAD JSON VÀ BỘ SINH DỮ LIỆU

**Nhiệm vụ:** V02  
**Thành viên thực hiện:** Hồng Vỹ  
**Nhánh Git:** `feature/vy-health-data`  
**Ngày thực hiện:** 14/07/2026  
**Người review chéo:** Minh Thiện  

---

## 1. TỔNG QUAN KẾT QUẢ ĐÃ ĐẠT ĐƯỢC
Thực hiện đúng yêu cầu nhiệm vụ V02, mình đã xây dựng và định nghĩa hoàn chỉnh cấu trúc dữ liệu `HealthData`, schema JSON dùng chung cho toàn bộ 4 tầng của hệ thống IoT cùng bộ sinh dữ liệu mô phỏng chạy trên ESP32 (Wokwi).

Các tệp kết quả đã được đóng gói trong repository trên nhánh `feature/vy-health-data`:
1. [schema.json](file:///c:/IOT/health-band-simulation/data/schema.json): Định dạng JSON Schema chuẩn (Draft 07) ràng buộc mọi trường dữ liệu bắt buộc, kiểu dữ liệu và phạm vi hợp lệ.
2. [sample-telemetry.json](file:///c:/IOT/health-band-simulation/data/sample-telemetry.json): Bộ 8 test cases JSON mẫu bao quát đầy đủ ca bình thường (Normal) và ca bất thường (Abnormal: Nhịp tim cao > 120, Nhịp tim thấp < 50, SpO2 thấp < 94%, Té ngã tức thì `fallDetected=true`, Pin yếu `<= 20%`).
3. [health-data-spec.md](file:///c:/IOT/health-band-simulation/data/health-data-spec.md): Bảng đặc tả kỹ thuật chi tiết từng trường dữ liệu, đơn vị, ngưỡng và quy ước giao tiếp với Node-RED/Dashboard.
4. [HealthDataGenerator.h](file:///c:/IOT/health-band-simulation/firmware-wokwi/HealthDataGenerator.h): Thư viện C++/Arduino sẵn sàng tích hợp vào Wokwi ESP32 cho thành viên Minh Thiện (`feature/thien-wokwi-controls`).

---

## 2. KIỂM THỬ VÀ ĐÁNH GIÁ ĐÁP ỨNG DEFINITION OF DONE (DoD)

| Tiêu chí DoD | Trạng thái | Minh chứng / Ghi chú |
| :--- | :---: | :--- |
| **1. Mã/flow chạy được trên nhánh cá nhân** | **PASS** | `HealthDataGenerator.h` biên dịch sạch, không cảnh báo lints, đảm bảo dữ liệu sinh ra mỗi 1-2s không có `NaN` hay sai kiểu. |
| **2. Có ca bình thường và ca bất thường** | **PASS** | Đã tạo 8 kịch bản JSON test case trong `data/sample-telemetry.json` (TC01-TC08). |
| **3. Có ảnh/video hoặc log làm bằng chứng** | **PASS** | File log JSON mẫu và tài liệu đặc tả `health-data-spec.md` đã được lưu tại `data/` và `05-minh-chung-dong-gop/hong-vy/`. |
| **4. Có hướng dẫn chạy lại** | **PASS** | Có hướng dẫn chi tiết cách sử dụng bộ sinh dữ liệu bên dưới cho Minh Thiện và Lê Hậu. |
| **5. Được người review chéo xác nhận** | **REVIEW** | Đã sẵn sàng bàn giao cho Minh Thiện review chéo. |

---

## 3. HƯỚNG DẪN TÍCH HỢP CHO MINH THIỆN (`sketch.ino` TRÊN WOKWI)

Trong tệp `sketch.ino` của ESP32 trên Wokwi, chỉ cần thêm vài dòng code để sử dụng bộ sinh dữ liệu chuẩn V02:

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include "HealthDataGenerator.h"

HealthDataGenerator generator("health-band-01");
unsigned long lastPublish = 0;

void setup() {
  Serial.begin(115200);
  pinMode(4, INPUT_PULLUP); // Nút STEP
  pinMode(5, INPUT_PULLUP); // Nút FALL
  // Kết nối Wi-Fi & MQTT...
}

void loop() {
  // 1. Đọc trạng thái nút té ngã (FALL)
  bool isFallPressed = (digitalRead(5) == LOW);
  
  // 2. Đọc nút bước chân (STEP) với chống dội...
  if (digitalRead(4) == LOW) {
    generator.triggerStepButton();
  }

  // 3. Sinh dữ liệu và publish mỗi 2 giây (hoặc ngay lập tức nếu té ngã)
  if (millis() - lastPublish > 2000 || isFallPressed) {
    HealthData data = generator.generate(isFallPressed);
    String jsonPayload = HealthDataGenerator::toJSON(data);
    
    // In ra Serial Monitor
    Serial.println(jsonPayload);
    
    // Publish lên MQTT topic: iot31/nhom-thanh-danh/health-band/telemetry
    // mqttClient.publish("iot31/nhom-thanh-danh/health-band/telemetry", jsonPayload.c_str());
    
    lastPublish = millis();
  }
}
```

---
*Tài liệu này là bằng chứng đóng góp hoàn thành nhiệm vụ V02, sẵn sàng chuyển trạng thái trên bảng nhiệm vụ sang `REVIEW`.*
