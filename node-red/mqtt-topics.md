# Hợp đồng MQTT - Đồ án 31

Tài liệu này là nguồn quy ước chung cho ESP32, bộ sinh dữ liệu, Node-RED và dashboard. Mọi thay đổi tên topic hoặc cấu trúc payload phải được cả nhóm thống nhất trước khi merge.

## 1. Topic prefix

```text
iot31/nhom-thanh-danh/health-band
```

Quy tắc đặt tên:

- Viết thường, không dấu và không có khoảng trắng.
- Không đặt dấu `/` ở cuối prefix.
- MQTT topic không chứa mật khẩu, token hoặc thông tin cá nhân.
- MVP chỉ có một thiết bị nên `deviceId` nằm trong payload, chưa cần nằm trong topic.
- Topic `iot31/nhom-thanh-danh/health-band/#` chỉ dùng để subscribe khi kiểm thử, không dùng để publish.

## 2. Bảng topic chính thức

| Topic đầy đủ | Publisher | Subscriber | QoS | Retain | Thời điểm |
|---|---|---|---:|---|---|
| `iot31/nhom-thanh-danh/health-band/telemetry` | ESP32 | Node-RED | 0 | Không | Mỗi 1-2 giây |
| `iot31/nhom-thanh-danh/health-band/status` | ESP32 hoặc MQTT Last Will | Node-RED | 1 | Có | Khi kết nối/mất kết nối |
| `iot31/nhom-thanh-danh/health-band/command` | Node-RED | ESP32 | 1 | Không | Khi người dùng ra lệnh |
| `iot31/nhom-thanh-danh/health-band/alert` | Node-RED | Dashboard hoặc module ghi log | 1 | Không | Khi luật cảnh báo được kích hoạt |

## 3. Quyết định QoS và retain

### Telemetry

- `QoS 0`: telemetry xuất hiện liên tục nên mất một bản tin không ảnh hưởng lớn; bản tin mới sẽ đến sau 1-2 giây.
- `retain=false`: subscriber mới không được nhận lại một mẫu sức khỏe đã cũ như thể đó là dữ liệu hiện tại.

### Status

- `QoS 1`: trạng thái online/offline cần được nhận ít nhất một lần.
- `retain=true`: Node-RED vừa kết nối vẫn đọc được trạng thái cuối của thiết bị.
- MQTT Last Will and Testament (LWT) dùng cùng topic để broker tự phát `online=false` khi thiết bị mất kết nối đột ngột.

### Command

- `QoS 1`: lệnh điều khiển cần được thiết bị nhận ít nhất một lần.
- `retain=false`: tránh ESP32 thực hiện lại một lệnh cũ sau khi reconnect.
- ESP32 phải kiểm tra `requestId` nếu cần chống xử lý lặp.

### Alert

- `QoS 1`: cảnh báo cần đến dashboard hoặc module ghi log.
- `retain=false`: cảnh báo cũ không được hiển thị như sự kiện vừa xảy ra.

## 4. Topic telemetry

```text
iot31/nhom-thanh-danh/health-band/telemetry
```

Payload mẫu: [`../data/sample-telemetry.json`](../data/sample-telemetry.json)

Schema: [`../data/telemetry.schema.json`](../data/telemetry.schema.json)

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
  "signalQuality": "good"
}
```

Quy tắc:

- `timestamp`: số mili giây kể từ khi ESP32 khởi động, lấy từ `millis()`.
- `seq`: số nguyên tăng một đơn vị sau mỗi lần publish.
- Không gửi `NaN`, số âm, thiếu trường hoặc chuỗi thay cho số.
- Node-RED bổ sung thời gian nhận trên máy chủ nếu cần lưu lịch sử theo giờ thực.

## 5. Topic status

```text
iot31/nhom-thanh-danh/health-band/status
```

Payload online: [`../data/sample-status-online.json`](../data/sample-status-online.json)

```json
{
  "deviceId": "health-band-01",
  "online": true,
  "uptime": 15000,
  "firmwareVersion": "0.1.0"
}
```

Payload offline dùng cho LWT: [`../data/sample-status-offline.json`](../data/sample-status-offline.json)

```json
{
  "deviceId": "health-band-01",
  "online": false,
  "uptime": 0,
  "firmwareVersion": "0.1.0"
}
```

Schema: [`../data/status.schema.json`](../data/status.schema.json)

Trình tự kết nối:

1. ESP32 cấu hình LWT với payload offline, QoS 1 và retain true.
2. ESP32 kết nối broker.
3. ESP32 publish payload online, QoS 1 và retain true.
4. Khi mất kết nối đột ngột, broker publish payload offline đã cấu hình.
5. Node-RED vẫn có kiểm tra phụ: quá 5 giây không nhận telemetry thì đánh dấu thiết bị offline.

## 6. Topic command

```text
iot31/nhom-thanh-danh/health-band/command
```

Schema: [`../data/command.schema.json`](../data/command.schema.json)

### Chuyển chế độ mô phỏng

Payload mẫu: [`../data/sample-command-set-mode.json`](../data/sample-command-set-mode.json)

```json
{
  "requestId": "cmd-001",
  "command": "setMode",
  "value": "high_hr"
}
```

Giá trị `value` được phép:

- `normal`
- `high_hr`
- `low_hr`
- `low_spo2`
- `fall`
- `low_battery`

### Đặt lại số bước

Payload mẫu: [`../data/sample-command-reset-steps.json`](../data/sample-command-reset-steps.json)

```json
{
  "requestId": "cmd-002",
  "command": "resetSteps"
}
```

Quy tắc xử lý:

- `requestId` là chuỗi không rỗng để nhận diện lệnh.
- `setMode` bắt buộc có `value` hợp lệ.
- `resetSteps` không có `value` và đặt bộ đếm bước về 0.
- ESP32 bỏ qua lệnh sai và ghi `Unknown or invalid command` lên Serial Monitor.
- Lệnh không hợp lệ không được làm ESP32 treo hoặc ngừng publish telemetry.

## 7. Topic alert

```text
iot31/nhom-thanh-danh/health-band/alert
```

Publisher là Node-RED sau khi kiểm tra telemetry. ESP32 không publish topic này.

Payload mẫu: [`../data/sample-alert.json`](../data/sample-alert.json)

```json
{
  "deviceId": "health-band-01",
  "type": "HIGH_HEART_RATE",
  "severity": "warning",
  "value": 128,
  "threshold": 120,
  "sourceSeq": 25,
  "timestamp": 15000,
  "message": "Nhịp tim vượt ngưỡng demo"
}
```

Schema: [`../data/alert.schema.json`](../data/alert.schema.json)

Loại cảnh báo được phép:

- `HIGH_HEART_RATE`
- `LOW_HEART_RATE`
- `LOW_SPO2`
- `FALL_DETECTED`
- `LOW_BATTERY`
- `DEVICE_OFFLINE`
- `INVALID_PAYLOAD`

Mức độ được phép:

- `info`
- `warning`
- `critical`

Ngưỡng demo:

| Loại | Điều kiện |
|---|---|
| Nhịp tim cao | `heartRate > 120` |
| Nhịp tim thấp | `heartRate < 50` |
| SpO2 thấp | `spo2 < 94` |
| Té ngã | `fallDetected == true` |
| Pin yếu | `battery <= 20` |
| Offline | Không có telemetry quá 5 giây |

Các ngưỡng chỉ phục vụ minh họa trong đồ án, không phải tiêu chuẩn chẩn đoán y tế.

## 8. Ánh xạ Node-RED

Flow tối thiểu:

```text
MQTT In (telemetry)
  -> JSON
  -> Validate
  -> Debug
```

Flow đầy đủ:

```text
MQTT In (telemetry)
  -> JSON
  -> Validate
     |-> Dữ liệu hợp lệ -> Dashboard -> Lưu lịch sử -> Luật cảnh báo
     |-> Dữ liệu lỗi    -> INVALID_PAYLOAD -> Debug lỗi

MQTT In (status) -> Trạng thái online/offline
MQTT Out (command) -> ESP32
MQTT Out (alert) -> Dashboard/log
```

## 9. Ca kiểm thử hợp đồng MQTT

### MQTT-01: telemetry hợp lệ

- Publish `sample-telemetry.json` vào topic telemetry.
- Mong đợi: Node-RED parse thành công, không phát cảnh báo và không có lỗi schema.

### MQTT-02: telemetry sai kiểu

- Đổi `heartRate` từ `78` thành chuỗi `"78"`.
- Mong đợi: Node-RED từ chối bản tin, không đưa lên biểu đồ chính và ghi `INVALID_PAYLOAD`.

### MQTT-03: retained status

- Publish online với retain true, ngắt MQTT In rồi kết nối lại.
- Mong đợi: Node-RED nhận ngay trạng thái cuối được giữ bởi broker.

### MQTT-04: command setMode

- Publish `sample-command-set-mode.json` vào topic command.
- Mong đợi: Serial Monitor hiển thị chế độ `high_hr`; telemetry tiếp theo có nhịp tim cao.

### MQTT-05: command không hợp lệ

- Publish command `deleteEverything`.
- Mong đợi: ESP32 bỏ qua, ghi lỗi và tiếp tục publish telemetry.

### MQTT-06: alert

- Gửi telemetry có `heartRate=128`.
- Mong đợi: Node-RED tạo `HIGH_HEART_RATE` và publish lên topic alert.

## 10. Quy trình thay đổi hợp đồng

Nếu cần đổi topic hoặc trường JSON:

1. Tạo issue mô tả lý do.
2. Cập nhật tài liệu này và schema liên quan trong cùng pull request.
3. Hồng Vỹ kiểm tra bộ sinh dữ liệu.
4. Minh Thiện kiểm tra Wokwi/ESP32.
5. Lê Hậu kiểm tra Node-RED.
6. Thành Danh duyệt tích hợp và thông báo cả nhóm.

Không merge thay đổi hợp đồng nếu một trong các module chưa tương thích.
