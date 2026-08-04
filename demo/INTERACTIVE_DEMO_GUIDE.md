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
| Smart Coach | Chạy Demo thông minh tự động và theo dõi từng tình huống. |
| Health profile | Nhập hồ sơ mẫu, xem BMI và áp dụng mục tiêu ngủ. |
| App guide | Giải thích nhanh toàn bộ giao diện khi người xem hỏi. |
| Activity log | Tạo báo cáo, xuất sao lưu và xem dòng sự kiện. |

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

## 5. Dùng Demo thông minh tự động

Mở Smart Coach và nhấn **Start automatic demo**. Ứng dụng sẽ:

1. Mở Vòng tay số để giới thiệu thiết bị ảo.
2. Lần lượt gửi các tình huống qua MQTT.
3. Cập nhật vòng tay số, cảnh báo và timeline.
4. Trở về trạng thái Normal khi kết thúc.

## 6. Ảnh cần chụp/lưu

- Wokwi đang chạy: ESP32, OLED, RGB LED, buzzer, FALL/SOS.
- Overview normal.
- High HR hoặc Low SpO₂ có alert.
- Fall có LED đỏ/còi/alert critical.
- Digital Twin đối chiếu với Wokwi.
- Smart Coach đang chạy Demo thông minh tự động.
- App Guide và nút 🇻🇳/🇺🇸.
- Activity log có báo cáo, sao lưu và sự kiện.

Danh sách chi tiết: [../tests/STAGE_4_TEST_GUIDE.md](../tests/STAGE_4_TEST_GUIDE.md).

## 7. Khắc phục nhanh khi đang demo

| Hiện tượng | Xử lý |
|---|---|
| Dashboard không có dữ liệu | Đợi 5–10 giây, `Ctrl+F5`, kiểm tra Wokwi và MQTT connected. |
| Wokwi chạy firmware cũ | Stop, build lại, Start. |
| MQTT disconnected | Kiểm tra Internet/broker; dùng ảnh/video dự phòng nếu cần. |
| Không thấy Serial | Dùng OLED, LED, Node-RED connected và Dashboard thay đổi làm bằng chứng. |

## 8. Demo v0.6.0 trong 8–10 phút

1. **Normal**: giới thiệu HR, SpO₂, BP ước tính, bước, pin và đồng hồ toàn cục.
2. **Cá nhân hóa**: chọn Child, chọn Male/Female và đổi step goal.
3. **Không đeo**: chọn Off wrist; chứng minh thiết bị vẫn Connected nhưng vitals/sleep không được tính.
4. **Đeo lại**: chọn Worn hoặc Auto; dữ liệu sinh tồn trở lại.
5. **Sleep**: chọn mode Sleep, đợi 15–30 giây để xem Candidate/Light; giải thích sensor fusion và ngưỡng demo.
6. **Report**: bấm Generate now, tải báo cáo JSON và chỉ `wearCoveragePercent`.
7. **Gemini**: bấm Ask Gemini; nếu có key sẽ nhận gợi ý AI, nếu không sẽ dùng local fallback.
8. **Fall**: kích hoạt FALL/SOS và xác nhận countdown, LED, buzzer, alert.
9. **Recovery**: trở về Normal, export backup; restart Wokwi để chỉ event `DEVICE_RECOVERED`.

Nếu muốn chạy không cần thao tác từng bước, mở Smart Coach và bấm **Start automatic demo**:

```text
Digital Twin → Normal → High HR → Low SpO₂
→ Sleep → Fall → Low battery → Recovery
```
