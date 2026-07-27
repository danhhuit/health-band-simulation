# Dữ liệu mô phỏng và JSON Schema

Thư mục `data/` là hợp đồng dữ liệu dùng chung. Không lưu dữ liệu sức khỏe thật tại đây.

## Tệp có trong thư mục

| Nhóm | JSON mẫu | Schema |
|---|---|---|
| Telemetry | `sample-telemetry.json` | `telemetry.schema.json` |
| Status online/offline | `sample-status-online.json`, `sample-status-offline.json` | `status.schema.json` |
| Command | `sample-command-set-mode.json`, `sample-command-reset-steps.json` | `command.schema.json` |
| Alert | `sample-alert.json` | `alert.schema.json` |
| Event | `sample-event-command-accepted.json` | `event.schema.json` |

## Quy tắc quan trọng

- `deviceId` cố định: `health-band-01`.
- Telemetry có đủ 10 trường bắt buộc, `timestamp >= 0`, `seq >= 0`.
- HR cho phép 40–200; SpO₂ 70–100; pin 0–100 trong schema.
- `signalQuality`: `good`, `medium`, `poor`.
- `mode`: `normal`, `high_hr`, `low_hr`, `low_spo2`, `fall`, `low_battery`.
- Command chỉ chấp nhận `setMode` hoặc `resetSteps`.

## Kiểm tra JSON không lỗi cú pháp

Chạy ở thư mục gốc:

```powershell
Get-ChildItem .\data\*.json | ForEach-Object {
  Get-Content $_.FullName -Raw | ConvertFrom-Json | Out-Null
  Write-Output "OK: $($_.Name)"
}
```

Schema mô tả dữ liệu hợp lệ. Node-RED MVP hiện parse JSON cơ bản; schema validation đầy đủ là hạng mục nâng cấp.

Hợp đồng topic/publisher/subscriber: [../node-red/mqtt-topics.md](../node-red/mqtt-topics.md).
