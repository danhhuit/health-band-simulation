# Hướng dẫn demo tương tác Health Band

## 1. Mục tiêu

Không chỉ đọc số liệu. Mỗi thao tác phải cho người xem thấy chuỗi sau:

```text
Người xem chọn tình huống
  → Dashboard gửi MQTT command
  → ESP32/Wokwi nhận lệnh và đổi đầu ra
  → telemetry/event quay về Node-RED
  → rule tạo alert và Dashboard cập nhật
```

## 2. Chuẩn bị trước giờ trình bày

1. Làm theo [../DEPLOYMENT.md](../DEPLOYMENT.md) đến khi Dashboard có dữ liệu.
2. Kiểm tra Node-RED MQTT nodes là `connected`.
3. Chọn `Normal`, chờ ít nhất một telemetry cycle (2 giây).
4. Mở Wokwi và Dashboard cạnh nhau.
5. Chuẩn bị sẵn Word, Excel và thư mục `tests/evidence` dự phòng.

## 3. Chức năng từng trang

| Trang | Cách dùng khi demo |
|---|---|
| Overview | Chọn scenario, xem chỉ số, alert và timeline. |
| Digital twin | Đối chiếu vòng tay số với OLED/RGB/buzzer Wokwi. |
| IoT architecture | Nhấn layer hoặc `Play data journey` để giải thích 4 tầng. |
| Presenter mode | Dùng câu hỏi và nút `Run this step` để dẫn dắt người xem. |
| App guide | Giải thích nhanh toàn bộ giao diện khi người xem hỏi. |

## 4. Kịch bản tương tác đề xuất

### A. Normal

- Chọn `Normal`.
- Kết quả: HR 70–90, SpO₂ 97–99, LED xanh, không có health alert.
- Câu hỏi: “Nếu dữ liệu đang bình thường, hệ thống cần làm gì?”

### B. High HR hoặc Low SpO₂

- Mời người xem chọn một trong hai.
- Chọn scenario tương ứng.
- Chỉ ra: command trong timeline, OLED/RGB đổi, telemetry đổi, alert xuất hiện.
- Câu hỏi: “Vì sao Dashboard cần chờ dữ liệu telemetry quay về thay vì chỉ đổi màu ngay khi nhấn nút?”

### C. Fall / SOS

- Chọn `Fall` hoặc nhấn FALL/SOS trên Wokwi.
- Kết quả: `fallDetected=true`, LED đỏ, buzzer, alert critical, Digital Twin hiển thị fall.
- Câu hỏi: “Tại sao fall được ưu tiên hơn pin yếu?”

### D. Phục hồi và reset

- Chọn `Normal`.
- Nhấn `Reset steps`.
- Kết quả: hệ thống về normal, số bước về 0; timeline vẫn giữ sự kiện trước đó.

### E. Song ngữ

- Nhấn 🇻🇳 Tiếng Việt rồi 🇺🇸 English.
- Tải lại trang để cho thấy lựa chọn ngôn ngữ vẫn được lưu.

## 5. Dùng Presenter mode

Presenter mode có sẵn các bước. Ở mỗi bước:

1. Đọc mục tiêu và câu hỏi trên màn hình.
2. Cho người xem dự đoán kết quả.
3. Nhấn `Run this step`.
4. Đối chiếu Live proof với Wokwi/Dashboard.

## 6. Ảnh cần chụp/lưu

- Wokwi đang chạy: ESP32, OLED, RGB LED, buzzer, FALL/SOS.
- Overview normal.
- High HR hoặc Low SpO₂ có alert.
- Fall có LED đỏ/còi/alert critical.
- Digital Twin đối chiếu với Wokwi.
- Architecture có đủ 4 tầng.
- Presenter mode có câu hỏi.
- App Guide và nút 🇻🇳/🇺🇸.

Danh sách chi tiết: [../tests/STAGE_4_TEST_GUIDE.md](../tests/STAGE_4_TEST_GUIDE.md).

## 7. Khắc phục nhanh khi đang demo

| Hiện tượng | Xử lý |
|---|---|
| Dashboard không có dữ liệu | Đợi 5–10 giây, `Ctrl+F5`, kiểm tra Wokwi và MQTT connected. |
| Wokwi chạy firmware cũ | Stop, build lại, Start. |
| MQTT disconnected | Kiểm tra Internet/broker; dùng ảnh/video dự phòng nếu cần. |
| Không thấy Serial | Dùng OLED, LED, Node-RED connected và Dashboard thay đổi làm bằng chứng. |
