# ĐẶC TẢ DỮ LIỆU VÀ SCHEMA TELEMETRY (HEALTH DATA SPECIFICATION)

**Nhiệm vụ:** V02  
**Thành viên thực hiện:** Hồng Vỹ  
**Nhánh Git:** `feature/vy-health-data`  
**Review chéo:** Minh Thiện  

---

## 1. MỤC TIÊU
Tài liệu này định nghĩa chuẩn cấu trúc dữ liệu (`HealthData`) và payload JSON dùng chung giữa các tầng trong hệ thống Vòng tay sức khỏe IoT 4 tầng (Đồ án 31):
- **Tầng 1 (Wokwi ESP32 - Minh Thiện):** Đóng gói struct theo đúng định dạng này và publish qua MQTT.
- **Tầng 2 (Wi-Fi/MQTT Broker - Thành Danh):** Truyền tải payload trên topic `iot31/nhom-thanh-danh/health-band/telemetry`.
- **Tầng 3 & 4 (Node-RED/Dashboard - Lê Hậu):** Phân tích (parse JSON), kiểm tra schema validation, lưu DB và hiển thị trên Dashboard/phát cảnh báo.

---

## 2. BẢNG MÔ TẢ TRƯỜNG DỮ LIỆU (DATA FIELDS & VALIDATION BOUNDARIES)

Tất cả 9 trường dưới đây đều là **BẮT BUỘC (Required: Có)** trong mỗi gói tin telemetry:

| Trường (`Field`) | Kiểu (`Type`) | Đơn vị / Phạm vi hợp lệ (`Range`) | Bắt buộc | Ý nghĩa và Quy ước kiểm tra |
| :--- | :--- | :--- | :---: | :--- |
| **`deviceId`** | Chuỗi (`String`) | `health-band-01` | Có | Mã thiết bị ảo ESP32 trên Wokwi. Không được để trống. |
| **`timestamp`** | Số nguyên (`Integer`) | `≥ 0` (ms) | Có | Thời gian tính bằng mili giây kể từ lúc ESP32 khởi động (`millis()`). |
| **`seq`** | Số nguyên (`Integer`) | `≥ 0` | Có | Số thứ tự bản tin (Sequence Number), tăng đều +1 mỗi bản tin gửi đi để phát hiện mất hay lặp gói tin. |
| **`heartRate`** | Số nguyên (`Integer`) | `40 - 200` (BPM) | Có | Nhịp tim mô phỏng. Ngưỡng cảnh báo demo: cao khi `> 120`, thấp khi `< 50` (yêu cầu 3 bản tin liên tiếp). |
| **`spo2`** | Số nguyên (`Integer`) | `70 - 100` (%) | Có | Nồng độ oxy trong máu mô phỏng. Ngưỡng cảnh báo demo: thấp khi `< 94%` (yêu cầu 3 bản tin liên tiếp). |
| **`steps`** | Số nguyên (`Integer`) | `≥ 0` | Có | Tổng số bước chân tích lũy (tăng khi nhấn nút `STEP`). |
| **`fallDetected`** | `Boolean` | `true` / `false` | Có | Cờ trạng thái té ngã (khi nhấn nút `FALL`). **Cảnh báo NGAY LẬP TỨC nếu bằng `true`**. |
| **`battery`** | Số nguyên (`Integer`) | `0 - 100` (%) | Có | Mức pin mô phỏng. Ngưỡng cảnh báo pin yếu: `<= 20%`. |
| **`signalQuality`** | Chuỗi (`String`) | `"good"`, `"medium"`, `"poor"` | Có | Chất lượng tín hiệu cảm biến mô phỏng. |

---

## 3. CÁC QUY TẮC BỘ SINH DỮ LIỆU (DATA GENERATOR RULES)
Bộ sinh dữ liệu (được cài đặt trong `HealthDataGenerator.h` trên ESP32 Wokwi) phải tuân thủ nghiêm ngặt:
1. **Không có giá trị `NaN`, `Null` hoặc sai kiểu:** Mọi số nguyên phải là `int`, chuỗi không chứa ký tự đặc biệt gây lỗi JSON parse.
2. **Dao động tự nhiên (Random Walk / Markov chain nhẹ):** Nhịp tim và SpO2 khi ở chế độ bình thường chỉ thay đổi `±1` hoặc `±2` đơn vị sau mỗi chu kỳ 1-2 giây để tránh hiện tượng số nhảy giật cục bất hợp lý.
3. **Giới hạn biên (Clamping):** Nếu dao động vượt qua `200` hoặc dưới `40` đối với `heartRate`, hệ thống tự động gán về biên cực đại/cực tiểu.
4. **Tăng tự động số thứ tự (`seq`):** Luôn đảm bảo `seq` bản tin sau bằng `seq` bản tin trước + 1.
5. **Mô phỏng sụt pin:** Pin tự động giảm 1% sau mỗi `N` bản tin (ví dụ: mỗi 30 bản tin giảm 1%) hoặc có chế độ kích hoạt pin yếu nhanh để kiểm thử ca lỗi.

---

## 4. SCHEMA JSON VÀ FILE MẪU
- **Schema Validation:** Xem chi tiết tại [schema.json](file:///c:/IOT/health-band-simulation/data/schema.json).
- **Dữ liệu kiểm thử mẫu (8 Test Cases):** Xem chi tiết tại [sample-telemetry.json](file:///c:/IOT/health-band-simulation/data/sample-telemetry.json).
