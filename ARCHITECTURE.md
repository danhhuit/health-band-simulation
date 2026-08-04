# Kiến trúc IoT 4 tầng — Health Band 01

## 1. Mục tiêu kiến trúc

Dự án chọn kiến trúc IoT **4 tầng** vì dễ giải thích, phù hợp quy mô đồ án và thể hiện rõ cách dữ liệu đi từ thiết bị tới người dùng.

```text
┌─────────────────────────────────────────────────────────┐
│ Tầng 4 — Application                                     │
│ Dashboard · Digital Twin · Health Profile · Smart Coach   │
└──────────────────────────▲──────────────────────────────┘
                           │ telemetry / alert / event
┌──────────────────────────┴──────────────────────────────┐
│ Tầng 3 — Processing                                      │
│ Node-RED parse JSON · rules · offline · timeline          │
└──────────────────────────▲──────────────────────────────┘
                           │ MQTT
┌──────────────────────────┴──────────────────────────────┐
│ Tầng 2 — Network                                         │
│ Wi-Fi Wokwi-GUEST · broker.emqx.io:1883 · MQTT/JSON       │
└──────────────────────────▲──────────────────────────────┘
                           │ Wi-Fi
┌──────────────────────────┴──────────────────────────────┐
│ Tầng 1 — Device                                          │
│ ESP32 Wokwi · OLED · RGB LED · buzzer · FALL/SOS button   │
└─────────────────────────────────────────────────────────┘
```

## 2. Tầng 1 — Device / Perception

### Thành phần

- ESP32 DevKit mô phỏng trên Wokwi.
- MPU6050 I²C: đọc gia tốc ba trục để phát hiện chuyển động, bước và té ngã.
- HR sensor input trên GPIO 34: thanh trượt analog tương tác mô phỏng tín hiệu quang học.
- SpO₂ sensor input trên GPIO 35: thanh trượt analog tương tác mô phỏng tín hiệu quang học.
- DS18B20 trên GPIO 5: nhiệt độ cơ thể mô phỏng.
- LDR trên GPIO 33: ánh sáng môi trường và điều khiển độ tương phản OLED.
- BMP180 dùng chung I²C: áp suất, nhiệt độ môi trường và độ cao.
- GPS NMEA qua UART: tọa độ dùng trong workflow Fall/SOS.
- Haptic GPIO 23: đầu ra motor rung được biểu diễn bằng đèn báo trong Wokwi.
- OLED I²C: hiển thị HR, SpO₂, bước, pin và mode.
- RGB LED: xanh lá = normal, vàng = cảnh báo, đỏ = fall, xanh dương = mất MQTT.
- Buzzer: âm ngắn khi đổi mode, âm mạnh hơn khi fall.
- Nút FALL/SOS: chuyển thiết bị sang mode `fall` và gửi event cục bộ.

### Dữ liệu tạo ra

Ở mode `normal`, firmware đọc HR/SpO₂ từ đầu vào analog và tính bước/té ngã từ MPU6050. Khi chọn mode cảnh báo, dữ liệu cảm biến được ghi đè có chủ đích để tạo kịch bản lặp lại được. Telemetry được publish mỗi **2 giây** ở Live mode hoặc **8 giây** ở Eco mode:

| Mode | HR | SpO₂ | Ý nghĩa |
|---|---:|---:|---|
| `normal` | 70–90 | 97–99 | Bình thường. |
| `high_hr` | 125–145 | 95–97 | Nhịp tim cao. |
| `low_hr` | 40–48 | 96–98 | Nhịp tim thấp. |
| `low_spo2` | 80–95 | 85–92 | Oxy máu thấp. |
| `fall` | 100–130 | 93–95 | Té ngã. |
| `low_battery` | 75–85 | 97–98 | Pin yếu. |

### Trách nhiệm

- Đọc cảm biến → ánh xạ/hiệu chỉnh → tính chuyển động/bước/té ngã → sinh telemetry có `timestamp = millis()` và `seq` tăng dần.
- Publish `status` online retained sau khi kết nối MQTT.
- Subscribe `command`.
- Gửi `event` để xác nhận/thông báo việc thiết bị đã xử lý lệnh.
- Tự kết nối lại Wi-Fi/MQTT theo chu kỳ 5 giây.

## 3. Tầng 2 — Network / Transport

| Hạng mục | Giá trị |
|---|---|
| Wi-Fi Wokwi | `Wokwi-GUEST`, mật khẩu rỗng |
| Broker demo | `broker.emqx.io:1883` |
| Topic prefix | `iot31/nhom-thanh-danh/health-band` |
| Định dạng | JSON |
| Thiết bị | `health-band-01` |

Tầng này tách thiết bị với Node-RED/Dashboard. ESP32 không cần biết Dashboard trông như thế nào; Dashboard cũng không giao tiếp trực tiếp với ESP32.

Xem hợp đồng đầy đủ tại [node-red/mqtt-topics.md](node-red/mqtt-topics.md).

## 4. Tầng 3 — Processing / Platform

Node-RED chịu trách nhiệm:

1. Subscribe `telemetry`, `status`, `event`, `alert`.
2. Parse JSON; bỏ qua payload không phải object hoặc JSON sai cú pháp.
3. Ghi thời điểm telemetry cuối cùng để phát hiện offline sau hơn 8 giây.
4. Kiểm tra chất lượng packet: trường bắt buộc, range, sequence, timestamp và signal quality.
5. Đánh giá ngưỡng theo profile; HR/SpO₂ phải bất thường 3 mẫu liên tiếp mới publish alert.
6. Publish `alert` và chuyển dữ liệu sang Dashboard.
7. Nhận lệnh từ Dashboard rồi publish sang topic `command`.

> Các ngưỡng chỉ phục vụ demo môn học, không phải tiêu chuẩn chẩn đoán. Broker public hiện chưa dùng TLS/xác thực.

## 5. Tầng 4 — Application

Dashboard tại <http://localhost:1880/dashboard/overview> gồm 6 trang:

| Trang | Vai trò |
|---|---|
| Overview | Theo dõi dữ liệu, cảm biến mở rộng, alert và gửi scenario command. |
| Digital twin | Minh họa chiếc vòng tay số và các đầu ra phần cứng. |
| Smart Coach | F1–F12: điểm giải thích được, mục tiêu riêng, xu hướng, cảnh báo bền vững, profile, chất lượng dữ liệu, Eco mode, gợi ý và báo cáo. |
| Health profile | Lưu thông tin cơ thể trên trình duyệt, tính BMI sàng lọc và gợi ý mục tiêu ngủ theo tuổi. |
| App guide | Tài liệu sử dụng tích hợp trong Dashboard. |
| Activity log | Lưu báo cáo, điểm phục hồi và dòng sự kiện đầu cuối. |

Người dùng có thể chọn 🇻🇳 Tiếng Việt hoặc 🇺🇸 English; lựa chọn được lưu trong trình duyệt.

## 6. Chu trình dữ liệu end-to-end

Ví dụ người xem chọn `High HR`:

1. Dashboard tạo `{ requestId, command: "setMode", value: "high_hr" }`.
2. Node-RED publish payload sang topic `command`.
3. ESP32 nhận lệnh, đổi mode, đổi OLED/RGB/buzzer và gửi `COMMAND_ACCEPTED` vào topic `event`.
4. ESP32 publish telemetry HR 125–145 BPM.
5. Node-RED tăng bộ đếm High HR; sau 3 mẫu liên tiếp mới tạo alert `HIGH_HEART_RATE`.
6. Dashboard cập nhật Overview, Digital Twin, Health alerts và Event timeline.

Chu trình trên là bằng chứng quan trọng rằng dự án có luồng IoT hai chiều, không chỉ hiển thị dữ liệu tĩnh.

## 7. Trạng thái đeo và giấc ngủ

```text
HR/SpO₂ raw + DS18B20 + MPU6050 + thời gian NTP
        ↓
Nhận biết đang đeo
        ├─ Không đeo → vitalDataValid=false, sleep=not_tracked
        └─ Đang đeo → sensor fusion → awake/candidate/light/deep
```

Demo dùng ngưỡng tăng tốc 15/30/60 giây. Đây là mô hình giải thích thuật toán, không phải phân loại giấc ngủ lâm sàng.

## 8. Sao lưu và phục hồi

- Device: `Preferences` lưu NVS mỗi 30 giây và sau lệnh.
- Processing: Node-RED dùng `localfilesystem`; `/data` được mount ra `node-red/data`.
- Application: `localStorage` giữ snapshot hiện tại và snapshot trước; hỗ trợ Export/Restore JSON.
- Sau mất nguồn, ESP32 gửi `DEVICE_RECOVERED` và telemetry có `recovery.stateRestored`.

## 9. Gemini

Dashboard gửi aggregate ẩn danh đến API nội bộ Node-RED. Node-RED mới gọi Gemini bằng key trong biến môi trường. Nếu API lỗi, Smart Coach cục bộ tiếp tục hoạt động.
