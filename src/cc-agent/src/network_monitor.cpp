#include "network_monitor.hpp"
#include <cstring>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace cc {

NetworkMonitor::NetworkMonitor() {
    std::strncpy(draft_ssid, "THACO-WiFi", sizeof(draft_ssid) - 1);
    std::strncpy(draft_passphrase, "thaco123456", sizeof(draft_passphrase) - 1);
    std::strncpy(draft_eth0_ip, "10.14.95.43", sizeof(draft_eth0_ip) - 1);
    draft_channel = 6;
}

void NetworkMonitor::get_if_addr(const char* iface,
                                  char* ip_out, char* mask_out, int bufsz) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return;

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
        struct sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
        std::strncpy(ip_out, inet_ntoa(sa->sin_addr), bufsz - 1);
    }
    if (mask_out) {
        if (ioctl(fd, SIOCGIFNETMASK, &ifr) == 0) {
            struct sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
            std::strncpy(mask_out, inet_ntoa(sa->sin_addr), bufsz - 1);
        }
    }
    close(fd);
}

uint8_t NetworkMonitor::get_if_status(const char* iface) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return 0;

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

    uint8_t status = 0;
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0) {
        if (ifr.ifr_flags & IFF_UP) status = 2; // ACTIVE
    }
    close(fd);
    return status;
}

int8_t NetworkMonitor::get_wlan_rssi(const char* iface) {
    char path[128];
    std::snprintf(path, sizeof(path),
                  "/sys/class/net/%s/wireless/link", iface);
    std::ifstream f(path);
    if (!f.is_open()) return -100;
    int link = -100;
    f >> link;
    // Convert quality (0-70) to approx RSSI (-100 to -30 dBm)
    return static_cast<int8_t>(-100 + link);
}

uint8_t NetworkMonitor::get_ap_clients(const char* /*iface*/) {
    // Count entries in /proc/net/arp for 192.168.10.x (AP subnet)
    std::ifstream f("/proc/net/arp");
    if (!f.is_open()) return 0;
    uint8_t count = 0;
    std::string line;
    std::getline(f, line); // header
    while (std::getline(f, line)) {
        if (line.find("192.168.10.") != std::string::npos &&
            line.find("00:00:00:00:00:00") == std::string::npos) {
            ++count;
        }
    }
    return count;
}

void NetworkMonitor::read_net_counters(const char* iface,
                                        uint64_t* rx_bytes_out,
                                        uint64_t* tx_bytes_out) {
    char path[128];
    auto read_u64 = [](const char* p) -> uint64_t {
        std::ifstream f(p);
        uint64_t v = 0;
        if (f.is_open()) f >> v;
        return v;
    };
    std::snprintf(path, sizeof(path),
                  "/sys/class/net/%s/statistics/rx_bytes", iface);
    *rx_bytes_out = read_u64(path);
    std::snprintf(path, sizeof(path),
                  "/sys/class/net/%s/statistics/tx_bytes", iface);
    *tx_bytes_out = read_u64(path);
}

NetworkStats NetworkMonitor::read_stats() {
    NetworkStats s;

    // Interface addresses
    get_if_addr("eth0",  s.eth0_ip,  s.eth0_netmask,  sizeof(s.eth0_ip));
    get_if_addr("wlan0", s.wlan0_ip, s.wlan0_netmask, sizeof(s.wlan0_ip));
    get_if_addr("uap0",  s.uap0_ip,  s.uap0_netmask,  sizeof(s.uap0_ip));

    // Interface status
    s.eth0_status  = get_if_status("eth0");
    s.wlan0_status = get_if_status("wlan0");
    s.ap_status    = get_if_status("uap0");

    // RSSI / AP clients
    s.wlan0_rssi     = get_wlan_rssi("wlan0");
    s.ap_client_count = get_ap_clients("uap0");

    // Traffic counters (in KB)
    uint64_t rx = 0, tx = 0;
    read_net_counters("wlan0", &rx, &tx);
    s.wlan0_rx_kb = static_cast<uint32_t>(rx / 1024);
    s.wlan0_tx_kb = static_cast<uint32_t>(tx / 1024);
    read_net_counters("eth0", &rx, &tx);
    s.eth0_rx_kb = static_cast<uint32_t>(rx / 1024);
    s.eth0_tx_kb = static_cast<uint32_t>(tx / 1024);
    read_net_counters("uap0", &rx, &tx);
    s.uap0_rx_kb = static_cast<uint32_t>(rx / 1024);
    s.uap0_tx_kb = static_cast<uint32_t>(tx / 1024);

    // AP config (draft values)
    std::strncpy(s.ap_ssid, draft_ssid, sizeof(s.ap_ssid) - 1);
    std::strncpy(s.ap_wpa_passphrase, draft_passphrase, sizeof(s.ap_wpa_passphrase) - 1);
    std::strncpy(s.ap_key_mgmt, "WPA-PSK", sizeof(s.ap_key_mgmt) - 1);
    std::strncpy(s.ap_hw_mode, "g", sizeof(s.ap_hw_mode) - 1);
    s.ap_channel     = draft_channel;
    s.ap_ieee80211n  = 1;
    s.ap_wmm_enabled = 1;
    s.ap_wpa         = 2;
    s.eth0_is_static = 1;
    s.wlan0_dhcp     = 1;
    s.dnsmasq_status = (s.ap_status == 2) ? 2 : 0;

    return s;
}

} // namespace cc
