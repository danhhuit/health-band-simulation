# Bảng nhiệm vụ đồ án 31

## Trạng thái

- `TODO`: chưa bắt đầu.
- `DOING`: đang thực hiện.
- `REVIEW`: chờ review chéo.
- `DONE`: đã kiểm thử và merge vào `main`.

## Nhiệm vụ khởi động

| Mã | Công việc | Người chính | Review | Trạng thái | Đầu ra bắt buộc |
|---|---|---|---|---|---|
| T01 | Hoàn thiện repository, quy ước Git và tích hợp hệ thống | Thành Danh | Lê Hậu | REVIEW | README, nhánh, commit và hướng dẫn tích hợp |
| T02 | Tạo ESP32 và điều khiển STEP/FALL trên Wokwi | Minh Thiện | Hồng Vỹ | TODO | `sketch.ino`, `diagram.json`, ảnh Serial Monitor |
| T03 | Xây bộ sinh HR/SpO2/bước/pin và payload JSON | Hồng Vỹ | Minh Thiện | TODO | Dữ liệu hợp lệ mỗi 1-2 giây, có tình huống biên |
| T04 | Kết nối Wi-Fi, MQTT và publish telemetry | Thành Danh | Lê Hậu | TODO | Node-RED nhận được JSON từ Wokwi |
| T05 | Tạo Node-RED Flow, kiểm tra dữ liệu và dashboard | Lê Hậu | Thành Danh | TODO | `flows.json`, ảnh dashboard và log lỗi |
| T06 | Tạo cảnh báo nhịp tim, SpO2, té ngã và pin yếu | Lê Hậu + Hồng Vỹ | Thành Danh | TODO | Bốn cảnh báo có thể kích hoạt khi demo |
| T07 | Thực hiện kiểm thử và thu thập bằng chứng | Cả nhóm | Thành Danh | TODO | Test case, ảnh/video và kết quả pass/fail |
| T08 | Hoàn thiện báo cáo, slide và kịch bản demo | Cả nhóm | Thành Danh | TODO | Word/PDF, slide và video dự phòng |

## Quy tắc hoàn thành

Một nhiệm vụ chỉ chuyển sang `DONE` khi:

1. Mã hoặc flow chạy được trên nhánh cá nhân.
2. Có ít nhất một ca bình thường và một ca bất thường.
3. Có ảnh, video hoặc log làm bằng chứng.
4. Có hướng dẫn chạy lại.
5. Được người review chéo xác nhận.
6. Đã merge vào nhánh `main`.
