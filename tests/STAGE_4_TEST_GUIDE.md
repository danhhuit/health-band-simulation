# Kiểm thử và thu thập bằng chứng

## 1. Mục tiêu

Chứng minh đầy đủ luồng:

```text
ESP32/Wokwi → MQTT → Node-RED → Dashboard → command → ESP32/Wokwi
```

Một test chỉ ghi **Đạt** khi có thao tác tái hiện, kết quả quan sát được và ít nhất một ảnh/log bằng chứng.

## 2. Trình tự kiểm thử

1. Chạy setup theo [../DEPLOYMENT.md](../DEPLOYMENT.md).
2. Chạy 12 smoke test trong [test-cases.md](test-cases.md).
3. Lưu ảnh vào `tests/evidence/`.
4. Cập nhật Excel kết quả trong `D:\IOTs\tailieu26`.
5. Nếu phát hiện lỗi, ghi rõ nguyên nhân, trạng thái và cách tái hiện.

## 3. Ảnh bằng chứng tối thiểu

| Mã | Nội dung ảnh | Tên gợi ý |
|---|---|---|
| EV01 | Wokwi đang chạy với OLED/RGB/buzzer/FALL | `SM-01-wokwi-running.png` |
| EV02 | Node-RED MQTT connected | `SM-01-node-red-connected.png` |
| EV03 | Overview normal | `SM-02-overview-normal.png` |
| EV04 | High HR alert | `SM-03-high-hr.png` |
| EV05 | Low SpO₂ alert | `SM-05-low-spo2.png` |
| EV06 | Fall critical + Wokwi LED đỏ | `SM-06-fall.png` |
| EV07 | Low battery | `SM-07-low-battery.png` |
| EV08 | Reset steps trước/sau | `SM-08-before.png`, `SM-08-after.png` |
| EV09 | Digital Twin | `SM-11-digital-twin.png` |
| EV10 | Activity log: báo cáo/sao lưu/sự kiện | `SM-10-activity-log.png` |
| EV11 | Smart Coach/App guide/song ngữ | `SM-09-guide-language.png` |
| EV12 | Offline rồi recovery | `SM-12-recovery.png` |

## 4. Quy tắc chụp ảnh

- Ảnh phải thấy rõ chỉ số/tình huống/cảnh báo liên quan.
- Không cắt quá chặt khiến không biết đó là Dashboard hay Wokwi.
- Không sửa số liệu trên ảnh.
- Một ảnh có thể minh chứng nhiều điểm chỉ khi thấy rõ tất cả các điểm đó.
- Dùng tên file không dấu, có mã test để dễ tra cứu.

## 5. Kiểm thử tự động đã có

### Header, Notification Center và Theme

Chạy:

```powershell
node .\tests\header-features-e2e.mjs
```

Kết quả mong đợi: `7/7 PASS`.

Bằng chứng:

- `tests/evidence/header/header-features-results.json`
- `tests/evidence/header/dashboard-dark-theme.png`
- `tests/evidence/header/dashboard-light-theme.png`
- `tests/evidence/header/notification-center.png`

### Smart Coach F1–F12

Chạy:

```powershell
npm install --prefix .\tests
node .\tests\smart-features-e2e.mjs
```

Máy hiện tại đã có Playwright trong môi trường Codex nên có thể chạy trực tiếp. Trên máy mới, chỉ cần chạy lệnh `npm install` một lần.

Kết quả mong đợi:

```text
total: 16
passed: 16
failed: 0
```

Bằng chứng tự động:

- `tests/evidence/smart/smart-features-results.json`
- `tests/evidence/smart/smart-coach-desktop.png`
- `tests/evidence/smart/smart-coach-mobile.png`

Kịch bản E2E đã kiểm tra:

- Dashboard tải được, 6 tab hiển thị đúng thứ tự.
- 🇻🇳/🇺🇸 chuyển ngôn ngữ và lưu lựa chọn.
- Telemetry live, High HR, Low SpO₂, Fall.
- Digital Twin, Smart Coach, Health profile và Activity log.
- App guide, MQTT command, màn hình 390 px.
- Không có lỗi JavaScript trên giao diện.

Ảnh tự động: `tests/evidence/e2e/`.

## 6. Giới hạn phải ghi trong báo cáo

- Không dùng cảm biến/người đeo/pin/sạc thật.
- Broker public không có TLS/xác thực trong MVP.
- Chưa kiểm thử tải nhiều thiết bị, 24 giờ hoặc 7 ngày.
- Rule ngưỡng hiện là rule demo; không kết luận y khoa.
