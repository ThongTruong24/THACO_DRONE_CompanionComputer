#pragma once
#include "thaco_common/mavlink.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace vision {
using Bytes = std::vector<uint8_t>;
struct Config {
    std::string host;
    int port, system_id, component_id;
    int camera_component_id=105;
    double status_max_hz=2, selection_ttl_s=5, selection_stale_s=2;
    int reconnect_min_ms=100, reconnect_max_ms=5000, connect_timeout_ms=500;
    Config(std::string host="127.0.0.1", int port=5760, int system_id=1, int component_id=192);
    bool targets(int system, int component) const;
};
class Decoder {
    mavlink_message_t state_{};
    mavlink_status_t status_{};
public:
    std::vector<mavlink_message_t> feed(const Bytes& bytes);
    void reset();
};
class Codec {
    uint8_t system_, component_;
    mavlink_status_t status_{};
    std::mutex mutex_;
public:
    Codec(uint8_t system, uint8_t component):system_(system),component_(component){}
    Bytes heartbeat(uint8_t type=MAV_TYPE_ONBOARD_CONTROLLER);
    Bytes config_get(uint32_t request_id, const std::string& key);
    Bytes ack(uint16_t command, uint8_t result, uint8_t target_system, uint8_t target_component);
    Bytes telemetry(const mavlink_cc_telemetry_vision_t& payload);
    Bytes camera_information(const mavlink_camera_information_t& payload);
    Bytes tracking_status(const mavlink_camera_tracking_image_status_t& payload);
};
using TimePoint = std::chrono::steady_clock::time_point;
struct CameraEvent {
    bool clear;
    float x=0, y=0, radius=0;
    uint64_t generation=0;
};
struct CameraResponse {
    std::vector<Bytes> packets;
    std::optional<CameraEvent> event;
};
// Owns one encoder/sequence counter for every packet from the camera identity.
// Selection snapshots arrive through bounded IPC; no inference runs here.
class CameraProtocol {
    Config config_;
    Codec codec_;
    std::mutex mutex_;
    TimePoint boot_=std::chrono::steady_clock::now(), expires_{}, updated_{}, next_status_{}, last_status_{};
    uint64_t generation_=0;
    bool point_pending_=false;
    std::optional<std::array<float,4>> bbox_;
    uint16_t width_=0, height_=0;
    std::chrono::microseconds interval_{0};
    Bytes information(TimePoint now);
    Bytes status(TimePoint now);
public:
    explicit CameraProtocol(Config config);
    Bytes heartbeat();
    CameraResponse handle(const mavlink_message_t& message, TimePoint now=std::chrono::steady_clock::now());
    void update_selection(uint64_t generation, std::optional<std::array<float,4>> bbox,
                          uint16_t width, uint16_t height, TimePoint now=std::chrono::steady_clock::now());
    std::optional<Bytes> poll(TimePoint now=std::chrono::steady_clock::now());
    void reset();
};
class PacketQueue {
    struct Item { Bytes bytes; bool latest; };
    size_t capacity_;
    std::deque<Item> queue_;
    mutable std::mutex mutex_;
public:
    explicit PacketQueue(size_t capacity):capacity_(capacity){}
    bool push(Bytes bytes, bool latest=false);
    std::optional<Bytes> pop();
    size_t size() const;
    void clear();
};
class Backoff {
    int minimum_, maximum_, next_;
public:
    Backoff(int minimum,int maximum):minimum_(minimum),maximum_(maximum),next_(minimum){}
    int next() { int result=next_; next_=std::min(maximum_,next_*2); return result; }
    void reset() { next_=minimum_; }
};
class Transport {
    Config config_;
    Codec codec_;
    CameraProtocol camera_;
    PacketQueue outbound_{32};
    std::function<void(const mavlink_message_t&)> message_;
    std::function<void(bool)> connection_;
    std::atomic<bool> running_{false}, connected_{false};
    std::thread worker_;
    void run();
public:
    Transport(Config config, std::function<void(const mavlink_message_t&)> message,
              std::function<void(bool)> connection={});
    ~Transport() { stop(); }
    void start();
    void stop();
    bool send(Bytes bytes, bool latest=false);
    bool connected() const { return connected_.load(); }
    size_t pending() const { return outbound_.size(); }
    Codec& primary_codec() { return codec_; }
    CameraProtocol& camera() { return camera_; }
};
}
