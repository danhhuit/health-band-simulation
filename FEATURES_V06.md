# Health Band v0.6.0 – Chức năng mới

## 1. Hồ sơ người dùng

- Bốn đối tượng: Student, Older adult, Athlete và Child.
- Mọi đối tượng chọn được giới tính Male/Female.
- Profile thay đổi ngưỡng demo và gợi ý mục tiêu; giới tính hiện là metadata để phân nhóm báo cáo, không tự ý biến thành kết luận y tế.

## 2. Huyết áp

Telemetry thêm `bloodPressure.systolic/diastolic`. Giá trị được ước tính từ profile, HR và nhiễu mô phỏng.

> Đây không phải phép đo bằng cuff, không phải PTT đã hiệu chuẩn và không có giá trị chẩn đoán.

## 3. Không đeo 24/24

Ba chế độ:

- `auto`: suy luận đang đeo từ tín hiệu HR/SpO₂ và nhiệt độ bề mặt.
- `worn`: ép trạng thái đang đeo để demo.
- `off_wrist`: mô phỏng tháo vòng.

Khi tháo vòng:

- ESP32 và MQTT vẫn online.
- HR, SpO₂, huyết áp chuyển thành 0 kèm `vitalDataValid=false`.
- Node-RED không tạo cảnh báo sức khỏe từ mẫu thiếu.
- Biểu đồ không thêm điểm 0.
- Báo cáo ghi **wear coverage** và loại mẫu khỏi trung bình.
- Giấc ngủ chuyển `not_tracked`; không suy luận “đã ngủ”.

## 4. Giấc ngủ tự động

Sensor fusion dùng:

1. MPU6050: bất động/tư thế.
2. HR: giảm và ổn định.
3. SpO₂: dữ liệu xác nhận.
4. DS18B20: vòng đang tiếp xúc bề mặt.
5. NTP và khung 21:00–09:00.

Trạng thái: `awake → candidate → light → deep`. Demo dùng 15/30/60 giây để quan sát được trong lớp; triển khai thực tế phải dùng thời gian dài hơn và thuật toán đã được kiểm chứng.

## 5. Báo cáo

- Hourly: tự chốt sau khoảng một giờ.
- Daily: chốt khi sang ngày mới.
- Monthly: chốt khi sang tháng mới.
- Có nút Generate now để kiểm tra ngay.
- Mỗi báo cáo chứa coverage, số mẫu hợp lệ, HR/SpO₂/BP trung bình, bước, ngủ và cảnh báo.

## 6. Sao lưu và phục hồi

| Lớp | Dữ liệu | Khi phục hồi |
|---|---|---|
| ESP32 NVS | bước, pin, scenario, profile, giới tính, power, wear mode | Khởi động firmware |
| Browser | hai snapshot gần nhất, báo cáo, cấu hình, lịch sử ngắn | Mở Dashboard |
| Docker/Node-RED | flow, context và trạng thái xử lý | Container restart |

ESP32 lưu định kỳ 30 giây và ngay sau lệnh. Dashboard có Export/Restore backup JSON.

## 7. Demo thông minh tự động

Nút tự động chạy độc lập theo kịch bản:

```text
Digital Twin → Architecture journey → Normal → High HR → Low SpO₂
→ Sleep → Fall → Low battery → Recovery
```

Mỗi bước mở đúng trang và gửi MQTT command thật nếu có.

## 8. Gemini

Gemini nhận duy nhất dữ liệu báo cáo tổng hợp đã ẩn danh qua Node-RED. Nếu thiếu khóa hoặc lỗi mạng, Smart Coach cục bộ vẫn trả khuyến cáo. Xem [GEMINI_SETUP.md](GEMINI_SETUP.md).
