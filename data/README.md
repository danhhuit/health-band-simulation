# Dữ liệu mô phỏng

Không lưu dữ liệu sức khỏe thật trong thư mục này.

Payload telemetry dự kiến:

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
