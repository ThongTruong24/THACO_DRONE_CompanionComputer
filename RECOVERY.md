# 🆘 AI Context Recovery Guide

> Dùng tài liệu này khi AI bị quên context (mở chat mới, sau compaction, hoặc sau lỗi).

---

## ⚡ Cách Nhanh Nhất (Khuyến Nghị)

### Bước 1 — Khôi phục context tức thì
Gõ vào chat box:
```
@PROJECT_CONTEXT.md đọc lại trạng thái và tiếp tục [mô tả task]
```

Ví dụ:
```
@PROJECT_CONTEXT.md đọc lại trạng thái và tiếp tục debug mavlink-router
```
```
@PROJECT_CONTEXT.md đọc lại trạng thái và viết unit test cho cc-agent
```

> AI sẽ xác nhận đã đọc xong, sau đó bạn ra lệnh bình thường.

---

## 📋 Prompt Mẫu Theo Tình Huống

### 🔄 Khởi động phiên mới / sau compaction
```
@PROJECT_CONTEXT.md đọc lại trạng thái dự án Drone
```

### 🔬 Debug phức tạp / refactor lớn
```
@PROJECT_CONTEXT.md @PROJECT_CONTEXT_DEEP.md đọc cả hai file, sau đó /plan [mô tả task]
```

### 📦 Tiếp tục task bỏ dở
```
@PROJECT_CONTEXT.md đọc lại và liệt kê các task còn đang dang dở
```

### 🚨 Sau lỗi 'Agent execution terminated due to error'
```
@PROJECT_CONTEXT.md khôi phục context, bỏ qua lỗi trước, tiếp tục [mô tả]
```

---

## 🧠 Ghi Nhớ Vĩnh Viễn Sau Milestone Lớn

Khi hoàn thành một milestone quan trọng (hoàn thành module mới, fix bug khó, thay đổi kiến trúc), gõ:
```
/learn
```
AI sẽ ghi kiến thức quan trọng vào Knowledge Items — không mất dù mở chat mới.

---

## 📁 File Context Hệ Thống

| File | Dùng khi nào |
|---|---|
| `PROJECT_CONTEXT.md` | **Luôn dùng** — snapshot nhanh toàn bộ trạng thái |
| `PROJECT_CONTEXT_DEEP.md` | Debug nặng, refactor lớn, phân tích protocol |
| `AGENTS.md` | Xem index các rules đang áp dụng |
| `.agents/rules/` | Chi tiết từng rule theo module |

---

## ⚠️ Nguyên Nhân AI Mất Context

| Tình huống | Giải pháp |
|---|---|
| Mở chat mới trong IDE | Dùng `@PROJECT_CONTEXT.md` |
| Hội thoại quá dài (compaction) | Dùng `@PROJECT_CONTEXT.md` |
| Lỗi 'Agent execution terminated' | Dùng `@PROJECT_CONTEXT.md` + mô tả lại task |
| Đổi model (Flash → Sonnet → ...) | Context vẫn còn trong IDE; không cần khôi phục |
| Đổi workspace | Mở lại đúng workspace WSL `<workspace_dir>` |

---

## 🔧 Kiểm Tra Nhanh Hệ Thống

```bash
# Chạy toàn bộ test (phải pass 33/33)
make test

# Build
make build

# Deploy lên Pi
make deploy

# Doctor check
make doctor
```

---

*Cập nhật file này khi thêm prompt mẫu mới hoặc thay đổi workflow.*  
*Xem rule `.agents/rules/06-doc-sync-and-rule-governance.md` để biết khi nào cần cập nhật.*
