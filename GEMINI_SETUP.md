# Tích hợp Gemini cho Health Band

Gemini là tính năng **tùy chọn**. Khi chưa có khóa, Dashboard vẫn hoạt động và tự dùng khuyến cáo cục bộ.

## Cách tích hợp an toàn

Dashboard không gọi Gemini trực tiếp. Trình duyệt chỉ gửi dữ liệu tổng hợp, đã loại tên, `deviceId`, GPS và lịch sử thô đến:

```text
POST /health-band/api/ai-recommendation
```

Node-RED mới giữ khóa bí mật và gọi `generateContent` với header `x-goog-api-key`. Kết quả chỉ là gợi ý sức khỏe chung, không chẩn đoán.

## Thiết lập

1. Tạo API key trong Google AI Studio.
2. Sao chép file mẫu:

```powershell
Copy-Item .env.example .env
```

3. Mở `.env`, thay giá trị:

```dotenv
GEMINI_API_KEY=YOUR_REAL_KEY
GEMINI_MODEL=gemini-3.5-flash
NODE_RED_CREDENTIAL_SECRET=YOUR_LONG_RANDOM_SECRET
```

4. Không commit `.env`. File này đã được chặn bởi `.gitignore`.
5. Khởi động lại:

```powershell
node .\node-red\build-english-dashboard.js
docker compose up -d --build --force-recreate
```

6. Mở Dashboard → **Overview** → **Gemini wellness assistant** → **Ask Gemini**.

## Dữ liệu gửi đi

- Đối tượng và giới tính đã chọn.
- Tỷ lệ thời gian đeo.
- Trung bình HR, SpO₂, huyết áp ước tính.
- Bước chân, số giờ ngủ, mục tiêu ngủ và mã cảnh báo.

Không gửi tên, email, số điện thoại, tọa độ GPS, `deviceId` hoặc toàn bộ mẫu telemetry.

## Kiểm tra API

Khi chưa cấu hình khóa, API phải trả thông báo `GEMINI_API_KEY is not configured`. Đây là hành vi đúng.

Tài liệu chính thức: [Gemini API reference](https://ai.google.dev/api) và [Using Gemini API keys](https://ai.google.dev/gemini-api/docs/generate-content/api-key).

> Google đang chuyển dần sang authorization key. Hãy dùng loại key mới do Google AI Studio tạo và không đặt khóa trong mã nguồn hoặc JavaScript phía trình duyệt.
