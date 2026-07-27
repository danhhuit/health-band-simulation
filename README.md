# Health Band Simulation — Đề tài 31

Mô phỏng vòng tay theo dõi sức khỏe cá nhân cho môn IoTs. Dự án sử dụng **ESP32 trên Wokwi**, **MQTT**, **Node-RED Dashboard 2.0** và không yêu cầu mua linh kiện thật.

> Lưu ý: dữ liệu nhịp tim, SpO₂ và cảnh báo trong dự án là dữ liệu mô phỏng phục vụ học tập; ứng dụng không phải thiết bị y tế và không dùng để chẩn đoán.

## 1. Dự án thể hiện điều gì?

Hệ thống mô phỏng một vòng tay có thể:

- Sinh nhịp tim, SpO₂, số bước, pin, tín hiệu và trạng thái té ngã.
- Gửi dữ liệu qua Wi-Fi/MQTT mỗi **2 giây**.
- Hiển thị OLED, LED RGB, còi và nút FALL/SOS ngay trên Wokwi.
- Phân tích dữ liệu bằng Node-RED, tạo cảnh báo và phát hiện offline sau 8 giây không nhận telemetry.
- Hiển thị Dashboard tương tác, mô hình vòng tay số, kiến trúc IoT 4 tầng, chế độ thuyết trình và hướng dẫn trong ứng dụng.

## 2. Kiến trúc 4 tầng

```text
Tầng 4 — Ứng dụng: Dashboard, Digital Twin, Presenter mode, App guide
                         ↑↓ MQTT command / telemetry / alert / event
Tầng 3 — Xử lý: Node-RED parse JSON, cảnh báo, offline, timeline
                         ↑↓ MQTT
Tầng 2 — Mạng: Wi-Fi Wokwi-GUEST + broker.emqx.io:1883
                         ↑↓ MQTT
Tầng 1 — Thiết bị: ESP32 Wokwi + OLED + RGB LED + buzzer + FALL/SOS
```

Giải thích chi tiết: [ARCHITECTURE.md](ARCHITECTURE.md).

## 3. Cài đặt nhanh (lần đầu)

### Điều kiện cần

- Windows 10/11, Internet ổn định.
- [Docker Desktop](https://www.docker.com/products/docker-desktop/) đang chạy.
- VS Code, extension **PlatformIO IDE** và **Wokwi Simulator**.
- Tài khoản/giấy phép Wokwi hợp lệ nếu extension yêu cầu.
- Cổng `1880` chưa bị ứng dụng khác dùng.

### Bước 1 — Mở đúng thư mục

Mở thư mục gốc này trong VS Code:

```text
D:\IOTs\projects\health-band-simulation
```

### Bước 2 — Build firmware ESP32

Mở PowerShell tại thư mục gốc và chạy:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --project-dir ".\firmware-wokwi"
```

Kết quả mong đợi: cuối log có dòng `[SUCCESS]`.

### Bước 3 — Khởi động Node-RED

```powershell
docker compose up --build -d
docker compose ps
```

Kết quả mong đợi: container `health-band-node-red` có trạng thái `healthy`.

### Bước 4 — Tạo và deploy Dashboard flow

```powershell
node .\node-red\build-english-dashboard.js
curl.exe -sS -X POST -H "Content-Type: application/json" --data-binary "@node-red/data/flows.json" http://localhost:1880/flows
```

Mở:

- Node-RED Editor: <http://localhost:1880>
- Dashboard: <http://localhost:1880/dashboard/overview>

### Bước 5 — Chạy Wokwi

1. Trong VS Code, mở Command Palette (`Ctrl+Shift+P`).
2. Chọn `Wokwi: Start Simulation`.
3. Đợi ESP32 kết nối Wi-Fi/MQTT; LED trạng thái nháy chậm khi đã kết nối.
4. Quay lại Dashboard và nhấn `Ctrl+F5` nếu chưa thấy dữ liệu.

Sau mỗi lần build firmware mới, hãy **Stop Simulation** rồi **Start Simulation** để Wokwi nạp file `.bin` mới.

## 4. Dùng Dashboard

Dashboard có 5 trang, chuyển bằng các tab ở đầu trang:

| Trang | Dùng để làm gì? |
|---|---|
| **Overview** | Xem HR, SpO₂, bước, pin, tín hiệu, biểu đồ, cảnh báo và timeline. |
| **Digital twin** | Quan sát chiếc vòng tay số hiển thị cùng dữ liệu với ESP32 Wokwi. |
| **IoT architecture** | Giải thích 4 tầng và nút `Play data journey`. |
| **Presenter mode** | Thuyết trình theo kịch bản, có câu hỏi cho người xem. |
| **App guide** | Xem giải thích từng trang và chức năng ngay trong ứng dụng. |

Ở góc đầu Dashboard có hai lựa chọn ngôn ngữ: **🇻🇳 Tiếng Việt** và **🇺🇸 English**. Lựa chọn được lưu sau khi tải lại trang.

### Các tình huống demo

| Nút | Tác động mô phỏng |
|---|---|
| `Normal` | HR 70–90, SpO₂ 97–99, không té ngã. |
| `High HR` | HR 125–145 và cảnh báo nhịp tim cao. |
| `Low HR` | HR 40–48 và cảnh báo nhịp tim thấp. |
| `Low SpO₂` | SpO₂ 85–92 và cảnh báo oxy máu thấp. |
| `Fall` | Té ngã, LED đỏ, còi và cảnh báo critical. |
| `Low battery` | Pin 12–19% và cảnh báo pin yếu. |
| `Reset steps` | Đặt số bước về 0. |

## 5. MQTT dùng chung

```text
Broker: broker.emqx.io:1883
Prefix: iot31/nhom-thanh-danh/health-band
Device ID: health-band-01
```

| Topic cuối | Hướng | Mục đích |
|---|---|---|
| `telemetry` | ESP32 → Node-RED | Số đo và trạng thái mô phỏng. |
| `status` | ESP32 → Node-RED | Online/offline, retained. |
| `command` | Dashboard/Node-RED → ESP32 | `setMode`, `resetSteps`. |
| `event` | ESP32 → Node-RED | Xác nhận lệnh, FALL/SOS cục bộ. |
| `alert` | Node-RED → Dashboard | Cảnh báo từ rule xử lý. |

Xem payload, QoS và các lệnh mẫu tại [node-red/mqtt-topics.md](node-red/mqtt-topics.md).

## 6. Kiểm tra nhanh

```powershell
# Kiểm tra container
docker compose ps

# Xem log Node-RED/MQTT
docker logs --tail 60 health-band-node-red

# Build lại firmware
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --project-dir ".\firmware-wokwi"
```

Danh sách lỗi thường gặp và cách xử lý: [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

## 7. Tài liệu theo nhu cầu

| Khi cần | Đọc tệp |
|---|---|
| Cài đặt/khởi động chi tiết | [DEPLOYMENT.md](DEPLOYMENT.md) |
| Lỗi Wokwi, Docker, MQTT, Dashboard | [TROUBLESHOOTING.md](TROUBLESHOOTING.md) |
| Giải thích kiến trúc 4 tầng | [ARCHITECTURE.md](ARCHITECTURE.md) |
| Firmware ESP32/Wokwi | [firmware-wokwi/README.md](firmware-wokwi/README.md) |
| Flow và topic MQTT | [node-red/mqtt-topics.md](node-red/mqtt-topics.md) |
| Payload/JSON schema | [data/README.md](data/README.md) |
| Demo trước giảng viên | [demo/INTERACTIVE_DEMO_GUIDE.md](demo/INTERACTIVE_DEMO_GUIDE.md) |
| Kiểm thử và ảnh bằng chứng | [tests/STAGE_4_TEST_GUIDE.md](tests/STAGE_4_TEST_GUIDE.md) |
| Phân công nhóm | [TASKS.md](TASKS.md) |
| Tài liệu Word/Excel ngoài repo | [DOCUMENTATION.md](DOCUMENTATION.md) |

## 8. Cấu trúc thư mục

```text
health-band-simulation/
├── firmware-wokwi/      # ESP32, PlatformIO, sơ đồ Wokwi
├── node-red/            # Dockerfile, flow, template Dashboard, MQTT contract
├── data/                # JSON mẫu và JSON Schema
├── demo/                # kịch bản demo và script chạy tình huống
├── tests/               # hướng dẫn test và ảnh/log bằng chứng
├── dashboard/           # ghi chú giao diện
├── ARCHITECTURE.md
├── DEPLOYMENT.md
├── TROUBLESHOOTING.md
└── README.md
```

## 9. Trạng thái hiện tại và giới hạn

MVP đã chạy được end-to-end và có Dashboard song ngữ. Tuy nhiên đây vẫn là mô phỏng môn học:

- Chưa dùng cảm biến y tế thật, pin/sạc thật hoặc người đeo thật.
- Broker hiện là public và không dùng TLS/xác thực; không gửi dữ liệu cá nhân thật.
- Chưa hỗ trợ nhiều thiết bị đồng thời hoặc kiểm thử tải/dài ngày.
- Các ngưỡng HR/SpO₂ chỉ là ngưỡng demo.

Kết quả test chi tiết nằm trong `D:\IOTs\tailieu26\TestCase_VongTayTheoDoiSucKhoe_DaChay_2026-07-28.xlsx`.
