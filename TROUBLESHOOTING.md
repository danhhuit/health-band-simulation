# Troubleshooting — Health Band Simulation

## 1. Docker / Node-RED

### `docker compose up` lỗi hoặc Docker chưa chạy

1. Mở Docker Desktop và chờ trạng thái Running.
2. Mở PowerShell mới tại thư mục dự án.
3. Chạy lại:

```powershell
docker compose up --build -d
docker compose ps
```

### Port 1880 đã được sử dụng

Kiểm tra ứng dụng chiếm cổng:

```powershell
netstat -ano | findstr :1880
```

Dừng ứng dụng đang dùng cổng hoặc đổi phần `ports` trong `docker-compose.yml` từ `1880:1880` sang một cổng khác, ví dụ `1881:1880`. Khi đổi cổng, Dashboard là `http://localhost:1881/dashboard/overview`.

### Dashboard trắng hoặc không mở được

```powershell
docker compose ps
docker logs --tail 100 health-band-node-red
```

Nếu container healthy nhưng giao diện cũ, chạy lại:

```powershell
node .\node-red\build-english-dashboard.js
curl.exe -sS -X POST -H "Content-Type: application/json" --data-binary "@node-red/data/flows.json" http://localhost:1880/flows
```

Sau đó nhấn `Ctrl+F5` trong trình duyệt.

## 2. PlatformIO

### Không nhận lệnh `pio`

Không cần cài riêng `pio` vào PATH. Dùng đường dẫn PlatformIO mặc định:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --project-dir ".\firmware-wokwi"
```

Hoặc mở PlatformIO trong VS Code và chọn **Build**.

### Build lỗi thư viện

- Kiểm tra Internet.
- Mở `firmware-wokwi/platformio.ini`, không tự đổi `board` hoặc `framework`.
- Chạy lại build; PlatformIO sẽ tải dependency ở lần đầu.

## 3. Wokwi

### Không có Wokwi Terminal / Serial

Đây có thể là giới hạn của extension hoặc cách Wokwi chạy simulator. Không dùng Terminal là tiêu chí duy nhất để kết luận lỗi. Hãy kiểm tra theo thứ tự:

1. Simulator đang chạy, không màn hình đen.
2. Status LED nháy.
3. OLED hiển thị `HEALTH BAND MQTT`.
4. Node-RED MQTT node là `connected`.
5. Dashboard thay đổi chỉ số mỗi vài giây.

### Màn hình Wokwi đen hoặc báo `Cannot read properties of null (reading 'parts')`

1. Stop Simulation.
2. Kiểm tra file `diagram.json` ở thư mục gốc là JSON hợp lệ.
3. Không mở nhầm `diagram.json` trong một thư mục khác.
4. Build firmware lại.
5. Start Simulation lại.

### Firmware cũ vẫn chạy

Wokwi không tự nạp lại file `.bin` khi simulator đang chạy. Thực hiện:

1. Stop Simulation.
2. Build PlatformIO thành công.
3. Start Simulation.

## 4. MQTT không connected

### Node-RED hiển thị `disconnected`

- Kiểm tra Internet.
- Broker demo là `broker.emqx.io`, port `1883`.
- Chờ 10–20 giây; Node-RED có thể tự reconnect.
- Xem log container:

```powershell
docker logs --tail 100 health-band-node-red
```

### Dashboard không có telemetry

1. Đảm bảo Wokwi đang chạy.
2. Đợi 5–10 giây.
3. Tải lại Dashboard bằng `Ctrl+F5`.
4. Trong Node-RED Editor, kiểm tra node `Telemetry` là `connected`.
5. Kiểm tra topic prefix không bị thay đổi: `iot31/nhom-thanh-danh/health-band`.

## 5. Lệnh Dashboard không làm Wokwi đổi mode

1. Đảm bảo Wokwi đã kết nối MQTT trước khi nhấn nút scenario.
2. Kiểm tra node `Publish Command` là `connected`.
3. Chọn lại `Normal`, chờ một telemetry cycle rồi thử scenario khác.
4. Stop/Start Wokwi nếu firmware đang chạy phiên cũ.

## 6. Tiếng Việt/English không đổi

1. Tải lại Dashboard với `Ctrl+F5`.
2. Nếu vừa sửa template, chạy lại build/deploy flow ở phần Docker/Node-RED.
3. Xóa dữ liệu site `localhost:1880` trong trình duyệt nếu cần đặt lại lựa chọn ngôn ngữ.

## 7. Khi demo không có Internet

Broker `broker.emqx.io` là public nên hệ thống cần Internet. Chuẩn bị trước:

- ảnh bằng chứng trong `tests/evidence/`;
- video demo dự phòng;
- Word/Excel báo cáo;
- ảnh kiến trúc 4 tầng.

Không cố thay broker ngay trước giờ demo nếu cả nhóm chưa kiểm tra lại flow và firmware.
