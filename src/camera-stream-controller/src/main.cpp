#include "camera_interface.hpp"
#include "safe_ring_buffer.hpp"
#include "neon_processor.hpp"
#include "standby_generator.hpp"
#include "realsense_camera.hpp"
#include "v4l2_camera.hpp"
#include "gst_stream_sink.hpp"
#include "config_manager.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <mutex>
#include <csignal>
#include <chrono>
#include <sstream>
#include <cstdio>
#include <unistd.h>

using namespace drone;

static std::atomic<bool> g_running(true);
static std::atomic<bool> g_reset_requested(false);

void signalHandler(int sig) {
    (void)sig;
    std::cout << "\n[StreamerEngine] Nhận tín hiệu dừng, tiến hành giải phóng tài nguyên...\n";
    g_running = false;
}

static void exportCameraStats(int bitrate_kbps, double fps, uint64_t dropped, bool adaptive,
                              bool connected, bool is_standby, int rotation,
                              const std::string& camera_type, const std::string& device_name) {
    const std::string tmp_path = "/run/drone/camera_stats.json.tmp";
    const std::string final_path = "/run/drone/camera_stats.json";

    std::ostringstream ss;
    ss << "{\n"
       << "  \"bitrate_kbps\": " << bitrate_kbps << ",\n"
       << "  \"measured_fps\": " << std::fixed << std::setprecision(1) << fps << ",\n"
       << "  \"dropped_frames\": " << dropped << ",\n"
       << "  \"adaptive_rate\": " << (adaptive ? "true" : "false") << ",\n"
       << "  \"connected\": " << (connected ? "true" : "false") << ",\n"
       << "  \"is_standby\": " << (is_standby ? "true" : "false") << ",\n"
       << "  \"rotation\": " << rotation << ",\n"
       << "  \"camera_type\": \"" << camera_type << "\",\n"
       << "  \"device_name\": \"" << device_name << "\"\n"
       << "}\n";

    std::string json_str = ss.str();

    std::ofstream ofs(tmp_path, std::ios::trunc);
    if (ofs.is_open()) {
        ofs << json_str;
        ofs.close();
        std::rename(tmp_path.c_str(), final_path.c_str());
    } else {
        std::ofstream fallback("/tmp/camera_stats.json", std::ios::trunc);
        if (fallback.is_open()) {
            fallback << json_str;
        }
    }
}

// Kiểm tra và xử lý file điều khiển runtime từ MAVLink / GCS
static void processCameraCommands(GstStreamSink& sink, NeonFrameProcessor& processor,
                                  CameraConfig& config) {
    const std::string cmd_path = "/run/drone/camera_command.json";
    std::ifstream ifs(cmd_path);
    if (!ifs.is_open()) return;

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    ifs.close();
    std::remove(cmd_path.c_str());

    std::string content = buffer.str();
    if (content.empty()) return;

    std::cout << "[CommandHandler] Nhận lệnh runtime: " << content << "\n";

    // 1. Bitrate
    auto pos = content.find("\"bitrate_kbps\":");
    if (pos != std::string::npos) {
        int br = std::atoi(content.c_str() + pos + 15);
        if (br >= 300 && br <= 20000) {
            config.encoder.bitrate_kbps = br;
            sink.setBitrate(br);
            std::cout << "[CommandHandler] ✓ Đã cập nhật bitrate: " << br << " kbps\n";
        }
    }

    // 2. Rotation
    pos = content.find("\"rotation\":");
    if (pos != std::string::npos) {
        int rot = std::atoi(content.c_str() + pos + 11);
        if (rot == 0 || rot == 90 || rot == 180 || rot == 270) {
            config.video.rotation = rot;
            processor.setRotation(rot);
            std::cout << "[CommandHandler] ✓ Đã cập nhật góc xoay: " << rot << "°\n";
        }
    }

    // 3. Reset
    pos = content.find("\"reset\":");
    if (pos != std::string::npos) {
        if (content.find("true", pos) != std::string::npos || content.find("1", pos) != std::string::npos) {
            std::cout << "[CommandHandler] ! Nhận yêu cầu CAMERA_RESET\n";
            g_reset_requested = true;
        }
    }
}

// Camera Factory (SOLID - Open/Closed & Dependency Inversion)
std::unique_ptr<ICameraDevice> createCamera(const std::string& type) {
    if (type == "v4l2") {
        return std::make_unique<V4L2Camera>();
    }
    // Mặc định là RealSense
    return std::make_unique<RealSenseCamera>();
}

int main(int argc, char** argv) {
    std::string config_path = "/app/camera.yaml";
    if (argc > 1) {
        config_path = argv[1];
    }

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    CameraConfig config;
    if (!ConfigManager::loadConfig(config_path, config)) {
        std::cerr << "[StreamerEngine][FATAL] Không thể đọc file cấu hình: " << config_path << "\n";
        return 1;
    }
    ConfigManager::printConfig(config);

    // 1. Khởi tạo GStreamer Sink NGAY LẬP TỨC để MediaMTX luôn có luồng RTSP /camera online
    GstStreamSink sink;
    while (g_running && !sink.start(config)) {
        std::cerr << "[StreamerEngine][RETRY] Chưa kết nối được MediaMTX (" 
                  << config.network.target_url << "), thử lại sau 1 giây...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "[StreamerEngine] ✓ GStreamer Sink đã sẵn sàng phục vụ RTSP clients\n";

    // 2. Khởi tạo Image Processor (NEON SIMD) & Standby Pattern Generator
    NeonFrameProcessor processor(config.video.rotation);
    StandbyGenerator standby_gen(config.video.width, config.video.height, config.video.fps);
    SafeRingBuffer ring_buffer;

    std::atomic<bool> camera_connected(false);
    std::atomic<bool> is_standby(true);
    std::mutex dev_name_mutex;
    std::string current_dev_name = "None (Standby Pattern)";

    // ==========================================================================
    // THREAD 1: CAPTURE / STANDBY WORKER (HOT-PLUG & AUTO-RECOVERY)
    // ==========================================================================
    std::thread capture_thread([&]() {
        std::cout << "[CaptureWorker] ✓ Luồng thu thập video (Hot-Plug & Standby) bắt đầu chạy\n";
        std::unique_ptr<ICameraDevice> camera = createCamera(config.camera_type);
        uint64_t capture_failures = 0;
        auto last_detect_time = std::chrono::steady_clock::now() - std::chrono::seconds(5);

        const auto frame_interval = std::chrono::microseconds(1000000 / (config.video.fps > 0 ? config.video.fps : 30));

        while (g_running) {
            // Nếu có yêu cầu reset
            if (g_reset_requested.exchange(false)) {
                std::cout << "[CaptureWorker][RESET] Tiến hành reset thiết bị camera...\n";
                if (camera_connected) {
                    camera->stopStream();
                    camera_connected = false;
                    is_standby = true;
                }
                camera = createCamera(config.camera_type);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            if (!camera_connected) {
                // CHẾ ĐỘ STANDBY: Camera chưa cắm hoặc đang khởi động
                is_standby = true;
                {
                    std::lock_guard<std::mutex> lock(dev_name_mutex);
                    current_dev_name = "None (Standby Pattern)";
                }

                auto loop_start = std::chrono::steady_clock::now();

                // 1. Sinh Standby frame và đẩy vào hàng đợi stream
                FrameData standby_frame;
                standby_gen.generateFrame(standby_frame, "WAITING FOR REALSENSE CAMERA DEVICE...");
                ring_buffer.push(std::move(standby_frame));

                // 2. Định kỳ kiểm tra xem thiết bị RealSense đã cắm vào chưa (mỗi 1.5 giây)
                if (std::chrono::duration_cast<std::chrono::milliseconds>(loop_start - last_detect_time).count() >= 1500) {
                    last_detect_time = loop_start;
                    if (camera->isAvailable()) {
                        std::cout << "[CaptureWorker][HOT-PLUG] Phát hiện thiết bị RealSense! Tiến hành kết nối...\n";
                        if (camera->initialize(config) && camera->startStream()) {
                            camera_connected = true;
                            is_standby = false;
                            capture_failures = 0;
                            {
                                std::lock_guard<std::mutex> lock(dev_name_mutex);
                                current_dev_name = camera->getDeviceName();
                            }
                            std::cout << "[CaptureWorker][HOT-PLUG] ✓ KẾT NỐI THÀNH CÔNG THIẾT BỊ: "
                                      << camera->getDeviceName() << "! Tự động chuyển sang Live Stream.\n";
                        } else {
                            std::cerr << "[CaptureWorker][HOT-PLUG] Khởi tạo thiết bị thất bại, tiếp tục chờ...\n";
                        }
                    }
                }

                // Điều tốc theo đúng FPS để luồng RTSP mượt mà
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - loop_start);
                if (elapsed < frame_interval) {
                    std::this_thread::sleep_for(frame_interval - elapsed);
                }
            } else {
                // CHẾ ĐỘ LIVE: Thu nhận hình ảnh từ RealSense / Camera phần cứng
                FrameData frame;
                if (camera->captureFrame(frame)) {
                    capture_failures = 0;
                    is_standby = false;
                    ring_buffer.push(std::move(frame));
                } else {
                    capture_failures++;
                    auto err = camera->getLastErrorCode();

                    if (capture_failures % 30 == 0) {
                        std::cerr << "[CaptureWorker][WARN] Lỗi bắt frame liên tiếp: " 
                                  << errorCodeToString(err) << "\n";
                    }

                    // Nếu thiết bị bị rút (EPROTO -71 / ERR_USB_IO_ERROR) hoặc mất kết nối kéo dài
                    if (err == StreamErrorCode::ERR_USB_IO_ERROR || capture_failures > 60) {
                        std::cerr << "[CaptureWorker][HOT-UNPLUG] Mất kết nối camera phần cứng! "
                                  << "Chuyển ngay về Standby Stream an toàn...\n";
                        camera->stopStream();
                        camera_connected = false;
                        is_standby = true;
                        capture_failures = 0;
                        camera = createCamera(config.camera_type);
                        last_detect_time = std::chrono::steady_clock::now();
                    } else {
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                }
            }
        }

        if (camera_connected) {
            camera->stopStream();
        }
        std::cout << "[CaptureWorker] Đã dừng luồng thu thập video\n";
    });

    // ==========================================================================
    // THREAD 2: ENCODE & STREAM WORKER (XOAY SIMD, ĐẨY GSTREAMER, XỬ LÝ LỆNH IPC)
    // ==========================================================================
    std::thread stream_thread([&]() {
        std::cout << "[StreamWorker] ✓ Luồng nén & xuất bản RTSP bắt đầu chạy\n";
        uint64_t stream_frames = 0;
        auto last_stats_time = std::chrono::steady_clock::now();
        auto last_cmd_time = std::chrono::steady_clock::now();

        while (g_running) {
            FrameData frame;
            // Lấy frame mới nhất từ ring buffer (timeout 40ms)
            if (ring_buffer.pop(frame, std::chrono::milliseconds(40))) {
                // Nếu là frame live từ camera thật, áp dụng xoay SIMD (nếu có cấu hình)
                if (!is_standby.load()) {
                    processor.process(frame);
                }

                // Đẩy vào pipeline GStreamer
                if (!sink.pushFrame(frame)) {
                    if (!sink.isConnected()) {
                        std::cerr << "[StreamWorker][WARN] Mất kết nối pipeline GStreamer, thử khôi phục lại...\n";
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        sink.start(config);
                    }
                } else {
                    stream_frames++;
                }
            }

            auto now = std::chrono::steady_clock::now();

            // Kiểm tra file lệnh /run/drone/camera_command.json mỗi 500ms
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_cmd_time).count() >= 500) {
                last_cmd_time = now;
                processCameraCommands(sink, processor, config);
            }

            // Cập nhật metrics mỗi 1.0 giây ra file /run/drone/camera_stats.json
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_stats_time).count() >= 1000) {
                double elapsed = std::chrono::duration<double>(now - last_stats_time).count();
                static uint64_t last_stream_frames = 0;
                double current_fps = (elapsed > 0)
                    ? static_cast<double>(stream_frames - last_stream_frames) / elapsed
                    : static_cast<double>(config.video.fps);
                last_stream_frames = stream_frames;

                int live_bitrate = sink.getCurrentBitrate();
                uint64_t total_drops = ring_buffer.getDroppedCount() + sink.getDroppedFrames();

                std::string dev_name;
                {
                    std::lock_guard<std::mutex> lock(dev_name_mutex);
                    dev_name = current_dev_name;
                }

                exportCameraStats(live_bitrate, current_fps, total_drops, config.encoder.adaptive_rate,
                                  camera_connected.load(), is_standby.load(), processor.getRotation(),
                                  config.camera_type, dev_name);

                static int print_counter = 0;
                if (++print_counter >= 5) {
                    print_counter = 0;
                    std::cout << "[StreamerEngine] Stream #" << stream_frames 
                              << " | State: " << (is_standby.load() ? "[STANDBY]" : "[LIVE FEED]")
                              << " | FPS: " << static_cast<int>(current_fps) << "/" << config.video.fps
                              << " | Bitrate: " << live_bitrate << " kbps"
                              << " | Dropped: " << total_drops
                              << " | Rotation: " << processor.getRotation() << "°"
                              << " | Status: " << (sink.isConnected() ? "ONLINE" : "RECONNECTING") << "\n";
                }
                last_stats_time = now;
            }
        }
        sink.stop();
        std::cout << "[StreamWorker] Đã dừng luồng nén & xuất bản\n";
    });

    // Chờ các luồng kết thúc
    if (capture_thread.joinable()) capture_thread.join();
    if (stream_thread.joinable()) stream_thread.join();

    std::cout << "[StreamerEngine] Hoàn tất tắt hệ thống an toàn.\n";
    return 0;
}
