# Đồ án 31 - Vòng tay theo dõi sức khỏe mô phỏng

## Mục tiêu

Xây dựng một hệ thống IoT mô phỏng vòng tay theo dõi sức khỏe cá nhân. Thiết bị ESP32 ảo sinh dữ liệu nhịp tim, SpO2, số bước, trạng thái té ngã và pin; dữ liệu được gửi qua MQTT đến Node-RED để kiểm tra, lưu trữ, cảnh báo và hiển thị.

Đồ án không yêu cầu mua linh kiện và không phải thiết bị y tế.

## Kiến trúc

Hệ thống sử dụng kiến trúc IoT 4 tầng:

1. Thiết bị: ESP32 và cảm biến/điều khiển ảo trên Wokwi.
2. Mạng: Wi-Fi mô phỏng và giao thức MQTT.
3. Xử lý: MQTT Broker và Node-RED.
4. Ứng dụng: dashboard, lịch sử và cảnh báo.

## Công cụ

- Wokwi và ESP32 ảo.
- MQTT Broker.
- Node-RED.
- Git và GitHub.

## Cấu trúc repository

- `firmware-wokwi`: mã ESP32 và sơ đồ Wokwi.
- `node-red`: Node-RED Flow và ghi chú cấu hình.
- `dashboard`: thiết kế giao diện và ảnh kết quả.
- `data`: schema và dữ liệu JSON/CSV mô phỏng.
- `tests`: ca kiểm thử và bằng chứng.
- `demo`: kịch bản demo và video dự phòng.
- `TASKS.md`: bảng phân công và tiến độ.
- `DOCUMENTATION.md`: quy ước lưu tài liệu ngoài repository.

## Vị trí tài liệu

Tài liệu khảo sát, sơ đồ, biên bản và báo cáo được lưu tại workspace cục bộ:

```text
..\..\tailieu\do-an-31\docs
```

Đường dẫn này nằm ngoài repository mã nguồn nên không được Git của repository này theo dõi. Xem `DOCUMENTATION.md` để biết cách quản lý.

## MQTT topic prefix

```text
iot31/nhom-thanh-danh/health-band
```

Các topic dự kiến:

```text
iot31/nhom-thanh-danh/health-band/telemetry
iot31/nhom-thanh-danh/health-band/status
iot31/nhom-thanh-danh/health-band/command
iot31/nhom-thanh-danh/health-band/alert
```

Hợp đồng MQTT chính thức, gồm publisher/subscriber, QoS, retain, payload và ca kiểm thử, nằm tại [`node-red/mqtt-topics.md`](node-red/mqtt-topics.md).

Payload và JSON Schema dùng chung:

- [`data/sample-telemetry.json`](data/sample-telemetry.json) và [`data/telemetry.schema.json`](data/telemetry.schema.json).
- [`data/sample-status-online.json`](data/sample-status-online.json), [`data/sample-status-offline.json`](data/sample-status-offline.json) và [`data/status.schema.json`](data/status.schema.json).
- [`data/sample-command-set-mode.json`](data/sample-command-set-mode.json), [`data/sample-command-reset-steps.json`](data/sample-command-reset-steps.json) và [`data/command.schema.json`](data/command.schema.json).
- [`data/sample-alert.json`](data/sample-alert.json) và [`data/alert.schema.json`](data/alert.schema.json).

## Mốc kỹ thuật đầu tiên

Hoàn thành đường truyền tối thiểu:

```text
ESP32 ảo trên Wokwi -> MQTT Broker -> Node-RED Debug
```

Chỉ chuyển sang làm dashboard khi Node-RED nhận và phân tích đúng bản tin JSON.

## Cách bắt đầu

1. Mỗi thành viên chuyển sang đúng nhánh của mình.
2. Đọc `TASKS.md` và README trong module phụ trách.
3. Làm một thay đổi nhỏ, kiểm tra và commit trên nhánh cá nhân.
4. Push nhánh và nhờ người được phân công review.
5. Chỉ merge vào `main` khi đã có bằng chứng chạy được.
