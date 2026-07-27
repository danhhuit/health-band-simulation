# Quản lý tài liệu dự án

## 1. Phân biệt mã nguồn và tài liệu nộp bài

| Loại | Vị trí | Mục đích |
|---|---|---|
| Mã nguồn, flow, JSON schema, test guide | Repository `health-band-simulation` | Có thể tái chạy và quản lý bằng Git. |
| Word, Excel, ảnh báo cáo, biểu mẫu nộp bài | `D:\IOTs\tailieu26` | Tài liệu học phần và file nộp. |

Không đặt file Word/Excel lớn vào repository trừ khi cả nhóm quyết định quản lý chúng bằng Git LFS.

## 2. Tài liệu hiện có trong `D:\IOTs\tailieu26`

- `TestCase_VongTayTheoDoiSucKhoe.xlsx`: bộ test case gốc.
- `TestCase_VongTayTheoDoiSucKhoe_DaChay_2026-07-28.xlsx`: kết quả đánh giá 272 test case.
- `BaoCao_TongQuan_UngDung_VongTaySucKhoe_2026-07-28.docx`: giải thích ứng dụng, kiến trúc và kết quả kiểm thử.

## 3. Quy ước tên file

```text
YYYY-MM-DD-loai-noi-dung-phien-ban.ext
```

Ví dụ:

```text
2026-07-28-bao-cao-do-an-v1.docx
2026-07-28-kich-ban-demo-v1.md
2026-07-28-anh-fall-dashboard.png
```

## 4. Quy tắc lưu bằng chứng

- Ảnh và log kỹ thuật lưu trong `tests/evidence/` của repository.
- Tài liệu tổng hợp/chọn lọc để nộp lưu trong `D:\IOTs\tailieu26`.
- Mỗi ảnh nên có tên thể hiện tình huống, ví dụ `TC06-fall-dashboard.png`.
- Không chỉnh sửa số liệu hoặc ghép nhiều tình huống khiến bằng chứng khó kiểm tra.

## 5. Trước khi nộp

1. Mở Word và Excel trên một máy khác nếu có thể.
2. Kiểm tra link/ảnh trong tài liệu không bị mất.
3. Chạy lại Dashboard ở chế độ `Normal`, `High HR`, `Low SpO₂`, `Fall`, `Low battery`.
4. Chuẩn bị ít nhất một video dự phòng nếu Internet hoặc broker public gặp lỗi.
