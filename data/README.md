# Dữ liệu mô phỏng và JSON Schema

Phiên bản hợp đồng hiện tại là v0.6.0. Telemetry có thêm profile `child`, giới tính, trạng thái đeo, huyết áp ước tính, sensor-fusion sleep và checkpoint recovery. Khi không đeo, HR/SpO₂/BP bằng 0 nhưng phải đi kèm `vitalDataValid=false`; phía xử lý không được xem đây là chỉ số sức khỏe thật.

Thư mục `data/` là hợp đồng dữ liệu dùng chung. Không lưu dữ liệu sức khỏe thật tại đây.

## Tệp có trong thư mục

| Nhóm | JSON mẫu | Schema |
|---|---|---|
| Telemetry | `sample-telemetry.json` | `telemetry.schema.json` |
| Status online/offline | `sample-status-online.json`, `sample-status-offline.json` | `status.schema.json` |
| Command | `sample-command-set-mode.json`, `sample-command-reset-steps.json`, `sample-command-set-profile.json`, `sample-command-set-power-mode.json` | `command.schema.json` |
| Alert | `sample-alert.json` | `alert.schema.json` |
| Event | `sample-event-command-accepted.json` | `event.schema.json` |

## Quy tắc quan trọng

- `deviceId` cố định: `health-band-01`.
- Telemetry có 13 trường bắt buộc, gồm `profile`, `powerMode`, `samplingIntervalMs`; `timestamp >= 0`, `seq >= 0`.
- HR cho phép 40–200; SpO₂ 70–100; pin 0–100 trong schema.
- `signalQuality`: `good`, `medium`, `poor`.
- `mode`: `normal`, `high_hr`, `low_hr`, `low_spo2`, `fall`, `low_battery`.
- Command chấp nhận `setMode`, `resetSteps`, `setProfile`, `setPowerMode`, `ackAlert`, `emergencyAction`.

## Kiểm tra JSON không lỗi cú pháp

Chạy ở thư mục gốc:

```powershell
Get-ChildItem .\data\*.json | ForEach-Object {
  Get-Content $_.FullName -Raw | ConvertFrom-Json | Out-Null
  Write-Output "OK: $($_.Name)"
}
```

Schema mô tả dữ liệu hợp lệ. Node-RED kiểm tra thêm required field, range, sequence, timestamp và signal quality trước khi chuyển sang Dashboard.

Hợp đồng topic/publisher/subscriber: [../node-red/mqtt-topics.md](../node-red/mqtt-topics.md).
