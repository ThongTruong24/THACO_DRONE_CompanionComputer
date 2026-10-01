# AI Vision Processor (YOLO / Computer Vision Pipeline)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](README.md)
[![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi%205%20%7C%20ARM64-orange.svg)](Dockerfile)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

Phân hệ xử lý thị giác máy tính và nhận diện mục tiêu AI thời gian thực trên Raspberry Pi 5. Thu nhận luồng video RTSP gốc từ `camera-stream-controller` (`rtsp://127.0.0.1:8554/camera`), chạy mô hình YOLOv8 phát hiện vật thể, gắn nhãn (bounding box & confidence), và phát lại luồng video đã chú thích lên `rtsp://<pi>:8554/yolo`.

---

## Mục lục (Table of Contents)
- [Tính năng chính](#tính-năng-chính)
- [Sơ đồ Kiến trúc & Luồng hoạt động](#sơ-đồ-kiến-trúc--luồng-hoạt-động)
- [Cài đặt & Biên dịch](#cài-đặt--biên-dịch)
- [Hướng dẫn Sử dụng](#hướng-dẫn-sử-dụng)
- [Đóng góp (Contributing)](#đóng-góp-contributing)
- [Giấy phép (License)](#giấy-phép-license)

---

## Tính năng chính
- **Không xâm lấn luồng FPV gốc**: Luồng `/camera` hoàn toàn độc lập; nếu AI Vision gặp sự cố hoặc quá tải CPU, luồng camera FPV điều khiển máy bay vẫn hoạt động trơn tru.
- **Tối ưu suy luận trên ARM64**: Tối ưu hóa mô hình mạng nơ-ron nhẹ (YOLOv8 nano) cho vi xử lý Cortex-A76 trên Raspberry Pi 5.
- **Compose Profile Cô Lập**: Chạy trong profile `vision`, chỉ được bật khi người vận hành kích hoạt hoặc phục vụ bài toán cụ thể.

---

## Sơ đồ Kiến trúc & Luồng hoạt động

### Kiến trúc Phân hệ (System Architecture)
```mermaid
graph TD
    Cam[RTSP Server /camera] -->|RTSP Ingest| Ingest[yolo_streamer Ingest]
    Ingest --> Engine[YOLOv8 Inference Engine]
    Weights[(yolo26n.pt Weights)] --> Engine
    Engine --> Draw[Annotator / Bounding Boxes]
    Draw --> Sink[GStreamer RTSP Sink]
    Sink --> Output[RTSP Server /yolo]
```

### Luồng Tuần tự (Sequence Diagram)
> Mã nguồn chi tiết PlantUML: xem tại [docs/sequence.puml](docs/sequence.puml) và [docs/system.puml](docs/system.puml).

```mermaid
sequenceDiagram
    autonumber
    participant Raw as MediaMTX (:8554/camera)
    participant Vision as yolo_streamer
    participant Model as YOLOv8
    participant Out as MediaMTX (:8554/yolo)

    Vision->>Raw: Kết nối luồng RTSP raw
    Raw-->>Vision: H.264 Video stream
    loop Từng khung hình
        Vision->>Model: Dự đoán bounding boxes
        Model-->>Vision: Nhãn + Điểm tin cậy (Confidence)
        Vision->>Vision: Vẽ khung nhãn lên frame
        Vision->>Out: Đẩy luồng RTSP annotated (:8554/yolo)
    end
```

---

## Cài đặt & Biên dịch (Installation & Build)

```bash
# Build Docker ARM64
make build-vision

# Triển khai lên Raspberry Pi
make deploy-vision
```

---

## Hướng dẫn Sử dụng (Usage)

```bash
# Bật dịch vụ Vision trên Pi
make start-vision

# Xem log suy luận
make logs-vision

# Xem video phát hiện vật thể từ máy tính
ffplay -rtsp_transport tcp rtsp://thong.local:8554/yolo

# Tắt dịch vụ Vision khi không dùng
make stop-vision
```

---

## Đóng góp (Contributing)
Mọi đề xuất đóng góp vui lòng tuân thủ quy tắc quản trị tài liệu tại [`.agents/rules/06-doc-sync-and-rule-governance.md`](../../.agents/rules/06-doc-sync-and-rule-governance.md).

---

## Giấy phép (License)
Dự án được phân phối dưới giấy phép [MIT License](https://opensource.org/licenses/MIT).

## GitHub / GHCR deployment contract

Source/config templates are pulled from GitHub; images are pulled from GHCR.
Set the actual lowercase `IMAGE_NAMESPACE` in private `.env`; choose a commit
SHA or development `IMAGE_TAG`. All internal runtime images target linux/arm64;
Pi Compose has no build definitions. See [deployment guide](../../deploy/README.md).

WSL: `make publish-vision` after commit/push. Pi: `bash deploy/update.sh --service drone-vision`.

`publish-vision` first publishes `drone-vision-base:FULL_GIT_SHA`, then passes its
GHCR reference as `VISION_BASE_IMAGE`. Apt OpenCV/GStreamer stays in use. The model
`yolo26n.pt` is tracked and mounted ro; private YAML/model overrides are supported.
