# Bộ test nhanh cho MVP Health Band

Tệp này là checklist kỹ thuật ngắn để chạy trước demo. Bộ test đầy đủ 272 ca nằm trong:

```text
D:\IOTs\tailieu26\TestCase_VongTayTheoDoiSucKhoe_DaChay_2026-07-28.xlsx
```

## Kết quả tự động gần nhất

| Hạng mục | Kết quả |
|---|---|
| Kiểm thử trình duyệt E2E | 18/18 đạt |
| Build firmware PlatformIO | SUCCESS |
| Restart Node-RED | healthy, Dashboard HTTP 200, MQTT reconnect |
| Tổng test case đã đánh giá | 272 |

## Checklist smoke test

| ID | Tình huống | Thao tác | Kết quả mong đợi | Bằng chứng |
|---|---|---|---|---|
| SM-01 | Khởi động | Docker + Wokwi | Dashboard nhận telemetry, MQTT connected | Dashboard/Wokwi/Node-RED. |
| SM-02 | Normal | Chọn `Normal` | HR 70–90, SpO₂ 97–99, không alert | Overview. |
| SM-03 | High HR | Chọn `High HR` | HR 125–145, alert cao | Overview + timeline. |
| SM-04 | Low HR | Chọn `Low HR` | HR 40–48, alert thấp | Overview + timeline. |
| SM-05 | Low SpO₂ | Chọn `Low SpO₂` | SpO₂ 85–92, alert thấp | Overview + timeline. |
| SM-06 | Fall | Chọn `Fall`/FALL button | LED đỏ, buzzer, critical alert | Wokwi + Dashboard. |
| SM-07 | Low battery | Chọn `Low battery` | Pin 12–19, alert pin yếu | Overview. |
| SM-08 | Reset steps | Nhấn `Reset steps` | Steps về 0 | Ảnh trước/sau. |
| SM-09 | Song ngữ | Đổi 🇻🇳/🇺🇸 và reload | Ngôn ngữ hiển thị/lưu đúng | Dashboard. |
| SM-10 | Kiến trúc | Nhấn `Play data journey` | 4 layer lần lượt nổi bật | Architecture. |
| SM-11 | Lệnh hai chiều | Chọn scenario | Có `COMMAND_ACCEPTED`/thay đổi Wokwi | Timeline + Wokwi. |
| SM-12 | Offline/recovery | Dừng/chạy lại Wokwi | Offline sau 8 giây, sau đó connected lại | Dashboard. |

## Lưu ý khi ghi kết quả

- Ghi `Đạt`, `Không đạt`, `Bị chặn` hoặc `Không áp dụng`; không tự đánh dấu Đạt khi thiếu bằng chứng.
- Phần cứng người đeo thật, sạc thật, nhiều thiết bị, TLS và endurance không thuộc phạm vi MVP Wokwi hiện tại.
- Lưu ảnh/log trong `tests/evidence/`.
