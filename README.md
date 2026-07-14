\# Đồ án 31 - Vòng tay theo dõi sức khỏe mô phỏng



\## Mục tiêu



Xây dựng hệ thống IoT mô phỏng vòng tay theo dõi sức khỏe bằng ESP32 ảo,

Wokwi, MQTT và Node-RED.



Đồ án không yêu cầu mua linh kiện.



\## Kiến trúc



Hệ thống sử dụng kiến trúc IoT 4 tầng:



1\. Tầng thiết bị

2\. Tầng mạng

3\. Tầng xử lý

4\. Tầng ứng dụng



\## Cấu trúc mã nguồn



\- firmware-wokwi: chương trình ESP32 và sơ đồ Wokwi

\- node-red: Node-RED Flow và cấu hình

\- dashboard: giao diện dashboard

\- data: dữ liệu JSON/CSV mô phỏng

\- tests: ca kiểm thử và bằng chứng

\- demo: kịch bản demo



\## Vị trí tài liệu



Tài liệu dự án được lưu tại:



..\\..\\tailieu\\do-an-31\\docs



Xem thêm tệp DOCUMENTATION.md.



\## MQTT topic prefix



iot31/nhom-thanh-danh/health-band

