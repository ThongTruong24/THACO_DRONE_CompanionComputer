# Drone Edge rules index

> **MANDATORY FIRST STEP FOR AI ASSISTANT**:
> Khi bắt đầu một phiên làm việc mới hoặc sau khi hội thoại bị nén ngữ cảnh (compaction), AI Assistant **BẮT BUỘC** phải đọc file [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) để nắm toàn bộ trạng thái hệ thống, bảng cổng mạng/UART/IPC socket và các quyết định kiến trúc bất biến.
> Nếu thực hiện tái cấu trúc lớn hoặc debug phức tạp, đọc thêm [PROJECT_CONTEXT_DEEP.md](PROJECT_CONTEXT_DEEP.md).

Project rules are organized by service boundary & system governance:

1. [Core](.agents/rules/00-core.md)
2. [Networking](.agents/rules/01-networking.md)
3. [MAVLink](.agents/rules/02-mavlink.md)
4. [Camera + RTSP](.agents/rules/03-camera-rtsp.md)
5. [Vision](.agents/rules/04-vision.md)
6. [Doctor](.agents/rules/05-doctor.md)
7. [Doc Sync & Rule Governance](.agents/rules/06-doc-sync-and-rule-governance.md)

For operator-facing behavior and architecture, start with [README.md](README.md)
and the README in the relevant `src/` module.
