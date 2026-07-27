# Kiến trúc IoT 4 tầng — Health Band 01

## 1. Mục tiêu kiến trúc

Dự án chọn kiến trúc IoT **4 tầng** vì dễ giải thích, phù hợp quy mô đồ án và thể hiện rõ cách dữ liệu đi từ thiết bị tới người dùng.

```text
┌─────────────────────────────────────────────────────────┐
│ Tầng 4 — Application                                     │
│ Dashboard · Digital Twin · Presenter Mode · App Guide     │
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
- OLED I²C: hiển thị HR, SpO₂, bước, pin và mode.
- RGB LED: xanh lá = normal, vàng = cảnh báo, đỏ = fall, xanh dương = mất MQTT.
- Buzzer: âm ngắn khi đổi mode, âm mạnh hơn khi fall.
- Nút FALL/SOS: chuyển thiết bị sang mode `fall` và gửi event cục bộ.

### Dữ liệu tạo ra

Firmware tạo dữ liệu theo mode và publish mỗi **2 giây**:

| Mode | HR | SpO₂ | Ý nghĩa |
|---|---:|---:|---|
| `normal` | 70–90 | 97–99 | Bình thường. |
| `high_hr` | 125–145 | 95–97 | Nhịp tim cao. |
| `low_hr` | 40–48 | 96–98 | Nhịp tim thấp. |
| `low_spo2` | 80–95 | 85–92 | Oxy máu thấp. |
| `fall` | 100–130 | 93–95 | Té ngã. |
| `low_battery` | 75–85 | 97–98 | Pin yếu. |

### Trách nhiệm

- Sinh telemetry có `timestamp = millis()` và `seq` tăng dần.
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
4. Đánh giá ngưỡng demo: HR cao/thấp, SpO₂ thấp, fall và pin yếu.
5. Publish `alert` và chuyển dữ liệu sang Dashboard.
6. Nhận lệnh từ Dashboard rồi publish sang topic `command`.

> Hiện rule cảnh báo theo ngưỡng trực tiếp. Bộ đếm “3 mẫu liên tiếp”, xác thực schema đầy đủ, chống trùng `seq` và TLS là hướng cải tiến, chưa phải tính năng của MVP.

## 5. Tầng 4 — Application

Dashboard tại <http://localhost:1880/dashboard/overview> gồm 5 trang:

| Trang | Vai trò |
|---|---|
| Overview | Theo dõi dữ liệu, alert, timeline và gửi scenario command. |
| Digital twin | Minh họa chiếc vòng tay số và các đầu ra phần cứng. |
| IoT architecture | Dạy kiến trúc 4 tầng, có `Play data journey`. |
| Presenter mode | Dẫn dắt demo bằng kịch bản/câu hỏi/kết quả. |
| App guide | Tài liệu sử dụng tích hợp trong Dashboard. |

Người dùng có thể chọn 🇻🇳 Tiếng Việt hoặc 🇺🇸 English; lựa chọn được lưu trong trình duyệt.

## 6. Chu trình dữ liệu end-to-end

Ví dụ người xem chọn `High HR`:

1. Dashboard tạo `{ requestId, command: "setMode", value: "high_hr" }`.
2. Node-RED publish payload sang topic `command`.
3. ESP32 nhận lệnh, đổi mode, đổi OLED/RGB/buzzer và gửi `COMMAND_ACCEPTED` vào topic `event`.
4. ESP32 publish telemetry HR 125–145 BPM.
5. Node-RED tạo alert `HIGH_HEART_RATE` khi HR > 120.
6. Dashboard cập nhật Overview, Digital Twin, Health alerts và Event timeline.

Chu trình trên là bằng chứng quan trọng rằng dự án có luồng IoT hai chiều, không chỉ hiển thị dữ liệu tĩnh.
