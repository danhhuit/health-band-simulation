# Ghi chú Node-RED

## Cách chạy

Node-RED chạy trong Docker qua `docker-compose.yml`:

```powershell
docker compose up --build -d
```

| Thành phần | Giá trị |
|---|---|
| Container | `health-band-node-red` |
| Editor | <http://localhost:1880> |
| Dashboard | <http://localhost:1880/dashboard/overview> |
| Data volume | `node-red/data` được mount vào `/data` |

## Flow hiện tại

Flow được sinh bằng script:

```powershell
node .\node-red\build-english-dashboard.js
```

Script đọc `dashboard-template.html`, cập nhật `node-red/data/flows.json` và xuất bản sao `health-band-flow.json`.

Sau đó deploy flow:

```powershell
curl.exe -sS -X POST -H "Content-Type: application/json" --data-binary "@node-red/data/flows.json" http://localhost:1880/flows
```

## Các node chính

| Nhóm | Node/logic |
|---|---|
| Nhận dữ liệu | Telemetry, Device Events, Device Status, Alert Events. |
| Xử lý | Parse JSON, Record Last Seen, Evaluate Alerts, Detect Offline Device. |
| Điều khiển | Prepare Command, Publish Command. |
| Ứng dụng | `English Health Dashboard` (`ui-template`). |

## Lưu ý dữ liệu

- Node-RED hiện parse JSON để tránh flow crash; schema validation đầy đủ là hướng nâng cấp.
- Offline được đánh giá sau 8 giây không nhận telemetry.
- Broker public không có TLS/xác thực trong MVP. Không dùng dữ liệu thật.
- `flows_cred.json` không được commit nếu chứa thông tin bí mật.

## Kiểm tra lỗi

```powershell
docker logs --tail 100 health-band-node-red
docker exec health-band-node-red node -e "const net=require('net');const s=net.connect(1883,'broker.emqx.io');s.on('connect',()=>{console.log('MQTT OK');s.end()});s.on('error',e=>{console.error(e.message);process.exit(1)})"
```
