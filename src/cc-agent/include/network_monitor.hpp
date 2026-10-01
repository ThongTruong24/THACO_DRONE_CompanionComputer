#pragma once
#include <string>
#include <cstdint>
#include <cstring>

namespace cc {

struct NetworkStats {
    char eth0_ip[16]       = {};
    char eth0_netmask[16]  = {};
    char wlan0_ip[16]      = {};
    char wlan0_netmask[16] = {};
    char uap0_ip[16]       = {};
    char uap0_netmask[16]  = {};

    char ap_ssid[32]          = {};
    char ap_wpa_passphrase[32]= {};
    char ap_key_mgmt[16]      = {};
    char ap_hw_mode[8]        = {};
    uint8_t ap_channel        = 6;

    uint32_t wlan0_rx_kb  = 0;
    uint32_t wlan0_tx_kb  = 0;
    uint32_t eth0_rx_kb   = 0;
    uint32_t eth0_tx_kb   = 0;
    uint32_t uap0_rx_kb   = 0;
    uint32_t uap0_tx_kb   = 0;

    uint8_t ap_status      = 0;  // 2=active
    uint8_t wlan0_status   = 0;
    uint8_t eth0_status    = 0;
    uint8_t eth0_is_static = 1;
    uint8_t wlan0_dhcp     = 1;
    uint8_t dnsmasq_status = 0;
    uint8_t ap_ieee80211n  = 1;
    uint8_t ap_wmm_enabled = 1;
    uint8_t ap_wpa         = 2;
    uint8_t ap_client_count= 0;
    int8_t  wlan0_rssi     = -100;
};

class NetworkMonitor {
public:
    NetworkMonitor();
    NetworkStats read_stats();

    // Draft settings updated by incoming MAVLink (QGC -> Drone)
    char    draft_ssid[32]        = {};
    char    draft_passphrase[32]  = {};
    char    draft_eth0_ip[16]     = {};
    uint8_t draft_channel         = 6;

private:
    static void get_if_addr(const char* iface, char* ip_out, char* mask_out, int bufsz);
    static uint8_t get_if_status(const char* iface);
    static int8_t  get_wlan_rssi(const char* iface);
    static uint8_t get_ap_clients(const char* iface);
    static void read_net_counters(const char* iface,
                                  uint64_t* rx_bytes_out,
                                  uint64_t* tx_bytes_out);
};

} // namespace cc
