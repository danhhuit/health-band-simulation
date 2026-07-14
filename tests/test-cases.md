# Ca kiểm thử ban đầu

| Mã | Tình huống | Thao tác | Kết quả mong đợi | Trạng thái |
|---|---|---|---|---|
| TC01 | Khởi động bình thường | Chạy Wokwi và Node-RED | Có telemetry hợp lệ trong Node-RED Debug | Chưa chạy |
| TC02 | Nhấn STEP | Nhấn nút STEP một lần | `steps` tăng đúng một đơn vị | Chưa chạy |
| TC03 | Phát hiện té ngã | Nhấn nút FALL | `fallDetected=true` và có cảnh báo | Chưa chạy |
| TC04 | Nhịp tim cao | Chọn chế độ High HR | Dashboard đổi giá trị và phát cảnh báo | Chưa chạy |
| TC05 | SpO2 thấp | Chọn chế độ Low SpO2 | Dashboard đổi giá trị và phát cảnh báo | Chưa chạy |
| TC06 | Pin yếu | Đưa pin xuống dưới ngưỡng demo | Có cảnh báo pin yếu | Chưa chạy |
| TC07 | JSON sai | Gửi bản tin thiếu trường bắt buộc | Bản tin bị đánh dấu lỗi, hệ thống không treo | Chưa chạy |
| TC08 | Mất kết nối | Dừng publish telemetry | Dashboard hiển thị thiết bị offline | Chưa chạy |

Mỗi lần chạy phải lưu ảnh hoặc log vào `tests/evidence`.
