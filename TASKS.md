# Phân công và checklist dự án

## 1. Quy ước trạng thái

- `DONE`: đã hoàn thành và có bằng chứng.
- `DOING`: đang làm.
- `NEXT`: việc ưu tiên tiếp theo.
- `OPTIONAL`: chỉ làm khi còn thời gian.

## 2. Phần việc đã hoàn thành

| Mã | Công việc | Người phụ trách chính | Trạng thái | Bằng chứng |
|---|---|---|---|---|
| T01 | Repository, kiến trúc 4 tầng, MQTT prefix | Thành Danh | DONE | README, ARCHITECTURE, mqtt-topics. |
| T02 | Firmware ESP32/Wokwi, OLED/RGB/buzzer/FALL | Minh Thiện | DONE | `firmware-wokwi/src/main.cpp`, Wokwi diagram. |
| T03 | Sinh telemetry và các mode demo | Hồng Vỹ | DONE | Firmware, JSON schema, Dashboard. |
| T04 | Wi-Fi, MQTT, status retained, command/event | Thành Danh | DONE | Firmware + Node-RED flow. |
| T05 | Node-RED Docker, flow, Dashboard 5 trang | Lê Hậu | DONE | `node-red/`, Dashboard local. |
| T06 | Cảnh báo HR, SpO₂, fall, pin, offline | Lê Hậu + Hồng Vỹ | DONE | Node-RED rule, E2E evidence. |
| T07 | Song ngữ Việt/Anh, App guide, Digital Twin | Thành Danh | DONE | Dashboard template, E2E tests. |
| T08 | Excel/Word kết quả kiểm thử | Cả nhóm | DONE | `D:\IOTs\tailieu26`. |

## 3. Việc cần làm tiếp theo trước khi nộp

| Mã | Công việc | Người phụ trách | Ưu tiên | Đầu ra |
|---|---|---|---|---|
| N01 | Chụp đủ ảnh bằng chứng theo test guide | Minh Thiện + Lê Hậu | NEXT | `tests/evidence/`. |
| N02 | Quay video demo dự phòng 3–5 phút | Thành Danh + cả nhóm | NEXT | Video trong `demo/` hoặc `tailieu26`. |
| N03 | Rà soát báo cáo Word, thay ảnh minh chứng thật nếu cần | Hồng Vỹ + Thành Danh | NEXT | Word/PDF bản nộp. |
| N04 | Tập thuyết trình theo Presenter mode | Cả nhóm | NEXT | Demo 5–7 phút. |
| N05 | Bổ sung kiểm tra 3 mẫu liên tiếp cho HR/SpO₂ | Hồng Vỹ + Lê Hậu | OPTIONAL | Cập nhật firmware/Node-RED/test. |
| N06 | Bảo mật broker/TLS, nhiều thiết bị, endurance | Cả nhóm | OPTIONAL | Chỉ làm nếu giảng viên yêu cầu. |

## 4. Checklist trước giờ demo

- [ ] Docker Desktop đang chạy; `docker compose ps` báo `healthy`.
- [ ] Firmware PlatformIO build thành công.
- [ ] Wokwi đã Stop/Start sau lần build cuối.
- [ ] Dashboard mở được tại <http://localhost:1880/dashboard/overview>.
- [ ] MQTT node của Node-RED là `connected`.
- [ ] Đã thử `Normal`, `High HR`, `Low SpO₂`, `Fall`, `Low battery`, `Reset steps`.
- [ ] Đã có ảnh/video dự phòng.
- [ ] Đã chuẩn bị link/đường dẫn Word và Excel nộp bài.
