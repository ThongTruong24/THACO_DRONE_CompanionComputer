// Spike 2 (T3): writer shm ring theo spec §5.10, 3 slot, seqlock.
// Descriptor (slot, seq) gửi qua UDP loopback thay cho cc_msgs/FrameReady.
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <thread>

constexpr uint32_t W = 640, H = 480;
constexpr size_t COLOR = size_t(W) * H * 3, DEPTH = size_t(W) * H * 2;
constexpr size_t SLOT_HDR = 64;  // seq, timestamp_ns, phần còn lại để đệm
constexpr size_t SLOT_SIZE = (SLOT_HDR + COLOR + DEPTH + 63) / 64 * 64;
constexpr size_t FILE_HDR = 64;
constexpr int SLOTS = 3;

struct Desc {
    uint32_t slot;
    uint32_t pad;
    uint64_t seq;
};

int main(int argc, char **argv) {
    double hz = 10;
    long frames = 600;
    int port = 47001;
    std::string path = "/dev/shm/spike_pool";
    for (int i = 1; i + 1 < argc; i += 2) {
        std::string k = argv[i];
        if (k == "--hz") hz = std::atof(argv[i + 1]);
        else if (k == "--frames") frames = std::atol(argv[i + 1]);
        else if (k == "--port") port = std::atoi(argv[i + 1]);
        else if (k == "--path") path = argv[i + 1];
    }

    const size_t total = FILE_HDR + SLOT_SIZE * SLOTS;
    int fd = open(path.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0 || ftruncate(fd, total) != 0) { std::perror("open/ftruncate"); return 1; }
    auto *base = static_cast<uint8_t *>(mmap(nullptr, total, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (base == MAP_FAILED) { std::perror("mmap"); return 1; }
    std::memset(base, 0, total);
    std::memcpy(base, "CCFP", 4);
    uint32_t hdr[5] = {1, SLOTS, W, H, uint32_t(SLOT_SIZE)};  // version, slots, w, h, slot_size
    std::memcpy(base + 4, hdr, sizeof(hdr));

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &dst.sin_addr);

    auto wall0 = std::chrono::steady_clock::now();
    std::clock_t cpu0 = std::clock();
    auto period = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<double>(1.0 / hz));
    auto next = wall0;

    for (long n = 1; n <= frames; ++n) {
        next += period;
        uint8_t *slot = base + FILE_HDR + SLOT_SIZE * (n % SLOTS);
        auto *seq = reinterpret_cast<uint64_t *>(slot);
        uint8_t fill = uint8_t(n & 0xFF);

        __atomic_store_n(seq, uint64_t(2 * n - 1), __ATOMIC_RELEASE);  // lẻ: đang ghi
        uint64_t ts = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::steady_clock::now().time_since_epoch()).count();
        std::memcpy(slot + 8, &ts, 8);
        std::memset(slot + SLOT_HDR, fill, COLOR);          // color (BGR giả)
        std::memset(slot + SLOT_HDR + COLOR, fill, DEPTH);  // depth (16UC1 giả)
        __atomic_store_n(seq, uint64_t(2 * n), __ATOMIC_RELEASE);  // chẵn: hợp lệ

        Desc d{uint32_t(n % SLOTS), 0, uint64_t(2 * n)};
        sendto(sock, &d, sizeof(d), 0, reinterpret_cast<sockaddr *>(&dst), sizeof(dst));
        std::this_thread::sleep_until(next);
    }
    Desc end{0xFFFFFFFFu, 0, uint64_t(frames)};
    for (int i = 0; i < 3; ++i) {
        sendto(sock, &end, sizeof(end), 0, reinterpret_cast<sockaddr *>(&dst), sizeof(dst));
        usleep(10000);
    }

    double wall = std::chrono::duration<double>(std::chrono::steady_clock::now() - wall0).count();
    double cpu = double(std::clock() - cpu0) / CLOCKS_PER_SEC;
    std::printf("writer: frames=%ld hz=%.0f wall=%.2fs cpu=%.2fs (%.1f%% 1 core)\n", frames, hz, wall, cpu,
                100.0 * cpu / wall);
    munmap(base, total);
    close(fd);
    unlink(path.c_str());
    return 0;
}
