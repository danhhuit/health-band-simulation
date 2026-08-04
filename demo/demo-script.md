# Kịch bản demo ngắn (5–7 phút)

Mục tiêu: cho giảng viên thấy đầy đủ chuỗi **thiết bị → MQTT → Node-RED → Dashboard → lệnh quay lại thiết bị**.

## Chuẩn bị

- Docker/Node-RED đang `healthy`.
- Firmware PlatformIO build thành công.
- Wokwi đang chạy và Dashboard hiển thị dữ liệu.
- Mở sẵn Wokwi và Dashboard cạnh nhau.

## Kịch bản nói và thao tác

### 1. Giới thiệu — 30 giây

> Đây là vòng tay theo dõi sức khỏe cá nhân mô phỏng. Dữ liệu không phải dữ liệu y tế thật; mục tiêu là minh họa kiến trúc IoT bốn tầng và luồng cảnh báo hai chiều.

Mở **Overview**, chỉ ra trạng thái Connected, HR, SpO₂, bước và pin.

### 2. Kiến trúc — 45 giây

Mở **IoT architecture**. Nhấn từng tầng và nói ngắn:

1. ESP32/Wokwi sinh dữ liệu và phản ứng OLED/LED/còi.
2. Wi-Fi/MQTT truyền JSON.
3. Node-RED xử lý rule/cảnh báo/offline.
4. Dashboard giúp người dùng theo dõi và điều khiển.

Nhấn **Play data journey**.

### 3. Cảnh báo HR cao — 60 giây

1. Trên Overview chọn **High HR**.
2. Hỏi người xem: “Theo bạn dữ liệu nào thay đổi đầu tiên?”
3. Chỉ ra Dashboard gửi command, Wokwi đổi LED/OLED, HR tăng 125–145.
4. Chỉ ra Health alerts và Event timeline.

### 4. Té ngã — 60 giây

1. Chọn **Fall** hoặc nhấn nút FALL/SOS trên Wokwi.
2. Chỉ ra LED đỏ, còi, OLED/đồng hồ số và alert critical.
3. Nói rõ: fall được ưu tiên cao hơn các cảnh báo thường.

### 5. Điều khiển và phục hồi — 45 giây

1. Chọn **Normal**.
2. Quan sát cảnh báo hiện tại hết nhưng timeline vẫn giữ sự kiện.
3. Nhấn **Reset steps** để chứng minh lệnh điều khiển hai chiều.

### 6. Tính trực quan — 45 giây

1. Mở **Digital twin** để đối chiếu vòng tay số với Wokwi.
2. Chuyển 🇻🇳/🇺🇸 để chứng minh giao diện song ngữ.
3. Mở **App guide** để cho thấy ứng dụng có hướng dẫn sử dụng tích hợp.

### 7. Kết luận — 30 giây

> Dự án chứng minh được dữ liệu mô phỏng đi từ ESP32 đến Dashboard và lệnh từ Dashboard quay về ESP32. Các giới hạn như TLS, cảm biến thật và kiểm thử nhiều thiết bị đã được ghi rõ trong báo cáo/test case.

## Phương án dự phòng

Nếu broker/Internet lỗi, dùng ảnh trong `tests/evidence/`, video dự phòng và báo cáo trong `D:\IOTs\tailieu26`.
