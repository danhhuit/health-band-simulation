# Quy ước phối hợp

## Nhánh

- Thành Danh: `feature/danh-integration-mqtt`
- Hồng Vỹ: `feature/vy-health-data`
- Minh Thiện: `feature/thien-wokwi-controls`
- Lê Hậu: `feature/hau-node-red-dashboard`

Không lập trình trực tiếp trên `main` sau commit khởi tạo.

## Commit

Sử dụng tiền tố:

- `feat:` chức năng mới.
- `fix:` sửa lỗi.
- `docs:` cập nhật tài liệu.
- `test:` thêm hoặc cập nhật kiểm thử.
- `chore:` cấu hình và công việc bảo trì.

Ví dụ:

```text
feat: publish telemetry qua MQTT
fix: chong doi nut FALL
docs: bo sung schema JSON
test: them ca canh bao SpO2 thap
```

## Review

1. Pull nhánh `main` mới nhất trước khi bắt đầu.
2. Chỉ commit các tệp thuộc nhiệm vụ của mình.
3. Push nhánh cá nhân và gửi cho người review chéo.
4. Không merge nếu chưa có bằng chứng chạy được.
5. Không commit mật khẩu, token hoặc thông tin cá nhân.

