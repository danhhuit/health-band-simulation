# Ghi chú Node-RED

## Tệp đầu ra

- `flows.json`: flow đã export, không chứa thông tin bí mật.
- Ảnh flow và dashboard được lưu tại `dashboard` hoặc `tests/evidence`.

## Flow tối thiểu

```text
MQTT In -> JSON -> Validate -> Debug
```

Sau khi flow tối thiểu chạy đúng, bổ sung:

```text
Validate -> Dashboard
Validate -> Lưu lịch sử
Validate -> Luật cảnh báo
```

Không commit `flows_cred.json` hoặc mật khẩu broker.
