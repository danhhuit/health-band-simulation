# So sánh với thiết bị thực tế

## Kết luận nhanh

Project này mạnh ở **khả năng giải thích kiến trúc IoT, tương tác và tái hiện kịch bản**. Nó không thể đạt độ chính xác, kiểm định, thuật toán độc quyền, độ bền hay hệ sinh thái của thiết bị thương mại/nghiên cứu.

| Nội dung | Project Health Band | Apple Watch Series 9 | Fitbit Charge 6 | Empatica E4 |
|---|---|---|---|---|
| Mục tiêu | Đồ án mô phỏng, giáo dục | Đồng hồ thông minh tiêu dùng | Vòng theo dõi sức khỏe/fitness tiêu dùng | Thu thập dữ liệu sinh lý cho nghiên cứu |
| HR/PPG | Thanh trượt mô phỏng tín hiệu quang | Cảm biến tim quang thế hệ 3, thêm cảm biến tim điện | Theo dõi HR, resting HR, HRV | PPG xuất dữ liệu thô |
| SpO₂ | Giá trị mô phỏng | Có tùy khu vực/phiên bản và chính sách tính năng | Ước tính SpO₂ khi ngủ | Không phải chức năng SpO₂ tiêu dùng chính |
| Chuyển động | MPU6050 mô phỏng | Gia tốc kế high-g, gyroscope | Cảm biến chuyển động cho hoạt động/ngủ | Gia tốc kế 3 trục 32 Hz |
| Nhiệt độ | DS18B20 mô phỏng nhiệt độ bề mặt | Cảm biến nhiệt độ | Biến thiên nhiệt độ da | Cảm biến nhiệt độ da |
| EDA/stress | Chưa có cảm biến EDA | Không phải cảm biến cốt lõi của Series 9 | EDA Scan và Stress Management | EDA là cảm biến nghiên cứu cốt lõi |
| ECG | Không có | Có ECG một chuyển đạo ở khu vực hỗ trợ | Có ECG/AFib assessment ở khu vực hỗ trợ | Không phải thiết bị ECG |
| Giấc ngủ | Sensor fusion giải thích được, ngưỡng demo tăng tốc | Sleep app và sleep stages | Sleep log, stages, score | Cung cấp raw signals; phân tích tùy pipeline nghiên cứu |
| Huyết áp | **Ước tính mô phỏng**, không có cuff | Không có cảm biến đo huyết áp được liệt kê trong Series 9 | Không có cảm biến đo huyết áp được liệt kê | Không đo huyết áp |
| GPS | NMEA giả lập | GPS/GNSS tích hợp | GPS tích hợp/kết nối | Không phải tính năng cốt lõi |
| Hiển thị | OLED Wokwi + Digital Twin | LTPO OLED always-on | Màn hình màu nhỏ | Không có màn hình |
| Dữ liệu thô | MQTT JSON có thể xem hoàn toàn | Hạn chế theo API/hệ sinh thái | Hạn chế theo ứng dụng/API | Điểm mạnh: PPG, EDA, nhiệt độ, gia tốc thô |
| Chứng nhận/độ chính xác | Không, chỉ demo | Sản phẩm thương mại; một số tính năng có quy định theo vùng | Sản phẩm thương mại; tính năng phụ thuộc vùng | Thiết bị nghiên cứu, không thay thế chẩn đoán |
| Mất nguồn | NVS + browser backup + Docker volume | Cơ chế hệ điều hành/đồng bộ hệ sinh thái | Đồng bộ tài khoản/ứng dụng | Bộ nhớ/stream và nền tảng nghiên cứu |

## Điểm khác biệt cần nói khi thuyết trình

1. HR, SpO₂ và huyết áp của project là dữ liệu mô phỏng hoặc ước tính; không có thuật toán hiệu chuẩn lâm sàng.
2. Giấc ngủ của project minh họa sensor fusion: chuyển động + HR + SpO₂ + nhiệt độ + thời gian. Các giai đoạn `light/deep` chỉ là trạng thái demo.
3. Apple Watch và Fitbit có thuật toán, kiểm thử phần cứng, quản lý pin và hệ sinh thái ứng dụng hoàn chỉnh.
4. Empatica E4 ưu tiên chất lượng tín hiệu thô phục vụ nghiên cứu, đặc biệt EDA/PPG, không ưu tiên giao diện người tiêu dùng. E4 hiện đã ngừng bán và được thay bằng EmbracePlus.
5. Lợi thế học thuật của project là toàn bộ luồng Device → MQTT → Node-RED → Dashboard có thể quan sát, sửa và chứng minh trực tiếp.

## Nguồn chính thức

- [Apple Watch Series 9 – Tech Specs](https://support.apple.com/en-ie/111833)
- [Fitbit Charge 6 – Get started](https://support.google.com/googlehealth/answer/14237104?hl=en-GB)
- [Fitbit data and supported health metrics](https://support.google.com/product-documentation/answer/14779613?hl=en)
- [Empatica E4 product and discontinuation](https://www.empatica.com/research/e4/)
- [Empatica E4 technical specifications](https://box.empatica.com/documentation/20141119_E4_TechSpecs.pdf)
