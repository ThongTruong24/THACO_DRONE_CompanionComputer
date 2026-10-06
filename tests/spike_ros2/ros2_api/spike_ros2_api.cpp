// Spike 1 (T3): take() trên callback group không gắn executor + AsyncParametersClient.
// Chế độ: take | param_absent | param_server | param_present | rss
#include <chrono>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

static double ms_since(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

static long rss_kb() {
    std::ifstream f("/proc/self/status");
    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("VmRSS:", 0) == 0) return std::stol(line.substr(6));
    }
    return -1;
}

static int check(bool ok, const char *what) {
    std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", what);
    return ok ? 0 : 1;
}

static int run_take() {
    constexpr int N = 100;
    auto node = std::make_shared<rclcpp::Node>("spike_take");
    auto group = node->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive, false);
    rclcpp::SubscriptionOptions opts;
    opts.callback_group = group;
    auto sub = node->create_subscription<std_msgs::msg::Int32>(
        "/spike/take", rclcpp::QoS(N).reliable(), [](std_msgs::msg::Int32::ConstSharedPtr) {}, opts);
    auto pub = node->create_publisher<std_msgs::msg::Int32>("/spike/take", rclcpp::QoS(N).reliable());

    // Chờ discovery giữa pub và sub (cùng process, không spin executor).
    auto t0 = Clock::now();
    while (pub->get_subscription_count() == 0 && ms_since(t0) < 3000) std::this_thread::sleep_for(10ms);

    int rc = 0;
    // Không có message: take() trả false ngay.
    std_msgs::msg::Int32 msg;
    rclcpp::MessageInfo info;
    t0 = Clock::now();
    bool empty_take = sub->take(msg, info);
    double empty_ms = ms_since(t0);
    rc |= check(!empty_take && empty_ms < 1.0, "take() rỗng trả false, < 1 ms");
    std::printf("      take() rỗng: %.3f ms\n", empty_ms);

    for (int i = 0; i < N; ++i) {
        std_msgs::msg::Int32 m;
        m.data = i;
        pub->publish(m);
    }
    // Chờ message về hàng đợi, không spin executor nào.
    t0 = Clock::now();
    int got = 0;
    bool in_order = true;
    while (got < N && ms_since(t0) < 3000) {
        if (sub->take(msg, info)) {
            if (msg.data != got) in_order = false;
            ++got;
        } else {
            std::this_thread::sleep_for(1ms);
        }
    }
    std::printf("      nhận %d/%d trong %.1f ms\n", got, N, ms_since(t0));
    rc |= check(got == N, "take() nhận đủ 100 message");
    rc |= check(in_order, "đúng thứ tự, đúng giá trị");

    t0 = Clock::now();
    bool after = sub->take(msg, info);
    rc |= check(!after && ms_since(t0) < 1.0, "sau khi cạn, take() trả false, < 1 ms");
    return rc;
}

static int run_param_absent() {
    auto node = std::make_shared<rclcpp::Node>("spike_param_client");
    rclcpp::AsyncParametersClient client(node, "/node_khong_ton_tai");
    int rc = 0;
    double worst = 0;
    bool any_ready = false;
    for (int i = 0; i < 20; ++i) {
        auto t0 = Clock::now();
        any_ready |= client.service_is_ready();
        worst = std::max(worst, ms_since(t0));
    }
    std::printf("      service_is_ready() x20, tệ nhất %.3f ms\n", worst);
    rc |= check(!any_ready, "node vắng: service_is_ready() == false");
    rc |= check(worst < 10.0, "không chặn (< 10 ms mỗi lần)");
    return rc;
}

static int run_param_server() {
    rclcpp::NodeOptions o;
    o.append_parameter_override("answer", 42);
    o.automatically_declare_parameters_from_overrides(true);
    auto node = std::make_shared<rclcpp::Node>("spike_param_server", o);
    rclcpp::spin(node);
    return 0;
}

static int run_param_present() {
    auto node = std::make_shared<rclcpp::Node>("spike_param_client");
    rclcpp::AsyncParametersClient client(node, "/spike_param_server");
    int rc = 0;
    auto t0 = Clock::now();
    while (!client.service_is_ready() && ms_since(t0) < 5000) {
        rclcpp::spin_some(node);
        std::this_thread::sleep_for(20ms);
    }
    bool ready = client.service_is_ready();
    std::printf("      service_is_ready() lên sau %.0f ms\n", ms_since(t0));
    rc |= check(ready, "node có mặt: service_is_ready() == true trong 5 s");
    if (!ready) return 1;
    auto fut = client.get_parameters({"answer"});
    auto st = rclcpp::spin_until_future_complete(node, fut, 3s);
    bool ok = st == rclcpp::FutureReturnCode::SUCCESS && fut.get().size() == 1 &&
              fut.get()[0].as_int() == 42;
    rc |= check(ok, "get_parameters(\"answer\") == 42");
    return rc;
}

static int run_rss() {
    auto node = std::make_shared<rclcpp::Node>("spike_rss");
    auto pub = node->create_publisher<std_msgs::msg::Int32>("/spike/rss", 10);
    std::this_thread::sleep_for(1s);
    std::printf("      RSS idle node C++ (x86 WSL, chỉ tham khảo): %ld kB\n", rss_kb());
    return 0;
}

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    std::string mode = argc > 1 ? argv[1] : "";
    int rc = 2;
    if (mode == "take") rc = run_take();
    else if (mode == "param_absent") rc = run_param_absent();
    else if (mode == "param_server") rc = run_param_server();
    else if (mode == "param_present") rc = run_param_present();
    else if (mode == "rss") rc = run_rss();
    else std::fprintf(stderr, "usage: %s take|param_absent|param_server|param_present|rss\n", argv[0]);
    rclcpp::shutdown();
    return rc;
}
