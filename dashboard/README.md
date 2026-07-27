# Dashboard Health Band

Dashboard thực tế được triển khai bằng **FlowFuse Node-RED Dashboard 2.0** trong `node-red/dashboard-template.html`, không phải ứng dụng tĩnh trong thư mục này.

## Mở Dashboard

```text
http://localhost:1880/dashboard/overview
```

## Các phần của Dashboard

1. **Overview**: số đo, trạng thái, biểu đồ, alert, điều khiển scenario, timeline.
2. **Digital twin**: vòng tay số, OLED/LED/buzzer/SOS mô phỏng.
3. **IoT architecture**: bốn tầng và hành trình dữ liệu.
4. **Presenter mode**: kịch bản tương tác khi thuyết trình.
5. **App guide**: hướng dẫn tích hợp trong ứng dụng.

Dashboard hỗ trợ 🇻🇳 Tiếng Việt và 🇺🇸 English. Ngôn ngữ được lưu trong `localStorage` của trình duyệt.

## Sửa giao diện

1. Sửa `../node-red/dashboard-template.html`.
2. Chạy `node ../node-red/build-english-dashboard.js` từ thư mục gốc repository.
3. Deploy lại `node-red/data/flows.json` qua API Node-RED.
4. Nhấn `Ctrl+F5` trong trình duyệt.

Hướng dẫn cài/chạy: [../DEPLOYMENT.md](../DEPLOYMENT.md).
