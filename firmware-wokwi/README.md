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

- Publish mỗi 2 giây vào topic `telemetry`.
- `timestamp` lấy từ `millis()`.
- `seq` tăng sau mỗi lần publish.
- `steps` tăng ngẫu nhiên khi không ở mode fall.
- Pin giảm chậm; mode `low_battery` giới hạn pin trong khoảng 12–19%.

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

## 7. Không nên thay đổi tùy tiện

Không đổi `DEVICE_ID`, MQTT broker, prefix topic hoặc tên trường JSON mà không cập nhật cùng lúc:

1. `node-red/mqtt-topics.md`;
2. các JSON Schema trong `data/`;
3. Node-RED flow;
4. Dashboard/test case.
