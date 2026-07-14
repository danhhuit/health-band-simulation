# BÁO CÁO KHẢO SÁT THIẾT BỊ WEARABLE, THÔNG SỐ HR/SpO2 VÀ GIỚI HẠN MÔ PHẢNG

**Nhiệm vụ:** V01  
**Thành viên thực hiện:** Hồng Vỹ  
**Nhánh Git:** `feature/vy-health-data`  
**Ngày thực hiện:** 14/07/2026  
**Người review chéo:** Minh Thiện  

---

## 1. TUYÊN BỐ KHÔNG PHẢI THIẾT BỊ Y TẾ (MEDICAL DISCLAIMER)

> [!CAUTION]
> **TUYÊN BỐ QUAN TRỌNG:**  
> Toàn bộ hệ thống **"Vòng tay theo dõi sức khỏe mô phỏng (Đồ án 31)"** là một dự án nghiên cứu và mô phỏng công nghệ Internet of Things (IoT) trên nền tảng phần cứng ảo **Wokwi**.  
> - Các số liệu đo lường (Nhịp tim - HR, Nồng độ oxy trong máu - SpO2, Số bước chân - Steps, Té ngã - Fall) được **SINH BẰNG THUẬT TOÁN MÔ PHẢNG**, không được thu thập từ cảm biến sinh học thực tế trên cơ thể người.  
> - Hệ thống **KHÔNG PHẢI LÀ THIẾT BỊ Y TẾ**, không đạt các tiêu chuẩn y tế lâm sàng (như FDA, CE Medical, ISO 13485).  
> - Mọi thông số hiển thị và cảnh báo chỉ mang tính chất **minh họa nguyên lý kỹ thuật truyền tải dữ liệu telemetry 4 tầng**, tuyệt đối **KHÔNG** được sử dụng nhằm mục đích chẩn đoán, theo dõi lâm sàng, điều trị hay quyết định y tế cho bệnh nhân.

---

## 2. KHẢO SÁT CÔNG NGHỆ WEARABLE VÀ CẢM BIẾN THEO DÕI SỨC KHỎE

### 2.1. Tổng quan thiết bị đeo (Wearables)
Thiết bị đeo theo dõi sức khỏe (Health-monitoring Wearables) là các hệ thống tích hợp vi điều khiển (MCU), cảm biến sinh học (Biosensors), mô-đun quản lý năng lượng và giao thức kết nối không dây (BLE, Wi-Fi, MQTT over Wi-Fi/Cellular) vào các dạng vật lý như vòng đeo tay, đồng hồ thông minh (Smartwatches).

Trong thực tế, các thiết bị wearable phổ biến như Apple Watch, Fitbit, Garmin hay Xiaomi Mi Band đều cấu thành từ các khối kỹ thuật cốt lõi:
1. **Khối cảm biến (Sensing Layer):** Cảm biến quang học PPG (Photoplethysmography), cảm biến gia tốc 3 trục (3-axis Accelerometer/IMU), cảm biến nhiệt độ da.
2. **Khối xử lý biên (Edge Processing):** Vi điều khiển low-power (ví dụ: dòng ARM Cortex-M, ESP32/nRF52) thực hiện lọc nhiễu số (DSP), phát hiện nhịp tim và bước chân.
3. **Khối truyền thông (Connectivity):** Gửi dữ liệu dưới dạng chuỗi thời gian (Time-series payload) lên điện thoại (qua BLE) hoặc thẳng tới MQTT Broker/Cloud IoT.
4. **Khối năng lượng (Power Management):** Pin Lithium-Polymer kèm IC quản lý sạc và theo dõi dung lượng pin.

---

## 3. KHẢO SÁT CHUYÊN SÂU THÔNG SỐ HR VÀ SpO2

### 3.1. Nhịp tim (Heart Rate - HR)
* **Nguyên lý đo thực tế (PPG):** Sử dụng đèn LED xanh lá (Green LED, bước sóng ~530nm) chiếu vào các mao mạch dưới da. Khi tim đập, thể tích máu thay đổi làm cường độ ánh sáng phản xạ về cảm biến quang (Photodiode) thay đổi theo chu kỳ. Vi điều khiển xử lý tín hiệu xoay chiều (AC component) của PPG để đếm số nhịp trong một phút (BPM - Beats Per Minute).
* **Phạm vi và ý nghĩa lâm sàng (theo Hiệp hội Tim mạch Mỹ - AHA):**
  - **Nhịp tim nghỉ ngơi bình thường (Normal Resting HR):** `60 - 100 BPM` ở người lớn.
  - **Nhịp tim chậm (Bradycardia):** `< 60 BPM` (hoặc `< 50 BPM` trong điều kiện cảnh báo sớm/người không phải vận động viên chuyên nghiệp).
  - **Nhịp tim nhanh (Tachycardia):** `> 100 BPM` lúc nghỉ ngơi (hoặc `> 120 BPM` ở mức cảnh báo cao của hệ thống demo).
* **Phạm vi mô phỏng trong đồ án:** `40 - 200 BPM`.
  - Lý do: Bao phủ toàn bộ dải sinh học từ tim đập cực chậm (40 BPM - ca bất thường/nguy hiểm) đến tim đập cực nhanh khi vận động cường độ cao hoặc sốt cao/rối loạn nhịp (200 BPM).

### 3.2. Nồng độ Oxy bão hòa trong máu (SpO2 - Peripheral Oxygen Saturation)
* **Nguyên lý đo thực tế:** Sử dụng kết hợp 2 bước sóng ánh sáng: LED Đỏ (Red LED, ~660nm) và LED Hồng ngoại (Infrared LED, ~880nm). Hemoglobin có oxy ($\text{HbO}_2$) hấp thụ tia hồng ngoại nhiều hơn và cho ánh sáng đỏ đi qua nhiều hơn; ngược lại, Hemoglobin khử ($\text{Hb}$) hấp thụ ánh sáng đỏ nhiều hơn. Tỷ lệ hấp thụ $\frac{R}{IR}$ được tính toán để suy ra % $\text{SpO}_2$.
* **Phạm vi và ý nghĩa y sinh (theo Tổ chức Y tế Thế giới - WHO & Tiêu chuẩn hô hấp):**
  - **Bình thường (Normal SpO2):** `95% - 100%`.
  - **Thiếu oxy nhẹ đến trung bình (Mild to Moderate Hypoxemia):** `90% - 94%` (Ngưỡng cảnh báo demo của hệ thống: `< 94%`).
  - **Thiếu oxy nghiêm trọng (Severe Hypoxemia):** `< 90%` (Cần can thiệp y tế khẩn cấp).
* **Phạm vi mô phỏng trong đồ án:** `70% - 100%`.
  - Lý do: Giá trị dưới 70% trong thực tế lâm sàng cực kỳ nguy kịch hoặc do cảm biến bị sút/nhiễu tín hiệu nặng. Khoảng `70 - 100%` cho phép kiểm thử đầy đủ các tình huống suy hô hấp từ nhẹ đến nặng.

---

## 4. GIỚI HẠN CỦA MÔ HÌNH MÔ PHẢNG (SIMULATION BOUNDARIES)

Vì đồ án không sử dụng phần cứng thực tế mà triển khai trên mô phỏng **Wokwi (ESP32 ảo)**, các giới hạn kỹ thuật và mô hình hóa bao gồm:

| Tiêu chí | Thiết bị thực tế (Real Wearable) | Mô phỏng trong Đồ án 31 (Wokwi Simulation) |
| :--- | :--- | :--- |
| **Cảm biến sinh học** | Cảm biến MAX30102 / PPG Sensor thực tế, chịu ảnh hưởng bởi ánh sáng môi trường, mồ hôi, chuyển động (Motion Artifacts). | **Bộ sinh dữ liệu số (Data Generator):** Thuật toán tự sinh (Random/Walk Markov hoặc ngắt giờ timer) tạo ra số liệu sạch, không bị nhiễu vật lý ngoài ý muốn. |
| **Chuyển động & Té ngã** | Cảm biến IMU 6 trục/9 trục (MPU6050/ADXL345) phân tích gia tốc vector $a = \sqrt{a_x^2 + a_y^2 + a_z^2}$. | **Nút nhấn ảo (Buttons):** Nút `STEP` để tăng số bước chân và nút `FALL` để kích hoạt cờ té ngã (`fallDetected = true`). |
| **Năng lượng & Pin** | Pin Li-Po thực tế sụt giảm phi tuyến theo dòng tiêu thụ của MCU và module Wi-Fi. | **Biến đếm giảm dần (Simulated Battery):** Giảm theo thời gian hoặc theo số lượng bản tin gửi đi trong phạm vi `0 - 100%`. |
| **Chất lượng tín hiệu** | Phụ thuộc vào vị trí đeo tay và độ tiếp xúc da (Perfusion Index - PI). | **Trường mô phỏng (`signalQuality`):** Nhận các chuỗi định sẵn `"good"`, `"medium"`, `"poor"` để mô phỏng trạng thái tiếp xúc. |
| **Độ chính xác thời gian** | Chu kỳ gửi phụ thuộc vào ngắt phần cứng thực tế và thời gian đánh thức từ Deep Sleep. | **Timer `millis()` của ESP32 ảo:** Gửi đều đặn `1 - 2 giây/bản tin` (chu kỳ chuẩn trong đồ án là 1000ms hoặc 2000ms). |

---

## 5. NGUỒN THAM KHẢO ACADEMIC & TECHNICAL REFERENCES

1. **American Heart Association (AHA):** *All About Heart Rate (Pulse)* - Normative data for resting heart rate and tachycardia boundaries.
2. **World Health Organization (WHO):** *Pulse Oximetry Training Manual* - Clinical guidelines and hypoxemia thresholds ($\text{SpO}_2 < 94\%$).
3. **IEEE Standards Association:** *IEEE 11073 - Health informatics / Personal health device communication standards* (Data structures for vital signs).
4. **Wokwi Documentation:** *ESP32 Virtual Environment and Peripheral Simulation Guide* (https://docs.wokwi.com/).
5. **MQTT Version 3.1.1 / 5.0 Specification:** OASIS Standard for Lightweight IoT Telemetry Messaging.

---
*Tài liệu này là đầu ra bắt buộc của nhiệm vụ V01 trên nhánh `feature/vy-health-data` và dùng để chứng minh cho quá trình review chéo từ thành viên Minh Thiện.*
