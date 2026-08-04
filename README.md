# Health Band Simulation — Đề tài 31

Mô phỏng vòng tay theo dõi sức khỏe cá nhân cho môn IoTs. Dự án sử dụng **ESP32 trên Wokwi**, **MQTT**, **Node-RED Dashboard 2.0** và không yêu cầu mua linh kiện thật.

> Lưu ý: dữ liệu nhịp tim, SpO₂ và cảnh báo trong dự án là dữ liệu mô phỏng phục vụ học tập; ứng dụng không phải thiết bị y tế và không dùng để chẩn đoán.

## 1. Dự án thể hiện điều gì?

Hệ thống mô phỏng một vòng tay có thể:

- Đọc HR/SpO₂, MPU6050, DS18B20, LDR và BMP180; hỗ trợ GPS NMEA giả lập và phản hồi rung mô phỏng.
- Gửi dữ liệu qua Wi-Fi/MQTT mỗi **2 giây** ở Live mode hoặc **8 giây** ở Eco mode.
- Hiển thị OLED, LED RGB, còi và nút FALL/SOS ngay trên Wokwi.
- Phân tích dữ liệu bằng Node-RED, tạo cảnh báo và phát hiện offline sau 8 giây không nhận telemetry.
- Hiển thị Dashboard tương tác, mô hình vòng tay số, kiến trúc IoT 4 tầng, Hồ sơ sức khỏe, Smart Coach và hướng dẫn trong ứng dụng.

## 2. Kiến trúc 4 tầng

```text
Tầng 4 — Ứng dụng: Dashboard, Digital Twin, Health Profile, Smart Coach, App guide
                         ↑↓ MQTT command / telemetry / alert / event
Tầng 3 — Xử lý: Node-RED parse JSON, cảnh báo, offline, timeline
                         ↑↓ MQTT
Tầng 2 — Mạng: Wi-Fi Wokwi-GUEST + broker.emqx.io:1883
                         ↑↓ MQTT
Tầng 1 — Thiết bị: ESP32 + HR/SpO₂ + MPU6050 + DS18B20 + LDR + BMP180 + GPS/haptic
```

Giải thích chi tiết: [ARCHITECTURE.md](ARCHITECTURE.md).

## 3. Cài đặt nhanh (lần đầu)

### Điều kiện cần

- Windows 10/11, Internet ổn định.
- [Docker Desktop](https://www.docker.com/products/docker-desktop/) đang chạy.
- VS Code, extension **PlatformIO IDE** và **Wokwi Simulator**.
- Tài khoản/giấy phép Wokwi hợp lệ nếu extension yêu cầu.
- Cổng `1880` chưa bị ứng dụng khác dùng.

### Kiểm tra nhanh trước khi demo

Sau khi đã cài đặt, có thể chạy một lệnh tuần tự để chuẩn bị toàn bộ môi trường:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\pre-demo-check.ps1 -Full
```

Không chạy đồng thời lệnh này với Build PlatformIO khác, vì PlatformIO sẽ cùng sử dụng tệp tạm trong `firmware-wokwi\.pio`.

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

Nếu Wokwi Terminal trống, chạy `Ctrl+Shift+P` → `Tasks: Run Task` → `Health Band: Live MQTT Log`. Chi tiết UART và cách sửa `Disconnected` nằm trong [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

## 4. Dùng Dashboard

Dashboard có 6 trang, chuyển bằng các tab ở đầu trang:

| Trang | Dùng để làm gì? |
|---|---|
| **Overview** | Xem chỉ số chính, cảm biến mở rộng, biểu đồ, điều khiển mô phỏng và cảnh báo. |
| **Digital twin** | Quan sát chiếc vòng tay số hiển thị cùng dữ liệu với ESP32 Wokwi. |
| **Smart Coach** | Điểm sức khỏe giải thích được, mục tiêu bước tùy chỉnh, xu hướng, chất lượng dữ liệu, hồ sơ demo, Eco mode và demo tự động. |
| **Health profile** | Nhập tuổi, chiều cao, cân nặng, giới tính và mức vận động; xem BMI cùng mục tiêu ngủ theo tuổi. |
| **App guide** | Xem giải thích từng trang và chức năng ngay trong ứng dụng. |
| **Activity log** | Tạo/tải báo cáo, xuất/phục hồi bản sao lưu và xem hoạt động gần đây. |

Ở góc đầu Dashboard có hai lựa chọn ngôn ngữ: **🇻🇳 Tiếng Việt** và **🇺🇸 English**. Lựa chọn được lưu sau khi tải lại trang.

Header còn có:

- **Chuông thông báo**: hiển thị badge số chưa đọc và tập hợp cảnh báo, thay đổi kết nối, event thiết bị, emergency và lỗi chất lượng dữ liệu. Có `Mark all as read` và `Clear`.
- **Theme sáng/tối**: bấm biểu tượng mặt trăng để bật Dark mode; bấm mặt trời để trở lại Light mode. Theme được lưu trong `localStorage` và giữ nguyên sau khi reload.

### Chức năng thông minh F1–F12

| Mã | Chức năng | Cách chứng minh khi demo |
|---|---|---|
| F1 | Điểm sức khỏe 0–100 có giải thích | Mở Smart Coach và chỉ bốn nhóm điểm: vitals, safety, activity, device. |
| F2 | Mục tiêu bước chân do người dùng tự đặt | Nhập mục tiêu 100–50.000 bước, bấm `Save goal`, reload trang và kiểm tra mục tiêu vẫn còn. |
| F3 | Phân tích xu hướng 10 mẫu gần nhất | Quan sát trạng thái tăng/giảm/ổn định của HR, SpO₂ và bước. |
| F4 | Xác nhận bất thường sau 3 mẫu liên tiếp | Chọn High HR/Low HR/Low SpO₂; bộ đếm chạy 1/3 → 3/3 rồi mới báo. |
| F5 | Xác nhận đã xem cảnh báo | Ở Overview bấm `Acknowledge`; Dashboard gửi lệnh MQTT `ackAlert`. |
| F6 | Quy trình té ngã có đếm ngược 10 giây | Chọn Fall, sau đó hủy bằng `I am safe` hoặc chờ gửi thông báo mô phỏng. |
| F7 | Giám sát chất lượng dữ liệu | Kiểm tra trường bắt buộc, range, `seq`, timestamp và signal quality. |
| F8 | Hồ sơ Student / Older adult / Athlete / Child và giới tính | Chọn profile/giới tính để đổi ngưỡng demo, mục tiêu và phân nhóm báo cáo. |
| F9 | Xuất báo cáo phiên JSON | Bấm `Export report` để tải dữ liệu, điểm, cảnh báo, profile và lịch sử. |
| F10 | Live/Eco thích ứng | Live gửi mỗi 2 giây; Eco gửi mỗi 8 giây; pin ≤20% tự chuyển Eco trên thiết bị. |
| F11 | Gợi ý sức khỏe không chẩn đoán | Smart Coach tạo gợi ý hành động kèm lưu ý không thay thế tư vấn y tế. |
| F12 | So sánh Normal với Current | Bảng hiển thị baseline và độ chênh HR, SpO₂, steps, battery. |

Hai tiện ích bổ sung:

- **Guided smart demo** tự chạy Digital Twin → Normal → High HR → Low SpO₂ → Sleep → Fall → Low battery → Recovery.
- **Clear session** xóa lịch sử, cảnh báo, mục tiêu và profile lưu cục bộ để bắt đầu một phiên mới.

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
| `command` | Dashboard/Node-RED → ESP32 | `setMode`, `resetSteps`, `setProfile`, `setPowerMode`, `ackAlert`, `emergencyAction`. |
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

Ứng dụng đã chạy end-to-end, có Dashboard song ngữ, Smart Coach và báo cáo tự động. Firmware v0.6.0 và smoke test hiện đều build/chạy thành công. Tuy nhiên đây vẫn là mô phỏng môn học:

- Đã có bộ cảm biến mở rộng trong Wokwi, nhưng chưa dùng MAX30102, GPS Neo-6M hoặc motor rung vật lý; mọi số đo vẫn chỉ phục vụ mô phỏng học tập.
- Broker hiện là public và không dùng TLS/xác thực; không gửi dữ liệu cá nhân thật.
- Chưa hỗ trợ nhiều thiết bị đồng thời hoặc kiểm thử tải/dài ngày.
- Các ngưỡng HR/SpO₂ chỉ là ngưỡng demo.

Kết quả test chi tiết nằm trong `D:\IOTs\tailieu26\TestCase_VongTayTheoDoiSucKhoe_DaChay_2026-07-28.xlsx`.

## 10. Cập nhật v0.6.0

- Thêm profile **Child** và giới tính **Male/Female** cho mọi profile.
- Thêm huyết áp ước tính, trạng thái đeo/không đeo và xử lý mẫu thiếu.
- Auto Sleep Tracking bằng sensor fusion và lịch ngủ cá nhân.
- Báo cáo hourly/daily/monthly, tạo ngay hoặc tải JSON.
- Checkpoint ba lớp: ESP32 NVS, hai browser snapshot và Node-RED/Docker volume.
- `Demo thông minh tự động` chạy độc lập và tự chuyển trang/tình huống theo kịch bản.
- Gemini tùy chọn qua API Node-RED; API key không xuất hiện trong trình duyệt.
- Ngày/giờ toàn cục hiển thị ở header.

Đọc thêm: [FEATURES_V06.md](FEATURES_V06.md), [GEMINI_SETUP.md](GEMINI_SETUP.md), [DEVICE_COMPARISON.md](DEVICE_COMPARISON.md).

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\smoke-test.ps1
```

Kết quả gần nhất: smoke test đạt toàn bộ, firmware `0.6.0` build thành công và Node-RED có 25 node.
