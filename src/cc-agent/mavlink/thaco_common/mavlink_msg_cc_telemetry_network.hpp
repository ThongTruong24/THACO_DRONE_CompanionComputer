// MESSAGE CC_TELEMETRY_NETWORK support class

#pragma once

namespace mavlink {
namespace thaco_common {
namespace msg {

/**
 * @brief CC_TELEMETRY_NETWORK message
 *
 * Companion Computer network interfaces, Access Point (hostapd), WiFi station, and Ethernet configuration.
 */
struct CC_TELEMETRY_NETWORK : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42012;
    static constexpr size_t LENGTH = 228;
    static constexpr size_t MIN_LENGTH = 228;
    static constexpr uint8_t CRC_EXTRA = 215;
    static constexpr auto NAME = "CC_TELEMETRY_NETWORK";


    uint32_t uap0_rx_kb; /*<  Hotspot uap0 total received traffic in kilobytes */
    uint32_t uap0_tx_kb; /*<  Hotspot uap0 total transmitted traffic in kilobytes */
    uint32_t wlan0_rx_kb; /*<  WiFi client wlan0 total received traffic in kilobytes */
    uint32_t wlan0_tx_kb; /*<  WiFi client wlan0 total transmitted traffic in kilobytes */
    uint32_t eth0_rx_kb; /*<  Ethernet eth0 total received traffic in kilobytes */
    uint32_t eth0_tx_kb; /*<  Ethernet eth0 total transmitted traffic in kilobytes */
    uint8_t ap_channel; /*<  Wi-Fi AP radio channel, e.g. 6 */
    uint8_t ap_ieee80211n; /*<  802.11n high-throughput flag (1: enabled) */
    uint8_t ap_wmm_enabled; /*<  WMM QoS flag (1: enabled) */
    uint8_t ap_wpa; /*<  WPA security version (2: WPA2-PSK) */
    uint8_t ap_client_count; /*<  Count of currently connected Wi-Fi clients */
    uint8_t ap_status; /*<  Access Point hostapd operational status */
    uint8_t wlan0_status; /*<  Wi-Fi station interface operational status */
    uint8_t eth0_status; /*<  Ethernet interface carrier / link status */
    uint8_t eth0_is_static; /*<  Ethernet IP configuration (1: Static IP, 0: DHCP Client) */
    uint8_t wlan0_dhcp; /*<  Wi-Fi client DHCP client active flag */
    uint8_t dnsmasq_status; /*<  Local DHCP/DNS server (dnsmasq) active flag */
    int8_t wlan0_rssi; /*<  WiFi station received signal strength in dBm, e.g. -65 */
    std::array<char, 16> eth0_ip; /*<  Ethernet interface IP address */
    std::array<char, 16> eth0_netmask; /*<  Ethernet subnet mask, e.g. 255.255.255.0 */
    std::array<char, 16> wlan0_ip; /*<  Wi-Fi station interface IP address */
    std::array<char, 16> wlan0_netmask; /*<  Wi-Fi station subnet mask */
    std::array<char, 32> wlan0_ssid; /*<  Connected Wi-Fi Access Point SSID */
    std::array<char, 16> ap_ip; /*<  Access Point interface gateway IP (uap0), e.g. 192.168.10.1 */
    std::array<char, 16> ap_netmask; /*<  Access Point subnet mask */
    std::array<char, 32> ap_ssid; /*<  Wi-Fi Access Point SSID, e.g. AP_DRONE */
    std::array<char, 16> ap_wpa_passphrase; /*<  Wi-Fi AP WPA passphrase (redacted if restricted) */
    std::array<char, 12> ap_key_mgmt; /*<  Key management protocol, e.g. WPA-PSK */
    std::array<char, 4> ap_hw_mode; /*<  Hardware radio band mode, e.g. g or a */


    inline std::string get_name(void) const override
    {
            return NAME;
    }

    inline Info get_message_info(void) const override
    {
            return { MSG_ID, LENGTH, MIN_LENGTH, CRC_EXTRA };
    }

    inline std::string to_yaml(void) const override
    {
        std::stringstream ss;

        ss << NAME << ":" << std::endl;
        ss << "  uap0_rx_kb: " << uap0_rx_kb << std::endl;
        ss << "  uap0_tx_kb: " << uap0_tx_kb << std::endl;
        ss << "  wlan0_rx_kb: " << wlan0_rx_kb << std::endl;
        ss << "  wlan0_tx_kb: " << wlan0_tx_kb << std::endl;
        ss << "  eth0_rx_kb: " << eth0_rx_kb << std::endl;
        ss << "  eth0_tx_kb: " << eth0_tx_kb << std::endl;
        ss << "  ap_channel: " << +ap_channel << std::endl;
        ss << "  ap_ieee80211n: " << +ap_ieee80211n << std::endl;
        ss << "  ap_wmm_enabled: " << +ap_wmm_enabled << std::endl;
        ss << "  ap_wpa: " << +ap_wpa << std::endl;
        ss << "  ap_client_count: " << +ap_client_count << std::endl;
        ss << "  ap_status: " << +ap_status << std::endl;
        ss << "  wlan0_status: " << +wlan0_status << std::endl;
        ss << "  eth0_status: " << +eth0_status << std::endl;
        ss << "  eth0_is_static: " << +eth0_is_static << std::endl;
        ss << "  wlan0_dhcp: " << +wlan0_dhcp << std::endl;
        ss << "  dnsmasq_status: " << +dnsmasq_status << std::endl;
        ss << "  wlan0_rssi: " << +wlan0_rssi << std::endl;
        ss << "  eth0_ip: \"" << to_string(eth0_ip) << "\"" << std::endl;
        ss << "  eth0_netmask: \"" << to_string(eth0_netmask) << "\"" << std::endl;
        ss << "  wlan0_ip: \"" << to_string(wlan0_ip) << "\"" << std::endl;
        ss << "  wlan0_netmask: \"" << to_string(wlan0_netmask) << "\"" << std::endl;
        ss << "  wlan0_ssid: \"" << to_string(wlan0_ssid) << "\"" << std::endl;
        ss << "  ap_ip: \"" << to_string(ap_ip) << "\"" << std::endl;
        ss << "  ap_netmask: \"" << to_string(ap_netmask) << "\"" << std::endl;
        ss << "  ap_ssid: \"" << to_string(ap_ssid) << "\"" << std::endl;
        ss << "  ap_wpa_passphrase: \"" << to_string(ap_wpa_passphrase) << "\"" << std::endl;
        ss << "  ap_key_mgmt: \"" << to_string(ap_key_mgmt) << "\"" << std::endl;
        ss << "  ap_hw_mode: \"" << to_string(ap_hw_mode) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << uap0_rx_kb;                    // offset: 0
        map << uap0_tx_kb;                    // offset: 4
        map << wlan0_rx_kb;                   // offset: 8
        map << wlan0_tx_kb;                   // offset: 12
        map << eth0_rx_kb;                    // offset: 16
        map << eth0_tx_kb;                    // offset: 20
        map << ap_channel;                    // offset: 24
        map << ap_ieee80211n;                 // offset: 25
        map << ap_wmm_enabled;                // offset: 26
        map << ap_wpa;                        // offset: 27
        map << ap_client_count;               // offset: 28
        map << ap_status;                     // offset: 29
        map << wlan0_status;                  // offset: 30
        map << eth0_status;                   // offset: 31
        map << eth0_is_static;                // offset: 32
        map << wlan0_dhcp;                    // offset: 33
        map << dnsmasq_status;                // offset: 34
        map << wlan0_rssi;                    // offset: 35
        map << eth0_ip;                       // offset: 36
        map << eth0_netmask;                  // offset: 52
        map << wlan0_ip;                      // offset: 68
        map << wlan0_netmask;                 // offset: 84
        map << wlan0_ssid;                    // offset: 100
        map << ap_ip;                         // offset: 132
        map << ap_netmask;                    // offset: 148
        map << ap_ssid;                       // offset: 164
        map << ap_wpa_passphrase;             // offset: 196
        map << ap_key_mgmt;                   // offset: 212
        map << ap_hw_mode;                    // offset: 224
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> uap0_rx_kb;                    // offset: 0
        map >> uap0_tx_kb;                    // offset: 4
        map >> wlan0_rx_kb;                   // offset: 8
        map >> wlan0_tx_kb;                   // offset: 12
        map >> eth0_rx_kb;                    // offset: 16
        map >> eth0_tx_kb;                    // offset: 20
        map >> ap_channel;                    // offset: 24
        map >> ap_ieee80211n;                 // offset: 25
        map >> ap_wmm_enabled;                // offset: 26
        map >> ap_wpa;                        // offset: 27
        map >> ap_client_count;               // offset: 28
        map >> ap_status;                     // offset: 29
        map >> wlan0_status;                  // offset: 30
        map >> eth0_status;                   // offset: 31
        map >> eth0_is_static;                // offset: 32
        map >> wlan0_dhcp;                    // offset: 33
        map >> dnsmasq_status;                // offset: 34
        map >> wlan0_rssi;                    // offset: 35
        map >> eth0_ip;                       // offset: 36
        map >> eth0_netmask;                  // offset: 52
        map >> wlan0_ip;                      // offset: 68
        map >> wlan0_netmask;                 // offset: 84
        map >> wlan0_ssid;                    // offset: 100
        map >> ap_ip;                         // offset: 132
        map >> ap_netmask;                    // offset: 148
        map >> ap_ssid;                       // offset: 164
        map >> ap_wpa_passphrase;             // offset: 196
        map >> ap_key_mgmt;                   // offset: 212
        map >> ap_hw_mode;                    // offset: 224
    }
};

} // namespace msg
} // namespace thaco_common
} // namespace mavlink
