# Hướng dẫn cài đặt và chạy Health Band Simulation

Tài liệu này dành cho người chưa từng chạy dự án. Làm tuần tự từ Bước 0 đến Bước 6.

## Bước 0 — Kiểm tra điều kiện máy

| Thành phần | Cách kiểm tra | Kết quả cần có |
|---|---|---|
| Docker Desktop | Mở Docker Desktop | Docker đang chạy. |
| Docker CLI | `docker --version` | Có phiên bản Docker. |
| Docker Compose | `docker compose version` | Có phiên bản Compose. |
| Node.js | `node --version` | Có Node.js để build flow. |
| PlatformIO | VS Code có extension PlatformIO IDE | Có biểu tượng PlatformIO hoặc lệnh build hoạt động. |
| Wokwi | VS Code có extension Wokwi Simulator | Có lệnh `Wokwi: Start Simulation`. |

Nếu thiếu phần nào, xem [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

## Bước 1 — Mở đúng workspace

Mở **thư mục gốc** sau trong VS Code, không mở riêng `firmware-wokwi`:

```text
D:\IOTs\projects\health-band-simulation
```

Lý do: `wokwi.toml` ở thư mục gốc trỏ đến firmware và Wokwi cần file này để biết vị trí `.bin`.

## Bước 2 — Build firmware

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --project-dir ".\firmware-wokwi"
```

Kết quả đúng:

```text
========================= [SUCCESS] Took ... =========================
```

Tệp firmware được tạo:

```text
firmware-wokwi\.pio\build\esp32dev\firmware.bin
firmware-wokwi\.pio\build\esp32dev\firmware.elf
```

Nếu đường dẫn PlatformIO khác, mở PlatformIO trong VS Code và dùng nút **Build** ở thanh trạng thái.

## Bước 3 — Khởi động Node-RED bằng Docker

```powershell
docker compose up --build -d
docker compose ps
```

Bạn cần thấy:

```text
health-band-node-red ... healthy
```

Nếu trạng thái chưa `healthy`, chờ tối đa 60 giây rồi kiểm tra log:

```powershell
docker logs --tail 100 health-band-node-red
```

## Bước 4 — Tạo Dashboard flow và deploy vào Node-RED

Dashboard flow được sinh từ `node-red/dashboard-template.html`. Chạy:

```powershell
node .\node-red\build-english-dashboard.js
curl.exe -sS -X POST -H "Content-Type: application/json" --data-binary "@node-red/data/flows.json" http://localhost:1880/flows
```

Mở hai địa chỉ sau:

| Mục đích | Địa chỉ |
|---|---|
| Sửa/quan sát flow | <http://localhost:1880> |
| Xem ứng dụng | <http://localhost:1880/dashboard/overview> |

Sau khi chỉnh `node-red/dashboard-template.html`, luôn chạy lại hai lệnh ở bước này.

## Bước 5 — Khởi động Wokwi

1. Nhấn `Ctrl+Shift+P` trong VS Code.
2. Chọn `Wokwi: Start Simulation`.
3. Đợi ESP32 xuất hiện cùng OLED, RGB LED, buzzer và nút FALL/SOS.
4. Chờ 5–10 giây để Wokwi kết nối Wi-Fi và MQTT.
5. Mở Dashboard, nhấn `Ctrl+F5` nếu chưa nhận dữ liệu.

### Dấu hiệu đang chạy đúng

- Status LED của ESP32 nháy chậm.
- OLED hiển thị `HEALTH BAND MQTT`.
- Node-RED MQTT nodes là `connected`.
- Dashboard hiện HR/SpO₂/bước/pin thay đổi.

> Wokwi Terminal có thể không hiện Serial tùy phiên bản extension. Khi đó dùng OLED, Status LED, Node-RED `connected` và Dashboard thay đổi dữ liệu làm bằng chứng.

## Bước 6 — Demo luồng hai chiều

1. Trên Dashboard chọn `High HR` hoặc `Fall`.
2. Xem Digital Twin thay đổi.
3. Xem Wokwi: OLED/màu RGB LED/còi thay đổi.
4. Quay về Overview để xem alert và timeline.
5. Chọn `Normal` để khôi phục; dùng `Reset steps` để đặt bước về 0.

Kịch bản đầy đủ: [demo/INTERACTIVE_DEMO_GUIDE.md](demo/INTERACTIVE_DEMO_GUIDE.md).

## Dừng và chạy lại

### Dừng Docker

```powershell
docker compose down
```

### Chạy lại Docker

```powershell
docker compose up -d
```

### Chạy lại toàn bộ sau khi đổi code

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --project-dir ".\firmware-wokwi"
node .\node-red\build-english-dashboard.js
curl.exe -sS -X POST -H "Content-Type: application/json" --data-binary "@node-red/data/flows.json" http://localhost:1880/flows
```

Sau đó Stop/Start Wokwi và tải lại Dashboard.
