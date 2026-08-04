# Dashboard Health Band

Dashboard thực tế được triển khai bằng **FlowFuse Node-RED Dashboard 2.0** trong `node-red/dashboard-template.html`, không phải ứng dụng tĩnh trong thư mục này.

## Mở Dashboard

```text
http://localhost:1880/dashboard/overview
```

## Các phần của Dashboard

1. **Overview**: chỉ số chính, cảm biến mở rộng, biểu đồ, trạng thái, điều khiển scenario và alert.
2. **Digital twin**: vòng tay số, OLED/LED/buzzer/SOS mô phỏng.
3. **Smart Coach**: phân tích, mục tiêu cá nhân và demo tự động.
4. **Health profile**: thông tin cơ thể cơ bản, BMI và mục tiêu ngủ theo tuổi.
5. **App guide**: hướng dẫn tích hợp trong ứng dụng.
6. **Activity log**: báo cáo tự động, sao lưu/phục hồi và hoạt động gần đây.

Dashboard hỗ trợ 🇻🇳 Tiếng Việt và 🇺🇸 English. Ngôn ngữ được lưu trong `localStorage` của trình duyệt.

## Sửa giao diện

1. Sửa `../node-red/dashboard-template.html`.
2. Chạy `node ../node-red/build-english-dashboard.js` từ thư mục gốc repository.
3. Deploy lại `node-red/data/flows.json` qua API Node-RED.
4. Nhấn `Ctrl+F5` trong trình duyệt.

Hướng dẫn cài/chạy: [../DEPLOYMENT.md](../DEPLOYMENT.md).
