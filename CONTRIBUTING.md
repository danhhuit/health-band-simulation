# Quy ước phối hợp nhóm

## Nhánh làm việc

| Thành viên | Nhánh đề xuất |
|---|---|
| Thành Danh | `feature/danh-integration-mqtt` |
| Hồng Vỹ | `feature/vy-health-data` |
| Minh Thiện | `feature/thien-wokwi-controls` |
| Lê Hậu | `feature/hau-node-red-dashboard` |

Không sửa trực tiếp `main` khi làm thay đổi lớn. Trước khi bắt đầu, cập nhật nhánh hiện tại từ `main`.

## Commit

Sử dụng tiền tố rõ ràng:

```text
feat: add fall event acknowledgement
fix: reconnect mqtt after wifi loss
docs: update deployment guide
test: add dashboard language test
chore: update docker image
```

## Quy trình thay đổi an toàn

1. Chỉ sửa các file thuộc phần việc được giao.
2. Nếu đổi topic hoặc payload, cập nhật **cùng pull request**: firmware, `node-red/mqtt-topics.md`, JSON schema và test.
3. Build firmware hoặc kiểm tra Node-RED tương ứng.
4. Chụp ảnh/log làm bằng chứng nếu thay đổi ảnh hưởng demo.
5. Không commit mật khẩu, token, dữ liệu sức khỏe thật, `node_modules` hoặc `.pio` build artifact.

## Review tối thiểu

Người review cần kiểm tra:

- Tên topic/payload có đúng hợp đồng MQTT không.
- Dashboard không làm mất luồng command/event.
- Wokwi build được.
- Tài liệu đã cập nhật nếu thay đổi ảnh hưởng cách cài hoặc demo.
