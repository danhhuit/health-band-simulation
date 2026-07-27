# Hợp đồng MQTT — Health Band 01

Đây là nguồn quy ước chung giữa ESP32/Wokwi, Node-RED và Dashboard. Không đổi topic hoặc JSON payload ở một nơi duy nhất.

## 1. Cấu hình chung

```text
Broker: broker.emqx.io
Port: 1883
Topic prefix: iot31/nhom-thanh-danh/health-band
Device ID: health-band-01
```

Broker này là broker **public chỉ dành cho demo**. Không gửi dữ liệu y tế thật, thông tin cá nhân, mật khẩu hoặc token.

## 2. Bảng topic chính thức

| Topic | Publisher | Subscriber | QoS | Retain | Khi nào dùng |
|---|---|---|---:|---|---|
| `.../telemetry` | ESP32 | Node-RED | 0 | Không | Mỗi 2 giây. |
| `.../status` | ESP32/LWT | Node-RED | 1 | Có | Online/offline. |
| `.../command` | Dashboard → Node-RED | ESP32 | 1 | Không | Đổi mode/reset steps. |
| `.../event` | ESP32 | Node-RED/Dashboard | 1 | Không | Xác nhận lệnh, FALL/SOS. |
| `.../alert` | Node-RED | Dashboard | 1 | Không | Cảnh báo rule. |

Trong bảng, `...` nghĩa là `iot31/nhom-thanh-danh/health-band`.

## 3. Telemetry

Topic:

```text
iot31/nhom-thanh-danh/health-band/telemetry
```

```json
{
  "deviceId": "health-band-01",
  "timestamp": 15000,
  "seq": 25,
  "heartRate": 78,
  "spo2": 98,
  "steps": 125,
  "fallDetected": false,
  "battery": 92,
  "signalQuality": "good",
  "mode": "normal"
}
```

| Trường | Kiểu | Ý nghĩa |
|---|---|---|
| `timestamp` | integer | Mili giây từ lúc ESP32 khởi động (`millis()`). |
| `seq` | integer | Tăng dần theo bản tin. |
| `heartRate` | integer | BPM mô phỏng. |
| `spo2` | integer | Phần trăm SpO₂ mô phỏng. |
| `steps` | integer | Số bước mô phỏng. |
| `fallDetected` | boolean | `true` khi fall. |
| `battery` | integer | 0–100 %. |
| `signalQuality` | string | `good`, `medium`, hoặc `poor`. |
| `mode` | string | Mode hiện tại. |

Schema: [../data/telemetry.schema.json](../data/telemetry.schema.json).

## 4. Status

Topic:

```text
iot31/nhom-thanh-danh/health-band/status
```

```json
{
  "deviceId": "health-band-01",
  "online": true,
  "uptime": 15000,
  "firmwareVersion": "0.2.0",
  "activeMode": "normal"
}
```

`status` dùng QoS 1 và retained. Firmware cấu hình MQTT Last Will với `online: false` để broker publish offline khi ESP32 mất kết nối bất ngờ.

## 5. Command

Topic:

```text
iot31/nhom-thanh-danh/health-band/command
```

### Đổi mode

```json
{
  "requestId": "dashboard-123456",
  "command": "setMode",
  "value": "high_hr"
}
```

`value` hợp lệ: `normal`, `high_hr`, `low_hr`, `low_spo2`, `fall`, `low_battery`.

### Reset steps

```json
{
  "requestId": "dashboard-123457",
  "command": "resetSteps"
}
```

Firmware phản hồi qua `event`. Command không hợp lệ phải tạo `COMMAND_REJECTED` và không làm dừng telemetry.

Schema: [../data/command.schema.json](../data/command.schema.json).

## 6. Event

Topic:

```text
iot31/nhom-thanh-danh/health-band/event
```

```json
{
  "deviceId": "health-band-01",
  "eventType": "COMMAND_ACCEPTED",
  "requestId": "dashboard-123456",
  "command": "setMode",
  "value": "high_hr",
  "activeMode": "high_hr",
  "timestamp": 18000,
  "message": "Scenario changed"
}
```

Event giúp Dashboard chứng minh thiết bị đã thật sự nhận lệnh, thay vì chỉ đổi giao diện ở phía web.

## 7. Alert

Topic:

```text
iot31/nhom-thanh-danh/health-band/alert
```

Node-RED publish alert khi điều kiện demo đúng:

| Loại | Điều kiện hiện tại |
|---|---|
| `HIGH_HEART_RATE` | `heartRate > 120` |
| `LOW_HEART_RATE` | `heartRate < 50` |
| `LOW_SPO2` | `spo2 < 94` |
| `FALL_DETECTED` | `fallDetected === true` |
| `LOW_BATTERY` | `battery <= 20` |
| `DEVICE_OFFLINE` | Không có telemetry quá 8 giây |

> Đây là ngưỡng demo. Các ngưỡng không phải tiêu chuẩn chẩn đoán y tế.

## 8. Kiểm tra nhanh bằng Dashboard

1. Chạy Docker và Wokwi theo [../DEPLOYMENT.md](../DEPLOYMENT.md).
2. Mở Dashboard, chọn `High HR`.
3. Kiểm tra `Health alerts` có cảnh báo nhịp tim cao.
4. Mở Digital twin để xem acknowledgement và vòng tay đổi trạng thái.
5. Chọn `Normal` để kết thúc tình huống.

## 9. Khi thay đổi hợp đồng

Mọi thay đổi topic/payload phải cập nhật và kiểm tra đồng thời:

- firmware `src/main.cpp`;
- `data/*.schema.json` và JSON mẫu;
- Node-RED flow;
- Dashboard template;
- tài liệu và test case.
