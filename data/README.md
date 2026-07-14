# Dữ liệu mô phỏng và hợp đồng JSON

Không lưu dữ liệu sức khỏe thật trong thư mục này. Các payload và schema ở đây là nguồn quy ước chung cho ESP32, Node-RED và kiểm thử.

## Telemetry

Payload mẫu: `sample-telemetry.json`.

Schema: `telemetry.schema.json`.

```json
{
  "deviceId": "health-band-01",
  "timestamp": 0,
  "seq": 1,
  "heartRate": 78,
  "spo2": 98,
  "steps": 125,
  "fallDetected": false,
  "battery": 92,
  "signalQuality": "good"
}
```

Yêu cầu:

- Dữ liệu gửi mỗi 1-2 giây.
- Không có `NaN`, giá trị âm hoặc sai kiểu.
- Có thể tạo tình huống nhịp tim cao, SpO2 thấp, té ngã và pin yếu.
- `seq` tăng sau mỗi bản tin để phát hiện mất hoặc lặp bản tin.

## Status

- `sample-status-online.json`: publish sau khi ESP32 kết nối broker.
- `sample-status-offline.json`: payload Last Will khi ESP32 mất kết nối.
- `status.schema.json`: quy tắc kiểm tra chung.
- Topic status dùng QoS 1 và retain true.

## Command

- `sample-command-set-mode.json`: chuyển chế độ mô phỏng.
- `sample-command-reset-steps.json`: đặt bộ đếm bước về 0.
- `command.schema.json`: chỉ cho phép `setMode` và `resetSteps`.
- Topic command dùng QoS 1 và không retain.

## Alert

- `sample-alert.json`: cảnh báo mẫu do Node-RED tạo.
- `alert.schema.json`: loại cảnh báo, mức độ và trường bắt buộc.
- Topic alert dùng QoS 1 và không retain.

## Kiểm tra cú pháp nhanh

Chạy từ thư mục gốc repository:

```powershell
Get-ChildItem .\data\*.json | ForEach-Object {
    Get-Content $_.FullName -Raw | ConvertFrom-Json | Out-Null
    Write-Output "OK: $($_.Name)"
}
```

Chi tiết topic và luồng publish/subscribe xem `node-red/mqtt-topics.md`.
