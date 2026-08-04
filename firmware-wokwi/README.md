# Firmware ESP32 trên Wokwi

Thư mục này chứa firmware PlatformIO cho thiết bị mô phỏng `health-band-01`.

## 1. File quan trọng

| File | Vai trò |
|---|---|
| `src/main.cpp` | Firmware ESP32: Wi-Fi, MQTT, dữ liệu mô phỏng, OLED, LED, còi, FALL/SOS. |
| `platformio.ini` | Board `esp32dev`, framework Arduino, thư viện cần dùng. |
| `../diagram.json` | Sơ đồ phần cứng Wokwi ở thư mục gốc. |
| `../wokwi.toml` | Cho Wokwi biết vị trí firmware `.bin` và `.elf`. |

## 2. Build

Chạy từ thư mục gốc repository:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --project-dir ".\firmware-wokwi"
```

Kết quả build nằm tại:

```text
.pio\build\esp32dev\firmware.bin
.pio\build\esp32dev\firmware.elf
```

Sau khi build, Stop/Start Wokwi để nạp firmware mới.

## 3. Phần cứng mô phỏng và chân ESP32

| Thiết bị | Chân ESP32 | Mục đích |
|---|---:|---|
| Status LED | GPIO 2 | Nháy chậm khi MQTT connected, nhanh khi mất kết nối. |
| RGB LED R/G/B | GPIO 25 / 26 / 27 | Xanh normal, vàng warning, đỏ fall, xanh dương offline. |
| Buzzer | GPIO 18 | Âm báo khi đổi scenario; mạnh hơn khi fall. |
| FALL/SOS button | GPIO 19 | Nút `INPUT_PULLUP`, nhấn để tạo fall cục bộ. |
| OLED SDA/SCL | GPIO 21 / 22 | Hiển thị HR, SpO₂, bước, pin, mode. |
| MPU6050 SDA/SCL | GPIO 21 / 22 | Đọc gia tốc ba trục, phát hiện chuyển động, bước và té ngã. |
| HR sensor input | GPIO 34 (ADC) | Thanh trượt Wokwi mô phỏng tín hiệu quang học của MAX30102, ánh xạ 40–180 BPM. |
| SpO₂ sensor input | GPIO 35 (ADC) | Thanh trượt Wokwi mô phỏng tín hiệu quang học, ánh xạ 80–100%. |
| DS18B20 DQ | GPIO 5 | Đọc nhiệt độ cơ thể mô phỏng bằng giao tiếp 1-Wire. |
| LDR AO | GPIO 33 (ADC) | Đo lux và tự động chuyển độ tương phản OLED giữa `dim`/`bright`. |
| BMP180 SDA/SCL | GPIO 21 / 22 | Đọc nhiệt độ môi trường, áp suất và ước lượng độ cao. |
| GPS NMEA | Wokwi UART2/Terminal | Parse câu `$GPRMC`; firmware có tọa độ mẫu và nhận NMEA mới từ Terminal. |
| Haptic output | GPIO 23 | Đèn xanh mô phỏng motor rung vì Wokwi chưa có linh kiện vibration motor. |
| Debug UART2 RX/TX | GPIO 16 / 17 | Đưa log firmware ra Wokwi Serial Monitor ở 115200 baud. |

## 4. Kết nối

```text
Wi-Fi SSID: Wokwi-GUEST
Wi-Fi password: (rỗng)
MQTT host: broker.emqx.io
MQTT port: 1883
Device ID: health-band-01
```

Firmware dùng MQTT Last Will trên topic `status`; khi mất kết nối đột ngột broker sẽ publish trạng thái offline retained.

## 5. Chức năng firmware

### Telemetry

- Publish mỗi 2 giây ở Live mode hoặc mỗi 8 giây ở Eco mode.
- `timestamp` lấy từ `millis()`.
- `seq` tăng sau mỗi lần publish.
- Ở mode `normal`, HR và SpO₂ được đọc từ hai đầu vào cảm biến tương tác; `steps` và `fallDetected` được suy ra từ MPU6050.
- Khi chọn scenario cảnh báo, firmware chuyển `dataSource` thành `scenario_override` và tạm ghi đè chỉ số để tạo tình huống lặp lại được.
- Pin giảm chậm; mode `low_battery` giới hạn pin trong khoảng 12–19%.
- Telemetry công bố thêm `dataSource`, `sensorHealth`, ADC thô và dữ liệu gia tốc trong object `motion`.
- Telemetry mở rộng có `bodyTemperatureC`, `ambientLightLux`, `displayMode`, `environment`, `location` và `hapticActive`.
- Nhiệt độ từ 38°C trong 3 mẫu liên tiếp được Node-RED/Dashboard xác nhận là cảnh báo mô phỏng.
- Pin ≤20% tự kích hoạt chu kỳ Eco 8 giây để minh họa tiết kiệm năng lượng.

### Mode nhận từ Dashboard

| Giá trị command `setMode` | Phản ứng |
|---|---|
| `normal` | Chỉ số bình thường, LED xanh. |
| `high_hr` | HR cao, LED vàng. |
| `low_hr` | HR thấp, LED vàng. |
| `low_spo2` | SpO₂ thấp, LED vàng. |
| `fall` | `fallDetected=true`, LED đỏ, còi cảnh báo. |
| `low_battery` | Pin yếu, LED vàng. |

Lệnh `resetSteps` đặt số bước về 0.

### Lệnh thông minh nhận từ Dashboard

| Command | Value | Phản ứng |
|---|---|---|
| `setProfile` | `student`, `older_adult`, `athlete`, `child` | Lưu profile demo và công bố lại status. |
| `setGender` | `male`, `female` | Lưu giới tính metadata cho dashboard/báo cáo. |
| `setWearState` | `auto`, `worn`, `off_wrist` | Chọn tự nhận biết hoặc ép trạng thái đeo. |
| `setPowerMode` | `normal`, `eco` | Đổi chu kỳ telemetry 2 giây/8 giây. |
| `ackAlert` | Mã alert | Gửi event xác nhận người trình bày đã xem cảnh báo. |
| `emergencyAction` | `cancel`, `send` | Hủy té ngã về Normal hoặc ghi nhận đã gửi thông báo mô phỏng. |

### Event trả về

Firmware publish topic `event` với các loại:

- `DEVICE_STARTED`
- `COMMAND_ACCEPTED`
- `COMMAND_REJECTED`
- `LOCAL_FALL_BUTTON`

## 6. Quan sát khi chạy Wokwi

Kết quả đúng có thể kiểm tra bằng OLED, LED và Dashboard. Serial log thường có các nhãn:

```text
[WiFi] connected
[MQTT] Connected and subscribed
[TELEMETRY] {...}
[COMMAND] {...}
[EVENT] {...}
```

Nếu Wokwi Terminal không hiển thị, xem [../TROUBLESHOOTING.md](../TROUBLESHOOTING.md).

Có hai task VS Code dự phòng:

- `Health Band: Wokwi Serial Monitor`: đọc UART2 qua `rfc2217://localhost:4000`.
- `Health Band: Live MQTT Log`: in status/telemetry/event trực tiếp từ broker, không phụ thuộc webview Wokwi.

## 7. Không nên thay đổi tùy tiện

Không đổi `DEVICE_ID`, MQTT broker, prefix topic hoặc tên trường JSON mà không cập nhật cùng lúc:

1. `node-red/mqtt-topics.md`;
2. các JSON Schema trong `data/`;
3. Node-RED flow;
4. Dashboard/test case.
