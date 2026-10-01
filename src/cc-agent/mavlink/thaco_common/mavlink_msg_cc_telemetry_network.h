#pragma once
// MESSAGE CC_TELEMETRY_NETWORK PACKING

#define MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK 42012


typedef struct __mavlink_cc_telemetry_network_t {
 uint32_t uap0_rx_kb; /*<  Hotspot uap0 total received traffic in kilobytes*/
 uint32_t uap0_tx_kb; /*<  Hotspot uap0 total transmitted traffic in kilobytes*/
 uint32_t wlan0_rx_kb; /*<  WiFi client wlan0 total received traffic in kilobytes*/
 uint32_t wlan0_tx_kb; /*<  WiFi client wlan0 total transmitted traffic in kilobytes*/
 uint32_t eth0_rx_kb; /*<  Ethernet eth0 total received traffic in kilobytes*/
 uint32_t eth0_tx_kb; /*<  Ethernet eth0 total transmitted traffic in kilobytes*/
 uint8_t ap_channel; /*<  Wi-Fi AP radio channel, e.g. 6*/
 uint8_t ap_ieee80211n; /*<  802.11n high-throughput flag (1: enabled)*/
 uint8_t ap_wmm_enabled; /*<  WMM QoS flag (1: enabled)*/
 uint8_t ap_wpa; /*<  WPA security version (2: WPA2-PSK)*/
 uint8_t ap_client_count; /*<  Count of currently connected Wi-Fi clients*/
 uint8_t ap_status; /*<  Access Point hostapd operational status*/
 uint8_t wlan0_status; /*<  Wi-Fi station interface operational status*/
 uint8_t eth0_status; /*<  Ethernet interface carrier / link status*/
 uint8_t eth0_is_static; /*<  Ethernet IP configuration (1: Static IP, 0: DHCP Client)*/
 uint8_t wlan0_dhcp; /*<  Wi-Fi client DHCP client active flag*/
 uint8_t dnsmasq_status; /*<  Local DHCP/DNS server (dnsmasq) active flag*/
 int8_t wlan0_rssi; /*<  WiFi station received signal strength in dBm, e.g. -65*/
 char eth0_ip[16]; /*<  Ethernet interface IP address*/
 char eth0_netmask[16]; /*<  Ethernet subnet mask, e.g. 255.255.255.0*/
 char wlan0_ip[16]; /*<  Wi-Fi station interface IP address*/
 char wlan0_netmask[16]; /*<  Wi-Fi station subnet mask*/
 char wlan0_ssid[32]; /*<  Connected Wi-Fi Access Point SSID*/
 char ap_ip[16]; /*<  Access Point interface gateway IP (uap0), e.g. 192.168.10.1*/
 char ap_netmask[16]; /*<  Access Point subnet mask*/
 char ap_ssid[32]; /*<  Wi-Fi Access Point SSID, e.g. AP_DRONE*/
 char ap_wpa_passphrase[16]; /*<  Wi-Fi AP WPA passphrase (redacted if restricted)*/
 char ap_key_mgmt[12]; /*<  Key management protocol, e.g. WPA-PSK*/
 char ap_hw_mode[4]; /*<  Hardware radio band mode, e.g. g or a*/
} mavlink_cc_telemetry_network_t;

#define MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN 228
#define MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN 228
#define MAVLINK_MSG_ID_42012_LEN 228
#define MAVLINK_MSG_ID_42012_MIN_LEN 228

#define MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_CRC 215
#define MAVLINK_MSG_ID_42012_CRC 215

#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_ETH0_IP_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_ETH0_NETMASK_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_WLAN0_IP_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_WLAN0_NETMASK_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_WLAN0_SSID_LEN 32
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_AP_IP_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_AP_NETMASK_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_AP_SSID_LEN 32
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_AP_WPA_PASSPHRASE_LEN 16
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_AP_KEY_MGMT_LEN 12
#define MAVLINK_MSG_CC_TELEMETRY_NETWORK_FIELD_AP_HW_MODE_LEN 4

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_NETWORK { \
    42012, \
    "CC_TELEMETRY_NETWORK", \
    29, \
    {  { "uap0_rx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_telemetry_network_t, uap0_rx_kb) }, \
         { "uap0_tx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_telemetry_network_t, uap0_tx_kb) }, \
         { "wlan0_rx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_cc_telemetry_network_t, wlan0_rx_kb) }, \
         { "wlan0_tx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_cc_telemetry_network_t, wlan0_tx_kb) }, \
         { "eth0_rx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_cc_telemetry_network_t, eth0_rx_kb) }, \
         { "eth0_tx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 20, offsetof(mavlink_cc_telemetry_network_t, eth0_tx_kb) }, \
         { "ap_channel", NULL, MAVLINK_TYPE_UINT8_T, 0, 24, offsetof(mavlink_cc_telemetry_network_t, ap_channel) }, \
         { "ap_ieee80211n", NULL, MAVLINK_TYPE_UINT8_T, 0, 25, offsetof(mavlink_cc_telemetry_network_t, ap_ieee80211n) }, \
         { "ap_wmm_enabled", NULL, MAVLINK_TYPE_UINT8_T, 0, 26, offsetof(mavlink_cc_telemetry_network_t, ap_wmm_enabled) }, \
         { "ap_wpa", NULL, MAVLINK_TYPE_UINT8_T, 0, 27, offsetof(mavlink_cc_telemetry_network_t, ap_wpa) }, \
         { "ap_client_count", NULL, MAVLINK_TYPE_UINT8_T, 0, 28, offsetof(mavlink_cc_telemetry_network_t, ap_client_count) }, \
         { "ap_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 29, offsetof(mavlink_cc_telemetry_network_t, ap_status) }, \
         { "wlan0_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_cc_telemetry_network_t, wlan0_status) }, \
         { "eth0_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 31, offsetof(mavlink_cc_telemetry_network_t, eth0_status) }, \
         { "eth0_is_static", NULL, MAVLINK_TYPE_UINT8_T, 0, 32, offsetof(mavlink_cc_telemetry_network_t, eth0_is_static) }, \
         { "wlan0_dhcp", NULL, MAVLINK_TYPE_UINT8_T, 0, 33, offsetof(mavlink_cc_telemetry_network_t, wlan0_dhcp) }, \
         { "dnsmasq_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 34, offsetof(mavlink_cc_telemetry_network_t, dnsmasq_status) }, \
         { "wlan0_rssi", NULL, MAVLINK_TYPE_INT8_T, 0, 35, offsetof(mavlink_cc_telemetry_network_t, wlan0_rssi) }, \
         { "eth0_ip", NULL, MAVLINK_TYPE_CHAR, 16, 36, offsetof(mavlink_cc_telemetry_network_t, eth0_ip) }, \
         { "eth0_netmask", NULL, MAVLINK_TYPE_CHAR, 16, 52, offsetof(mavlink_cc_telemetry_network_t, eth0_netmask) }, \
         { "wlan0_ip", NULL, MAVLINK_TYPE_CHAR, 16, 68, offsetof(mavlink_cc_telemetry_network_t, wlan0_ip) }, \
         { "wlan0_netmask", NULL, MAVLINK_TYPE_CHAR, 16, 84, offsetof(mavlink_cc_telemetry_network_t, wlan0_netmask) }, \
         { "wlan0_ssid", NULL, MAVLINK_TYPE_CHAR, 32, 100, offsetof(mavlink_cc_telemetry_network_t, wlan0_ssid) }, \
         { "ap_ip", NULL, MAVLINK_TYPE_CHAR, 16, 132, offsetof(mavlink_cc_telemetry_network_t, ap_ip) }, \
         { "ap_netmask", NULL, MAVLINK_TYPE_CHAR, 16, 148, offsetof(mavlink_cc_telemetry_network_t, ap_netmask) }, \
         { "ap_ssid", NULL, MAVLINK_TYPE_CHAR, 32, 164, offsetof(mavlink_cc_telemetry_network_t, ap_ssid) }, \
         { "ap_wpa_passphrase", NULL, MAVLINK_TYPE_CHAR, 16, 196, offsetof(mavlink_cc_telemetry_network_t, ap_wpa_passphrase) }, \
         { "ap_key_mgmt", NULL, MAVLINK_TYPE_CHAR, 12, 212, offsetof(mavlink_cc_telemetry_network_t, ap_key_mgmt) }, \
         { "ap_hw_mode", NULL, MAVLINK_TYPE_CHAR, 4, 224, offsetof(mavlink_cc_telemetry_network_t, ap_hw_mode) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_CC_TELEMETRY_NETWORK { \
    "CC_TELEMETRY_NETWORK", \
    29, \
    {  { "uap0_rx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_cc_telemetry_network_t, uap0_rx_kb) }, \
         { "uap0_tx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 4, offsetof(mavlink_cc_telemetry_network_t, uap0_tx_kb) }, \
         { "wlan0_rx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 8, offsetof(mavlink_cc_telemetry_network_t, wlan0_rx_kb) }, \
         { "wlan0_tx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 12, offsetof(mavlink_cc_telemetry_network_t, wlan0_tx_kb) }, \
         { "eth0_rx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 16, offsetof(mavlink_cc_telemetry_network_t, eth0_rx_kb) }, \
         { "eth0_tx_kb", NULL, MAVLINK_TYPE_UINT32_T, 0, 20, offsetof(mavlink_cc_telemetry_network_t, eth0_tx_kb) }, \
         { "ap_channel", NULL, MAVLINK_TYPE_UINT8_T, 0, 24, offsetof(mavlink_cc_telemetry_network_t, ap_channel) }, \
         { "ap_ieee80211n", NULL, MAVLINK_TYPE_UINT8_T, 0, 25, offsetof(mavlink_cc_telemetry_network_t, ap_ieee80211n) }, \
         { "ap_wmm_enabled", NULL, MAVLINK_TYPE_UINT8_T, 0, 26, offsetof(mavlink_cc_telemetry_network_t, ap_wmm_enabled) }, \
         { "ap_wpa", NULL, MAVLINK_TYPE_UINT8_T, 0, 27, offsetof(mavlink_cc_telemetry_network_t, ap_wpa) }, \
         { "ap_client_count", NULL, MAVLINK_TYPE_UINT8_T, 0, 28, offsetof(mavlink_cc_telemetry_network_t, ap_client_count) }, \
         { "ap_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 29, offsetof(mavlink_cc_telemetry_network_t, ap_status) }, \
         { "wlan0_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_cc_telemetry_network_t, wlan0_status) }, \
         { "eth0_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 31, offsetof(mavlink_cc_telemetry_network_t, eth0_status) }, \
         { "eth0_is_static", NULL, MAVLINK_TYPE_UINT8_T, 0, 32, offsetof(mavlink_cc_telemetry_network_t, eth0_is_static) }, \
         { "wlan0_dhcp", NULL, MAVLINK_TYPE_UINT8_T, 0, 33, offsetof(mavlink_cc_telemetry_network_t, wlan0_dhcp) }, \
         { "dnsmasq_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 34, offsetof(mavlink_cc_telemetry_network_t, dnsmasq_status) }, \
         { "wlan0_rssi", NULL, MAVLINK_TYPE_INT8_T, 0, 35, offsetof(mavlink_cc_telemetry_network_t, wlan0_rssi) }, \
         { "eth0_ip", NULL, MAVLINK_TYPE_CHAR, 16, 36, offsetof(mavlink_cc_telemetry_network_t, eth0_ip) }, \
         { "eth0_netmask", NULL, MAVLINK_TYPE_CHAR, 16, 52, offsetof(mavlink_cc_telemetry_network_t, eth0_netmask) }, \
         { "wlan0_ip", NULL, MAVLINK_TYPE_CHAR, 16, 68, offsetof(mavlink_cc_telemetry_network_t, wlan0_ip) }, \
         { "wlan0_netmask", NULL, MAVLINK_TYPE_CHAR, 16, 84, offsetof(mavlink_cc_telemetry_network_t, wlan0_netmask) }, \
         { "wlan0_ssid", NULL, MAVLINK_TYPE_CHAR, 32, 100, offsetof(mavlink_cc_telemetry_network_t, wlan0_ssid) }, \
         { "ap_ip", NULL, MAVLINK_TYPE_CHAR, 16, 132, offsetof(mavlink_cc_telemetry_network_t, ap_ip) }, \
         { "ap_netmask", NULL, MAVLINK_TYPE_CHAR, 16, 148, offsetof(mavlink_cc_telemetry_network_t, ap_netmask) }, \
         { "ap_ssid", NULL, MAVLINK_TYPE_CHAR, 32, 164, offsetof(mavlink_cc_telemetry_network_t, ap_ssid) }, \
         { "ap_wpa_passphrase", NULL, MAVLINK_TYPE_CHAR, 16, 196, offsetof(mavlink_cc_telemetry_network_t, ap_wpa_passphrase) }, \
         { "ap_key_mgmt", NULL, MAVLINK_TYPE_CHAR, 12, 212, offsetof(mavlink_cc_telemetry_network_t, ap_key_mgmt) }, \
         { "ap_hw_mode", NULL, MAVLINK_TYPE_CHAR, 4, 224, offsetof(mavlink_cc_telemetry_network_t, ap_hw_mode) }, \
         } \
}
#endif

/**
 * @brief Pack a cc_telemetry_network message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param uap0_rx_kb  Hotspot uap0 total received traffic in kilobytes
 * @param uap0_tx_kb  Hotspot uap0 total transmitted traffic in kilobytes
 * @param wlan0_rx_kb  WiFi client wlan0 total received traffic in kilobytes
 * @param wlan0_tx_kb  WiFi client wlan0 total transmitted traffic in kilobytes
 * @param eth0_rx_kb  Ethernet eth0 total received traffic in kilobytes
 * @param eth0_tx_kb  Ethernet eth0 total transmitted traffic in kilobytes
 * @param ap_channel  Wi-Fi AP radio channel, e.g. 6
 * @param ap_ieee80211n  802.11n high-throughput flag (1: enabled)
 * @param ap_wmm_enabled  WMM QoS flag (1: enabled)
 * @param ap_wpa  WPA security version (2: WPA2-PSK)
 * @param ap_client_count  Count of currently connected Wi-Fi clients
 * @param ap_status  Access Point hostapd operational status
 * @param wlan0_status  Wi-Fi station interface operational status
 * @param eth0_status  Ethernet interface carrier / link status
 * @param eth0_is_static  Ethernet IP configuration (1: Static IP, 0: DHCP Client)
 * @param wlan0_dhcp  Wi-Fi client DHCP client active flag
 * @param dnsmasq_status  Local DHCP/DNS server (dnsmasq) active flag
 * @param wlan0_rssi  WiFi station received signal strength in dBm, e.g. -65
 * @param eth0_ip  Ethernet interface IP address
 * @param eth0_netmask  Ethernet subnet mask, e.g. 255.255.255.0
 * @param wlan0_ip  Wi-Fi station interface IP address
 * @param wlan0_netmask  Wi-Fi station subnet mask
 * @param wlan0_ssid  Connected Wi-Fi Access Point SSID
 * @param ap_ip  Access Point interface gateway IP (uap0), e.g. 192.168.10.1
 * @param ap_netmask  Access Point subnet mask
 * @param ap_ssid  Wi-Fi Access Point SSID, e.g. AP_DRONE
 * @param ap_wpa_passphrase  Wi-Fi AP WPA passphrase (redacted if restricted)
 * @param ap_key_mgmt  Key management protocol, e.g. WPA-PSK
 * @param ap_hw_mode  Hardware radio band mode, e.g. g or a
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint32_t uap0_rx_kb, uint32_t uap0_tx_kb, uint32_t wlan0_rx_kb, uint32_t wlan0_tx_kb, uint32_t eth0_rx_kb, uint32_t eth0_tx_kb, uint8_t ap_channel, uint8_t ap_ieee80211n, uint8_t ap_wmm_enabled, uint8_t ap_wpa, uint8_t ap_client_count, uint8_t ap_status, uint8_t wlan0_status, uint8_t eth0_status, uint8_t eth0_is_static, uint8_t wlan0_dhcp, uint8_t dnsmasq_status, int8_t wlan0_rssi, const char *eth0_ip, const char *eth0_netmask, const char *wlan0_ip, const char *wlan0_netmask, const char *wlan0_ssid, const char *ap_ip, const char *ap_netmask, const char *ap_ssid, const char *ap_wpa_passphrase, const char *ap_key_mgmt, const char *ap_hw_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN];
    _mav_put_uint32_t(buf, 0, uap0_rx_kb);
    _mav_put_uint32_t(buf, 4, uap0_tx_kb);
    _mav_put_uint32_t(buf, 8, wlan0_rx_kb);
    _mav_put_uint32_t(buf, 12, wlan0_tx_kb);
    _mav_put_uint32_t(buf, 16, eth0_rx_kb);
    _mav_put_uint32_t(buf, 20, eth0_tx_kb);
    _mav_put_uint8_t(buf, 24, ap_channel);
    _mav_put_uint8_t(buf, 25, ap_ieee80211n);
    _mav_put_uint8_t(buf, 26, ap_wmm_enabled);
    _mav_put_uint8_t(buf, 27, ap_wpa);
    _mav_put_uint8_t(buf, 28, ap_client_count);
    _mav_put_uint8_t(buf, 29, ap_status);
    _mav_put_uint8_t(buf, 30, wlan0_status);
    _mav_put_uint8_t(buf, 31, eth0_status);
    _mav_put_uint8_t(buf, 32, eth0_is_static);
    _mav_put_uint8_t(buf, 33, wlan0_dhcp);
    _mav_put_uint8_t(buf, 34, dnsmasq_status);
    _mav_put_int8_t(buf, 35, wlan0_rssi);
    _mav_put_char_array(buf, 36, eth0_ip, 16);
    _mav_put_char_array(buf, 52, eth0_netmask, 16);
    _mav_put_char_array(buf, 68, wlan0_ip, 16);
    _mav_put_char_array(buf, 84, wlan0_netmask, 16);
    _mav_put_char_array(buf, 100, wlan0_ssid, 32);
    _mav_put_char_array(buf, 132, ap_ip, 16);
    _mav_put_char_array(buf, 148, ap_netmask, 16);
    _mav_put_char_array(buf, 164, ap_ssid, 32);
    _mav_put_char_array(buf, 196, ap_wpa_passphrase, 16);
    _mav_put_char_array(buf, 212, ap_key_mgmt, 12);
    _mav_put_char_array(buf, 224, ap_hw_mode, 4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN);
#else
    mavlink_cc_telemetry_network_t packet;
    packet.uap0_rx_kb = uap0_rx_kb;
    packet.uap0_tx_kb = uap0_tx_kb;
    packet.wlan0_rx_kb = wlan0_rx_kb;
    packet.wlan0_tx_kb = wlan0_tx_kb;
    packet.eth0_rx_kb = eth0_rx_kb;
    packet.eth0_tx_kb = eth0_tx_kb;
    packet.ap_channel = ap_channel;
    packet.ap_ieee80211n = ap_ieee80211n;
    packet.ap_wmm_enabled = ap_wmm_enabled;
    packet.ap_wpa = ap_wpa;
    packet.ap_client_count = ap_client_count;
    packet.ap_status = ap_status;
    packet.wlan0_status = wlan0_status;
    packet.eth0_status = eth0_status;
    packet.eth0_is_static = eth0_is_static;
    packet.wlan0_dhcp = wlan0_dhcp;
    packet.dnsmasq_status = dnsmasq_status;
    packet.wlan0_rssi = wlan0_rssi;
    mav_array_assign_char(packet.eth0_ip, eth0_ip, 16);
    mav_array_assign_char(packet.eth0_netmask, eth0_netmask, 16);
    mav_array_assign_char(packet.wlan0_ip, wlan0_ip, 16);
    mav_array_assign_char(packet.wlan0_netmask, wlan0_netmask, 16);
    mav_array_assign_char(packet.wlan0_ssid, wlan0_ssid, 32);
    mav_array_assign_char(packet.ap_ip, ap_ip, 16);
    mav_array_assign_char(packet.ap_netmask, ap_netmask, 16);
    mav_array_assign_char(packet.ap_ssid, ap_ssid, 32);
    mav_array_assign_char(packet.ap_wpa_passphrase, ap_wpa_passphrase, 16);
    mav_array_assign_char(packet.ap_key_mgmt, ap_key_mgmt, 12);
    mav_array_assign_char(packet.ap_hw_mode, ap_hw_mode, 4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_CRC);
}

/**
 * @brief Pack a cc_telemetry_network message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param uap0_rx_kb  Hotspot uap0 total received traffic in kilobytes
 * @param uap0_tx_kb  Hotspot uap0 total transmitted traffic in kilobytes
 * @param wlan0_rx_kb  WiFi client wlan0 total received traffic in kilobytes
 * @param wlan0_tx_kb  WiFi client wlan0 total transmitted traffic in kilobytes
 * @param eth0_rx_kb  Ethernet eth0 total received traffic in kilobytes
 * @param eth0_tx_kb  Ethernet eth0 total transmitted traffic in kilobytes
 * @param ap_channel  Wi-Fi AP radio channel, e.g. 6
 * @param ap_ieee80211n  802.11n high-throughput flag (1: enabled)
 * @param ap_wmm_enabled  WMM QoS flag (1: enabled)
 * @param ap_wpa  WPA security version (2: WPA2-PSK)
 * @param ap_client_count  Count of currently connected Wi-Fi clients
 * @param ap_status  Access Point hostapd operational status
 * @param wlan0_status  Wi-Fi station interface operational status
 * @param eth0_status  Ethernet interface carrier / link status
 * @param eth0_is_static  Ethernet IP configuration (1: Static IP, 0: DHCP Client)
 * @param wlan0_dhcp  Wi-Fi client DHCP client active flag
 * @param dnsmasq_status  Local DHCP/DNS server (dnsmasq) active flag
 * @param wlan0_rssi  WiFi station received signal strength in dBm, e.g. -65
 * @param eth0_ip  Ethernet interface IP address
 * @param eth0_netmask  Ethernet subnet mask, e.g. 255.255.255.0
 * @param wlan0_ip  Wi-Fi station interface IP address
 * @param wlan0_netmask  Wi-Fi station subnet mask
 * @param wlan0_ssid  Connected Wi-Fi Access Point SSID
 * @param ap_ip  Access Point interface gateway IP (uap0), e.g. 192.168.10.1
 * @param ap_netmask  Access Point subnet mask
 * @param ap_ssid  Wi-Fi Access Point SSID, e.g. AP_DRONE
 * @param ap_wpa_passphrase  Wi-Fi AP WPA passphrase (redacted if restricted)
 * @param ap_key_mgmt  Key management protocol, e.g. WPA-PSK
 * @param ap_hw_mode  Hardware radio band mode, e.g. g or a
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint32_t uap0_rx_kb, uint32_t uap0_tx_kb, uint32_t wlan0_rx_kb, uint32_t wlan0_tx_kb, uint32_t eth0_rx_kb, uint32_t eth0_tx_kb, uint8_t ap_channel, uint8_t ap_ieee80211n, uint8_t ap_wmm_enabled, uint8_t ap_wpa, uint8_t ap_client_count, uint8_t ap_status, uint8_t wlan0_status, uint8_t eth0_status, uint8_t eth0_is_static, uint8_t wlan0_dhcp, uint8_t dnsmasq_status, int8_t wlan0_rssi, const char *eth0_ip, const char *eth0_netmask, const char *wlan0_ip, const char *wlan0_netmask, const char *wlan0_ssid, const char *ap_ip, const char *ap_netmask, const char *ap_ssid, const char *ap_wpa_passphrase, const char *ap_key_mgmt, const char *ap_hw_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN];
    _mav_put_uint32_t(buf, 0, uap0_rx_kb);
    _mav_put_uint32_t(buf, 4, uap0_tx_kb);
    _mav_put_uint32_t(buf, 8, wlan0_rx_kb);
    _mav_put_uint32_t(buf, 12, wlan0_tx_kb);
    _mav_put_uint32_t(buf, 16, eth0_rx_kb);
    _mav_put_uint32_t(buf, 20, eth0_tx_kb);
    _mav_put_uint8_t(buf, 24, ap_channel);
    _mav_put_uint8_t(buf, 25, ap_ieee80211n);
    _mav_put_uint8_t(buf, 26, ap_wmm_enabled);
    _mav_put_uint8_t(buf, 27, ap_wpa);
    _mav_put_uint8_t(buf, 28, ap_client_count);
    _mav_put_uint8_t(buf, 29, ap_status);
    _mav_put_uint8_t(buf, 30, wlan0_status);
    _mav_put_uint8_t(buf, 31, eth0_status);
    _mav_put_uint8_t(buf, 32, eth0_is_static);
    _mav_put_uint8_t(buf, 33, wlan0_dhcp);
    _mav_put_uint8_t(buf, 34, dnsmasq_status);
    _mav_put_int8_t(buf, 35, wlan0_rssi);
    _mav_put_char_array(buf, 36, eth0_ip, 16);
    _mav_put_char_array(buf, 52, eth0_netmask, 16);
    _mav_put_char_array(buf, 68, wlan0_ip, 16);
    _mav_put_char_array(buf, 84, wlan0_netmask, 16);
    _mav_put_char_array(buf, 100, wlan0_ssid, 32);
    _mav_put_char_array(buf, 132, ap_ip, 16);
    _mav_put_char_array(buf, 148, ap_netmask, 16);
    _mav_put_char_array(buf, 164, ap_ssid, 32);
    _mav_put_char_array(buf, 196, ap_wpa_passphrase, 16);
    _mav_put_char_array(buf, 212, ap_key_mgmt, 12);
    _mav_put_char_array(buf, 224, ap_hw_mode, 4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN);
#else
    mavlink_cc_telemetry_network_t packet;
    packet.uap0_rx_kb = uap0_rx_kb;
    packet.uap0_tx_kb = uap0_tx_kb;
    packet.wlan0_rx_kb = wlan0_rx_kb;
    packet.wlan0_tx_kb = wlan0_tx_kb;
    packet.eth0_rx_kb = eth0_rx_kb;
    packet.eth0_tx_kb = eth0_tx_kb;
    packet.ap_channel = ap_channel;
    packet.ap_ieee80211n = ap_ieee80211n;
    packet.ap_wmm_enabled = ap_wmm_enabled;
    packet.ap_wpa = ap_wpa;
    packet.ap_client_count = ap_client_count;
    packet.ap_status = ap_status;
    packet.wlan0_status = wlan0_status;
    packet.eth0_status = eth0_status;
    packet.eth0_is_static = eth0_is_static;
    packet.wlan0_dhcp = wlan0_dhcp;
    packet.dnsmasq_status = dnsmasq_status;
    packet.wlan0_rssi = wlan0_rssi;
    mav_array_memcpy(packet.eth0_ip, eth0_ip, sizeof(char)*16);
    mav_array_memcpy(packet.eth0_netmask, eth0_netmask, sizeof(char)*16);
    mav_array_memcpy(packet.wlan0_ip, wlan0_ip, sizeof(char)*16);
    mav_array_memcpy(packet.wlan0_netmask, wlan0_netmask, sizeof(char)*16);
    mav_array_memcpy(packet.wlan0_ssid, wlan0_ssid, sizeof(char)*32);
    mav_array_memcpy(packet.ap_ip, ap_ip, sizeof(char)*16);
    mav_array_memcpy(packet.ap_netmask, ap_netmask, sizeof(char)*16);
    mav_array_memcpy(packet.ap_ssid, ap_ssid, sizeof(char)*32);
    mav_array_memcpy(packet.ap_wpa_passphrase, ap_wpa_passphrase, sizeof(char)*16);
    mav_array_memcpy(packet.ap_key_mgmt, ap_key_mgmt, sizeof(char)*12);
    mav_array_memcpy(packet.ap_hw_mode, ap_hw_mode, sizeof(char)*4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN);
#endif
}

/**
 * @brief Pack a cc_telemetry_network message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param uap0_rx_kb  Hotspot uap0 total received traffic in kilobytes
 * @param uap0_tx_kb  Hotspot uap0 total transmitted traffic in kilobytes
 * @param wlan0_rx_kb  WiFi client wlan0 total received traffic in kilobytes
 * @param wlan0_tx_kb  WiFi client wlan0 total transmitted traffic in kilobytes
 * @param eth0_rx_kb  Ethernet eth0 total received traffic in kilobytes
 * @param eth0_tx_kb  Ethernet eth0 total transmitted traffic in kilobytes
 * @param ap_channel  Wi-Fi AP radio channel, e.g. 6
 * @param ap_ieee80211n  802.11n high-throughput flag (1: enabled)
 * @param ap_wmm_enabled  WMM QoS flag (1: enabled)
 * @param ap_wpa  WPA security version (2: WPA2-PSK)
 * @param ap_client_count  Count of currently connected Wi-Fi clients
 * @param ap_status  Access Point hostapd operational status
 * @param wlan0_status  Wi-Fi station interface operational status
 * @param eth0_status  Ethernet interface carrier / link status
 * @param eth0_is_static  Ethernet IP configuration (1: Static IP, 0: DHCP Client)
 * @param wlan0_dhcp  Wi-Fi client DHCP client active flag
 * @param dnsmasq_status  Local DHCP/DNS server (dnsmasq) active flag
 * @param wlan0_rssi  WiFi station received signal strength in dBm, e.g. -65
 * @param eth0_ip  Ethernet interface IP address
 * @param eth0_netmask  Ethernet subnet mask, e.g. 255.255.255.0
 * @param wlan0_ip  Wi-Fi station interface IP address
 * @param wlan0_netmask  Wi-Fi station subnet mask
 * @param wlan0_ssid  Connected Wi-Fi Access Point SSID
 * @param ap_ip  Access Point interface gateway IP (uap0), e.g. 192.168.10.1
 * @param ap_netmask  Access Point subnet mask
 * @param ap_ssid  Wi-Fi Access Point SSID, e.g. AP_DRONE
 * @param ap_wpa_passphrase  Wi-Fi AP WPA passphrase (redacted if restricted)
 * @param ap_key_mgmt  Key management protocol, e.g. WPA-PSK
 * @param ap_hw_mode  Hardware radio band mode, e.g. g or a
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint32_t uap0_rx_kb,uint32_t uap0_tx_kb,uint32_t wlan0_rx_kb,uint32_t wlan0_tx_kb,uint32_t eth0_rx_kb,uint32_t eth0_tx_kb,uint8_t ap_channel,uint8_t ap_ieee80211n,uint8_t ap_wmm_enabled,uint8_t ap_wpa,uint8_t ap_client_count,uint8_t ap_status,uint8_t wlan0_status,uint8_t eth0_status,uint8_t eth0_is_static,uint8_t wlan0_dhcp,uint8_t dnsmasq_status,int8_t wlan0_rssi,const char *eth0_ip,const char *eth0_netmask,const char *wlan0_ip,const char *wlan0_netmask,const char *wlan0_ssid,const char *ap_ip,const char *ap_netmask,const char *ap_ssid,const char *ap_wpa_passphrase,const char *ap_key_mgmt,const char *ap_hw_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN];
    _mav_put_uint32_t(buf, 0, uap0_rx_kb);
    _mav_put_uint32_t(buf, 4, uap0_tx_kb);
    _mav_put_uint32_t(buf, 8, wlan0_rx_kb);
    _mav_put_uint32_t(buf, 12, wlan0_tx_kb);
    _mav_put_uint32_t(buf, 16, eth0_rx_kb);
    _mav_put_uint32_t(buf, 20, eth0_tx_kb);
    _mav_put_uint8_t(buf, 24, ap_channel);
    _mav_put_uint8_t(buf, 25, ap_ieee80211n);
    _mav_put_uint8_t(buf, 26, ap_wmm_enabled);
    _mav_put_uint8_t(buf, 27, ap_wpa);
    _mav_put_uint8_t(buf, 28, ap_client_count);
    _mav_put_uint8_t(buf, 29, ap_status);
    _mav_put_uint8_t(buf, 30, wlan0_status);
    _mav_put_uint8_t(buf, 31, eth0_status);
    _mav_put_uint8_t(buf, 32, eth0_is_static);
    _mav_put_uint8_t(buf, 33, wlan0_dhcp);
    _mav_put_uint8_t(buf, 34, dnsmasq_status);
    _mav_put_int8_t(buf, 35, wlan0_rssi);
    _mav_put_char_array(buf, 36, eth0_ip, 16);
    _mav_put_char_array(buf, 52, eth0_netmask, 16);
    _mav_put_char_array(buf, 68, wlan0_ip, 16);
    _mav_put_char_array(buf, 84, wlan0_netmask, 16);
    _mav_put_char_array(buf, 100, wlan0_ssid, 32);
    _mav_put_char_array(buf, 132, ap_ip, 16);
    _mav_put_char_array(buf, 148, ap_netmask, 16);
    _mav_put_char_array(buf, 164, ap_ssid, 32);
    _mav_put_char_array(buf, 196, ap_wpa_passphrase, 16);
    _mav_put_char_array(buf, 212, ap_key_mgmt, 12);
    _mav_put_char_array(buf, 224, ap_hw_mode, 4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN);
#else
    mavlink_cc_telemetry_network_t packet;
    packet.uap0_rx_kb = uap0_rx_kb;
    packet.uap0_tx_kb = uap0_tx_kb;
    packet.wlan0_rx_kb = wlan0_rx_kb;
    packet.wlan0_tx_kb = wlan0_tx_kb;
    packet.eth0_rx_kb = eth0_rx_kb;
    packet.eth0_tx_kb = eth0_tx_kb;
    packet.ap_channel = ap_channel;
    packet.ap_ieee80211n = ap_ieee80211n;
    packet.ap_wmm_enabled = ap_wmm_enabled;
    packet.ap_wpa = ap_wpa;
    packet.ap_client_count = ap_client_count;
    packet.ap_status = ap_status;
    packet.wlan0_status = wlan0_status;
    packet.eth0_status = eth0_status;
    packet.eth0_is_static = eth0_is_static;
    packet.wlan0_dhcp = wlan0_dhcp;
    packet.dnsmasq_status = dnsmasq_status;
    packet.wlan0_rssi = wlan0_rssi;
    mav_array_assign_char(packet.eth0_ip, eth0_ip, 16);
    mav_array_assign_char(packet.eth0_netmask, eth0_netmask, 16);
    mav_array_assign_char(packet.wlan0_ip, wlan0_ip, 16);
    mav_array_assign_char(packet.wlan0_netmask, wlan0_netmask, 16);
    mav_array_assign_char(packet.wlan0_ssid, wlan0_ssid, 32);
    mav_array_assign_char(packet.ap_ip, ap_ip, 16);
    mav_array_assign_char(packet.ap_netmask, ap_netmask, 16);
    mav_array_assign_char(packet.ap_ssid, ap_ssid, 32);
    mav_array_assign_char(packet.ap_wpa_passphrase, ap_wpa_passphrase, 16);
    mav_array_assign_char(packet.ap_key_mgmt, ap_key_mgmt, 12);
    mav_array_assign_char(packet.ap_hw_mode, ap_hw_mode, 4);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_CRC);
}

/**
 * @brief Encode a cc_telemetry_network struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_network C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_cc_telemetry_network_t* cc_telemetry_network)
{
    return mavlink_msg_cc_telemetry_network_pack(system_id, component_id, msg, cc_telemetry_network->uap0_rx_kb, cc_telemetry_network->uap0_tx_kb, cc_telemetry_network->wlan0_rx_kb, cc_telemetry_network->wlan0_tx_kb, cc_telemetry_network->eth0_rx_kb, cc_telemetry_network->eth0_tx_kb, cc_telemetry_network->ap_channel, cc_telemetry_network->ap_ieee80211n, cc_telemetry_network->ap_wmm_enabled, cc_telemetry_network->ap_wpa, cc_telemetry_network->ap_client_count, cc_telemetry_network->ap_status, cc_telemetry_network->wlan0_status, cc_telemetry_network->eth0_status, cc_telemetry_network->eth0_is_static, cc_telemetry_network->wlan0_dhcp, cc_telemetry_network->dnsmasq_status, cc_telemetry_network->wlan0_rssi, cc_telemetry_network->eth0_ip, cc_telemetry_network->eth0_netmask, cc_telemetry_network->wlan0_ip, cc_telemetry_network->wlan0_netmask, cc_telemetry_network->wlan0_ssid, cc_telemetry_network->ap_ip, cc_telemetry_network->ap_netmask, cc_telemetry_network->ap_ssid, cc_telemetry_network->ap_wpa_passphrase, cc_telemetry_network->ap_key_mgmt, cc_telemetry_network->ap_hw_mode);
}

/**
 * @brief Encode a cc_telemetry_network struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_network C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_cc_telemetry_network_t* cc_telemetry_network)
{
    return mavlink_msg_cc_telemetry_network_pack_chan(system_id, component_id, chan, msg, cc_telemetry_network->uap0_rx_kb, cc_telemetry_network->uap0_tx_kb, cc_telemetry_network->wlan0_rx_kb, cc_telemetry_network->wlan0_tx_kb, cc_telemetry_network->eth0_rx_kb, cc_telemetry_network->eth0_tx_kb, cc_telemetry_network->ap_channel, cc_telemetry_network->ap_ieee80211n, cc_telemetry_network->ap_wmm_enabled, cc_telemetry_network->ap_wpa, cc_telemetry_network->ap_client_count, cc_telemetry_network->ap_status, cc_telemetry_network->wlan0_status, cc_telemetry_network->eth0_status, cc_telemetry_network->eth0_is_static, cc_telemetry_network->wlan0_dhcp, cc_telemetry_network->dnsmasq_status, cc_telemetry_network->wlan0_rssi, cc_telemetry_network->eth0_ip, cc_telemetry_network->eth0_netmask, cc_telemetry_network->wlan0_ip, cc_telemetry_network->wlan0_netmask, cc_telemetry_network->wlan0_ssid, cc_telemetry_network->ap_ip, cc_telemetry_network->ap_netmask, cc_telemetry_network->ap_ssid, cc_telemetry_network->ap_wpa_passphrase, cc_telemetry_network->ap_key_mgmt, cc_telemetry_network->ap_hw_mode);
}

/**
 * @brief Encode a cc_telemetry_network struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param cc_telemetry_network C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_cc_telemetry_network_t* cc_telemetry_network)
{
    return mavlink_msg_cc_telemetry_network_pack_status(system_id, component_id, _status, msg,  cc_telemetry_network->uap0_rx_kb, cc_telemetry_network->uap0_tx_kb, cc_telemetry_network->wlan0_rx_kb, cc_telemetry_network->wlan0_tx_kb, cc_telemetry_network->eth0_rx_kb, cc_telemetry_network->eth0_tx_kb, cc_telemetry_network->ap_channel, cc_telemetry_network->ap_ieee80211n, cc_telemetry_network->ap_wmm_enabled, cc_telemetry_network->ap_wpa, cc_telemetry_network->ap_client_count, cc_telemetry_network->ap_status, cc_telemetry_network->wlan0_status, cc_telemetry_network->eth0_status, cc_telemetry_network->eth0_is_static, cc_telemetry_network->wlan0_dhcp, cc_telemetry_network->dnsmasq_status, cc_telemetry_network->wlan0_rssi, cc_telemetry_network->eth0_ip, cc_telemetry_network->eth0_netmask, cc_telemetry_network->wlan0_ip, cc_telemetry_network->wlan0_netmask, cc_telemetry_network->wlan0_ssid, cc_telemetry_network->ap_ip, cc_telemetry_network->ap_netmask, cc_telemetry_network->ap_ssid, cc_telemetry_network->ap_wpa_passphrase, cc_telemetry_network->ap_key_mgmt, cc_telemetry_network->ap_hw_mode);
}

/**
 * @brief Send a cc_telemetry_network message
 * @param chan MAVLink channel to send the message
 *
 * @param uap0_rx_kb  Hotspot uap0 total received traffic in kilobytes
 * @param uap0_tx_kb  Hotspot uap0 total transmitted traffic in kilobytes
 * @param wlan0_rx_kb  WiFi client wlan0 total received traffic in kilobytes
 * @param wlan0_tx_kb  WiFi client wlan0 total transmitted traffic in kilobytes
 * @param eth0_rx_kb  Ethernet eth0 total received traffic in kilobytes
 * @param eth0_tx_kb  Ethernet eth0 total transmitted traffic in kilobytes
 * @param ap_channel  Wi-Fi AP radio channel, e.g. 6
 * @param ap_ieee80211n  802.11n high-throughput flag (1: enabled)
 * @param ap_wmm_enabled  WMM QoS flag (1: enabled)
 * @param ap_wpa  WPA security version (2: WPA2-PSK)
 * @param ap_client_count  Count of currently connected Wi-Fi clients
 * @param ap_status  Access Point hostapd operational status
 * @param wlan0_status  Wi-Fi station interface operational status
 * @param eth0_status  Ethernet interface carrier / link status
 * @param eth0_is_static  Ethernet IP configuration (1: Static IP, 0: DHCP Client)
 * @param wlan0_dhcp  Wi-Fi client DHCP client active flag
 * @param dnsmasq_status  Local DHCP/DNS server (dnsmasq) active flag
 * @param wlan0_rssi  WiFi station received signal strength in dBm, e.g. -65
 * @param eth0_ip  Ethernet interface IP address
 * @param eth0_netmask  Ethernet subnet mask, e.g. 255.255.255.0
 * @param wlan0_ip  Wi-Fi station interface IP address
 * @param wlan0_netmask  Wi-Fi station subnet mask
 * @param wlan0_ssid  Connected Wi-Fi Access Point SSID
 * @param ap_ip  Access Point interface gateway IP (uap0), e.g. 192.168.10.1
 * @param ap_netmask  Access Point subnet mask
 * @param ap_ssid  Wi-Fi Access Point SSID, e.g. AP_DRONE
 * @param ap_wpa_passphrase  Wi-Fi AP WPA passphrase (redacted if restricted)
 * @param ap_key_mgmt  Key management protocol, e.g. WPA-PSK
 * @param ap_hw_mode  Hardware radio band mode, e.g. g or a
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_cc_telemetry_network_send(mavlink_channel_t chan, uint32_t uap0_rx_kb, uint32_t uap0_tx_kb, uint32_t wlan0_rx_kb, uint32_t wlan0_tx_kb, uint32_t eth0_rx_kb, uint32_t eth0_tx_kb, uint8_t ap_channel, uint8_t ap_ieee80211n, uint8_t ap_wmm_enabled, uint8_t ap_wpa, uint8_t ap_client_count, uint8_t ap_status, uint8_t wlan0_status, uint8_t eth0_status, uint8_t eth0_is_static, uint8_t wlan0_dhcp, uint8_t dnsmasq_status, int8_t wlan0_rssi, const char *eth0_ip, const char *eth0_netmask, const char *wlan0_ip, const char *wlan0_netmask, const char *wlan0_ssid, const char *ap_ip, const char *ap_netmask, const char *ap_ssid, const char *ap_wpa_passphrase, const char *ap_key_mgmt, const char *ap_hw_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN];
    _mav_put_uint32_t(buf, 0, uap0_rx_kb);
    _mav_put_uint32_t(buf, 4, uap0_tx_kb);
    _mav_put_uint32_t(buf, 8, wlan0_rx_kb);
    _mav_put_uint32_t(buf, 12, wlan0_tx_kb);
    _mav_put_uint32_t(buf, 16, eth0_rx_kb);
    _mav_put_uint32_t(buf, 20, eth0_tx_kb);
    _mav_put_uint8_t(buf, 24, ap_channel);
    _mav_put_uint8_t(buf, 25, ap_ieee80211n);
    _mav_put_uint8_t(buf, 26, ap_wmm_enabled);
    _mav_put_uint8_t(buf, 27, ap_wpa);
    _mav_put_uint8_t(buf, 28, ap_client_count);
    _mav_put_uint8_t(buf, 29, ap_status);
    _mav_put_uint8_t(buf, 30, wlan0_status);
    _mav_put_uint8_t(buf, 31, eth0_status);
    _mav_put_uint8_t(buf, 32, eth0_is_static);
    _mav_put_uint8_t(buf, 33, wlan0_dhcp);
    _mav_put_uint8_t(buf, 34, dnsmasq_status);
    _mav_put_int8_t(buf, 35, wlan0_rssi);
    _mav_put_char_array(buf, 36, eth0_ip, 16);
    _mav_put_char_array(buf, 52, eth0_netmask, 16);
    _mav_put_char_array(buf, 68, wlan0_ip, 16);
    _mav_put_char_array(buf, 84, wlan0_netmask, 16);
    _mav_put_char_array(buf, 100, wlan0_ssid, 32);
    _mav_put_char_array(buf, 132, ap_ip, 16);
    _mav_put_char_array(buf, 148, ap_netmask, 16);
    _mav_put_char_array(buf, 164, ap_ssid, 32);
    _mav_put_char_array(buf, 196, ap_wpa_passphrase, 16);
    _mav_put_char_array(buf, 212, ap_key_mgmt, 12);
    _mav_put_char_array(buf, 224, ap_hw_mode, 4);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK, buf, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_CRC);
#else
    mavlink_cc_telemetry_network_t packet;
    packet.uap0_rx_kb = uap0_rx_kb;
    packet.uap0_tx_kb = uap0_tx_kb;
    packet.wlan0_rx_kb = wlan0_rx_kb;
    packet.wlan0_tx_kb = wlan0_tx_kb;
    packet.eth0_rx_kb = eth0_rx_kb;
    packet.eth0_tx_kb = eth0_tx_kb;
    packet.ap_channel = ap_channel;
    packet.ap_ieee80211n = ap_ieee80211n;
    packet.ap_wmm_enabled = ap_wmm_enabled;
    packet.ap_wpa = ap_wpa;
    packet.ap_client_count = ap_client_count;
    packet.ap_status = ap_status;
    packet.wlan0_status = wlan0_status;
    packet.eth0_status = eth0_status;
    packet.eth0_is_static = eth0_is_static;
    packet.wlan0_dhcp = wlan0_dhcp;
    packet.dnsmasq_status = dnsmasq_status;
    packet.wlan0_rssi = wlan0_rssi;
    mav_array_assign_char(packet.eth0_ip, eth0_ip, 16);
    mav_array_assign_char(packet.eth0_netmask, eth0_netmask, 16);
    mav_array_assign_char(packet.wlan0_ip, wlan0_ip, 16);
    mav_array_assign_char(packet.wlan0_netmask, wlan0_netmask, 16);
    mav_array_assign_char(packet.wlan0_ssid, wlan0_ssid, 32);
    mav_array_assign_char(packet.ap_ip, ap_ip, 16);
    mav_array_assign_char(packet.ap_netmask, ap_netmask, 16);
    mav_array_assign_char(packet.ap_ssid, ap_ssid, 32);
    mav_array_assign_char(packet.ap_wpa_passphrase, ap_wpa_passphrase, 16);
    mav_array_assign_char(packet.ap_key_mgmt, ap_key_mgmt, 12);
    mav_array_assign_char(packet.ap_hw_mode, ap_hw_mode, 4);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK, (const char *)&packet, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_CRC);
#endif
}

/**
 * @brief Send a cc_telemetry_network message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_cc_telemetry_network_send_struct(mavlink_channel_t chan, const mavlink_cc_telemetry_network_t* cc_telemetry_network)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_cc_telemetry_network_send(chan, cc_telemetry_network->uap0_rx_kb, cc_telemetry_network->uap0_tx_kb, cc_telemetry_network->wlan0_rx_kb, cc_telemetry_network->wlan0_tx_kb, cc_telemetry_network->eth0_rx_kb, cc_telemetry_network->eth0_tx_kb, cc_telemetry_network->ap_channel, cc_telemetry_network->ap_ieee80211n, cc_telemetry_network->ap_wmm_enabled, cc_telemetry_network->ap_wpa, cc_telemetry_network->ap_client_count, cc_telemetry_network->ap_status, cc_telemetry_network->wlan0_status, cc_telemetry_network->eth0_status, cc_telemetry_network->eth0_is_static, cc_telemetry_network->wlan0_dhcp, cc_telemetry_network->dnsmasq_status, cc_telemetry_network->wlan0_rssi, cc_telemetry_network->eth0_ip, cc_telemetry_network->eth0_netmask, cc_telemetry_network->wlan0_ip, cc_telemetry_network->wlan0_netmask, cc_telemetry_network->wlan0_ssid, cc_telemetry_network->ap_ip, cc_telemetry_network->ap_netmask, cc_telemetry_network->ap_ssid, cc_telemetry_network->ap_wpa_passphrase, cc_telemetry_network->ap_key_mgmt, cc_telemetry_network->ap_hw_mode);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK, (const char *)cc_telemetry_network, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_CRC);
#endif
}

#if MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by reusing
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_cc_telemetry_network_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint32_t uap0_rx_kb, uint32_t uap0_tx_kb, uint32_t wlan0_rx_kb, uint32_t wlan0_tx_kb, uint32_t eth0_rx_kb, uint32_t eth0_tx_kb, uint8_t ap_channel, uint8_t ap_ieee80211n, uint8_t ap_wmm_enabled, uint8_t ap_wpa, uint8_t ap_client_count, uint8_t ap_status, uint8_t wlan0_status, uint8_t eth0_status, uint8_t eth0_is_static, uint8_t wlan0_dhcp, uint8_t dnsmasq_status, int8_t wlan0_rssi, const char *eth0_ip, const char *eth0_netmask, const char *wlan0_ip, const char *wlan0_netmask, const char *wlan0_ssid, const char *ap_ip, const char *ap_netmask, const char *ap_ssid, const char *ap_wpa_passphrase, const char *ap_key_mgmt, const char *ap_hw_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint32_t(buf, 0, uap0_rx_kb);
    _mav_put_uint32_t(buf, 4, uap0_tx_kb);
    _mav_put_uint32_t(buf, 8, wlan0_rx_kb);
    _mav_put_uint32_t(buf, 12, wlan0_tx_kb);
    _mav_put_uint32_t(buf, 16, eth0_rx_kb);
    _mav_put_uint32_t(buf, 20, eth0_tx_kb);
    _mav_put_uint8_t(buf, 24, ap_channel);
    _mav_put_uint8_t(buf, 25, ap_ieee80211n);
    _mav_put_uint8_t(buf, 26, ap_wmm_enabled);
    _mav_put_uint8_t(buf, 27, ap_wpa);
    _mav_put_uint8_t(buf, 28, ap_client_count);
    _mav_put_uint8_t(buf, 29, ap_status);
    _mav_put_uint8_t(buf, 30, wlan0_status);
    _mav_put_uint8_t(buf, 31, eth0_status);
    _mav_put_uint8_t(buf, 32, eth0_is_static);
    _mav_put_uint8_t(buf, 33, wlan0_dhcp);
    _mav_put_uint8_t(buf, 34, dnsmasq_status);
    _mav_put_int8_t(buf, 35, wlan0_rssi);
    _mav_put_char_array(buf, 36, eth0_ip, 16);
    _mav_put_char_array(buf, 52, eth0_netmask, 16);
    _mav_put_char_array(buf, 68, wlan0_ip, 16);
    _mav_put_char_array(buf, 84, wlan0_netmask, 16);
    _mav_put_char_array(buf, 100, wlan0_ssid, 32);
    _mav_put_char_array(buf, 132, ap_ip, 16);
    _mav_put_char_array(buf, 148, ap_netmask, 16);
    _mav_put_char_array(buf, 164, ap_ssid, 32);
    _mav_put_char_array(buf, 196, ap_wpa_passphrase, 16);
    _mav_put_char_array(buf, 212, ap_key_mgmt, 12);
    _mav_put_char_array(buf, 224, ap_hw_mode, 4);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK, buf, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_CRC);
#else
    mavlink_cc_telemetry_network_t *packet = (mavlink_cc_telemetry_network_t *)msgbuf;
    packet->uap0_rx_kb = uap0_rx_kb;
    packet->uap0_tx_kb = uap0_tx_kb;
    packet->wlan0_rx_kb = wlan0_rx_kb;
    packet->wlan0_tx_kb = wlan0_tx_kb;
    packet->eth0_rx_kb = eth0_rx_kb;
    packet->eth0_tx_kb = eth0_tx_kb;
    packet->ap_channel = ap_channel;
    packet->ap_ieee80211n = ap_ieee80211n;
    packet->ap_wmm_enabled = ap_wmm_enabled;
    packet->ap_wpa = ap_wpa;
    packet->ap_client_count = ap_client_count;
    packet->ap_status = ap_status;
    packet->wlan0_status = wlan0_status;
    packet->eth0_status = eth0_status;
    packet->eth0_is_static = eth0_is_static;
    packet->wlan0_dhcp = wlan0_dhcp;
    packet->dnsmasq_status = dnsmasq_status;
    packet->wlan0_rssi = wlan0_rssi;
    mav_array_assign_char(packet->eth0_ip, eth0_ip, 16);
    mav_array_assign_char(packet->eth0_netmask, eth0_netmask, 16);
    mav_array_assign_char(packet->wlan0_ip, wlan0_ip, 16);
    mav_array_assign_char(packet->wlan0_netmask, wlan0_netmask, 16);
    mav_array_assign_char(packet->wlan0_ssid, wlan0_ssid, 32);
    mav_array_assign_char(packet->ap_ip, ap_ip, 16);
    mav_array_assign_char(packet->ap_netmask, ap_netmask, 16);
    mav_array_assign_char(packet->ap_ssid, ap_ssid, 32);
    mav_array_assign_char(packet->ap_wpa_passphrase, ap_wpa_passphrase, 16);
    mav_array_assign_char(packet->ap_key_mgmt, ap_key_mgmt, 12);
    mav_array_assign_char(packet->ap_hw_mode, ap_hw_mode, 4);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK, (const char *)packet, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_CRC);
#endif
}
#endif

#endif

// MESSAGE CC_TELEMETRY_NETWORK UNPACKING


/**
 * @brief Get field uap0_rx_kb from cc_telemetry_network message
 *
 * @return  Hotspot uap0 total received traffic in kilobytes
 */
static inline uint32_t mavlink_msg_cc_telemetry_network_get_uap0_rx_kb(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field uap0_tx_kb from cc_telemetry_network message
 *
 * @return  Hotspot uap0 total transmitted traffic in kilobytes
 */
static inline uint32_t mavlink_msg_cc_telemetry_network_get_uap0_tx_kb(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  4);
}

/**
 * @brief Get field wlan0_rx_kb from cc_telemetry_network message
 *
 * @return  WiFi client wlan0 total received traffic in kilobytes
 */
static inline uint32_t mavlink_msg_cc_telemetry_network_get_wlan0_rx_kb(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  8);
}

/**
 * @brief Get field wlan0_tx_kb from cc_telemetry_network message
 *
 * @return  WiFi client wlan0 total transmitted traffic in kilobytes
 */
static inline uint32_t mavlink_msg_cc_telemetry_network_get_wlan0_tx_kb(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  12);
}

/**
 * @brief Get field eth0_rx_kb from cc_telemetry_network message
 *
 * @return  Ethernet eth0 total received traffic in kilobytes
 */
static inline uint32_t mavlink_msg_cc_telemetry_network_get_eth0_rx_kb(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  16);
}

/**
 * @brief Get field eth0_tx_kb from cc_telemetry_network message
 *
 * @return  Ethernet eth0 total transmitted traffic in kilobytes
 */
static inline uint32_t mavlink_msg_cc_telemetry_network_get_eth0_tx_kb(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint32_t(msg,  20);
}

/**
 * @brief Get field ap_channel from cc_telemetry_network message
 *
 * @return  Wi-Fi AP radio channel, e.g. 6
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_ap_channel(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  24);
}

/**
 * @brief Get field ap_ieee80211n from cc_telemetry_network message
 *
 * @return  802.11n high-throughput flag (1: enabled)
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_ap_ieee80211n(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  25);
}

/**
 * @brief Get field ap_wmm_enabled from cc_telemetry_network message
 *
 * @return  WMM QoS flag (1: enabled)
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_ap_wmm_enabled(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  26);
}

/**
 * @brief Get field ap_wpa from cc_telemetry_network message
 *
 * @return  WPA security version (2: WPA2-PSK)
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_ap_wpa(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  27);
}

/**
 * @brief Get field ap_client_count from cc_telemetry_network message
 *
 * @return  Count of currently connected Wi-Fi clients
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_ap_client_count(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  28);
}

/**
 * @brief Get field ap_status from cc_telemetry_network message
 *
 * @return  Access Point hostapd operational status
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_ap_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  29);
}

/**
 * @brief Get field wlan0_status from cc_telemetry_network message
 *
 * @return  Wi-Fi station interface operational status
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_wlan0_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  30);
}

/**
 * @brief Get field eth0_status from cc_telemetry_network message
 *
 * @return  Ethernet interface carrier / link status
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_eth0_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  31);
}

/**
 * @brief Get field eth0_is_static from cc_telemetry_network message
 *
 * @return  Ethernet IP configuration (1: Static IP, 0: DHCP Client)
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_eth0_is_static(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  32);
}

/**
 * @brief Get field wlan0_dhcp from cc_telemetry_network message
 *
 * @return  Wi-Fi client DHCP client active flag
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_wlan0_dhcp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  33);
}

/**
 * @brief Get field dnsmasq_status from cc_telemetry_network message
 *
 * @return  Local DHCP/DNS server (dnsmasq) active flag
 */
static inline uint8_t mavlink_msg_cc_telemetry_network_get_dnsmasq_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  34);
}

/**
 * @brief Get field wlan0_rssi from cc_telemetry_network message
 *
 * @return  WiFi station received signal strength in dBm, e.g. -65
 */
static inline int8_t mavlink_msg_cc_telemetry_network_get_wlan0_rssi(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  35);
}

/**
 * @brief Get field eth0_ip from cc_telemetry_network message
 *
 * @return  Ethernet interface IP address
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_eth0_ip(const mavlink_message_t* msg, char *eth0_ip)
{
    return _MAV_RETURN_char_array(msg, eth0_ip, 16,  36);
}

/**
 * @brief Get field eth0_netmask from cc_telemetry_network message
 *
 * @return  Ethernet subnet mask, e.g. 255.255.255.0
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_eth0_netmask(const mavlink_message_t* msg, char *eth0_netmask)
{
    return _MAV_RETURN_char_array(msg, eth0_netmask, 16,  52);
}

/**
 * @brief Get field wlan0_ip from cc_telemetry_network message
 *
 * @return  Wi-Fi station interface IP address
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_wlan0_ip(const mavlink_message_t* msg, char *wlan0_ip)
{
    return _MAV_RETURN_char_array(msg, wlan0_ip, 16,  68);
}

/**
 * @brief Get field wlan0_netmask from cc_telemetry_network message
 *
 * @return  Wi-Fi station subnet mask
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_wlan0_netmask(const mavlink_message_t* msg, char *wlan0_netmask)
{
    return _MAV_RETURN_char_array(msg, wlan0_netmask, 16,  84);
}

/**
 * @brief Get field wlan0_ssid from cc_telemetry_network message
 *
 * @return  Connected Wi-Fi Access Point SSID
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_wlan0_ssid(const mavlink_message_t* msg, char *wlan0_ssid)
{
    return _MAV_RETURN_char_array(msg, wlan0_ssid, 32,  100);
}

/**
 * @brief Get field ap_ip from cc_telemetry_network message
 *
 * @return  Access Point interface gateway IP (uap0), e.g. 192.168.10.1
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_ap_ip(const mavlink_message_t* msg, char *ap_ip)
{
    return _MAV_RETURN_char_array(msg, ap_ip, 16,  132);
}

/**
 * @brief Get field ap_netmask from cc_telemetry_network message
 *
 * @return  Access Point subnet mask
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_ap_netmask(const mavlink_message_t* msg, char *ap_netmask)
{
    return _MAV_RETURN_char_array(msg, ap_netmask, 16,  148);
}

/**
 * @brief Get field ap_ssid from cc_telemetry_network message
 *
 * @return  Wi-Fi Access Point SSID, e.g. AP_DRONE
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_ap_ssid(const mavlink_message_t* msg, char *ap_ssid)
{
    return _MAV_RETURN_char_array(msg, ap_ssid, 32,  164);
}

/**
 * @brief Get field ap_wpa_passphrase from cc_telemetry_network message
 *
 * @return  Wi-Fi AP WPA passphrase (redacted if restricted)
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_ap_wpa_passphrase(const mavlink_message_t* msg, char *ap_wpa_passphrase)
{
    return _MAV_RETURN_char_array(msg, ap_wpa_passphrase, 16,  196);
}

/**
 * @brief Get field ap_key_mgmt from cc_telemetry_network message
 *
 * @return  Key management protocol, e.g. WPA-PSK
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_ap_key_mgmt(const mavlink_message_t* msg, char *ap_key_mgmt)
{
    return _MAV_RETURN_char_array(msg, ap_key_mgmt, 12,  212);
}

/**
 * @brief Get field ap_hw_mode from cc_telemetry_network message
 *
 * @return  Hardware radio band mode, e.g. g or a
 */
static inline uint16_t mavlink_msg_cc_telemetry_network_get_ap_hw_mode(const mavlink_message_t* msg, char *ap_hw_mode)
{
    return _MAV_RETURN_char_array(msg, ap_hw_mode, 4,  224);
}

/**
 * @brief Decode a cc_telemetry_network message into a struct
 *
 * @param msg The message to decode
 * @param cc_telemetry_network C-struct to decode the message contents into
 */
static inline void mavlink_msg_cc_telemetry_network_decode(const mavlink_message_t* msg, mavlink_cc_telemetry_network_t* cc_telemetry_network)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    cc_telemetry_network->uap0_rx_kb = mavlink_msg_cc_telemetry_network_get_uap0_rx_kb(msg);
    cc_telemetry_network->uap0_tx_kb = mavlink_msg_cc_telemetry_network_get_uap0_tx_kb(msg);
    cc_telemetry_network->wlan0_rx_kb = mavlink_msg_cc_telemetry_network_get_wlan0_rx_kb(msg);
    cc_telemetry_network->wlan0_tx_kb = mavlink_msg_cc_telemetry_network_get_wlan0_tx_kb(msg);
    cc_telemetry_network->eth0_rx_kb = mavlink_msg_cc_telemetry_network_get_eth0_rx_kb(msg);
    cc_telemetry_network->eth0_tx_kb = mavlink_msg_cc_telemetry_network_get_eth0_tx_kb(msg);
    cc_telemetry_network->ap_channel = mavlink_msg_cc_telemetry_network_get_ap_channel(msg);
    cc_telemetry_network->ap_ieee80211n = mavlink_msg_cc_telemetry_network_get_ap_ieee80211n(msg);
    cc_telemetry_network->ap_wmm_enabled = mavlink_msg_cc_telemetry_network_get_ap_wmm_enabled(msg);
    cc_telemetry_network->ap_wpa = mavlink_msg_cc_telemetry_network_get_ap_wpa(msg);
    cc_telemetry_network->ap_client_count = mavlink_msg_cc_telemetry_network_get_ap_client_count(msg);
    cc_telemetry_network->ap_status = mavlink_msg_cc_telemetry_network_get_ap_status(msg);
    cc_telemetry_network->wlan0_status = mavlink_msg_cc_telemetry_network_get_wlan0_status(msg);
    cc_telemetry_network->eth0_status = mavlink_msg_cc_telemetry_network_get_eth0_status(msg);
    cc_telemetry_network->eth0_is_static = mavlink_msg_cc_telemetry_network_get_eth0_is_static(msg);
    cc_telemetry_network->wlan0_dhcp = mavlink_msg_cc_telemetry_network_get_wlan0_dhcp(msg);
    cc_telemetry_network->dnsmasq_status = mavlink_msg_cc_telemetry_network_get_dnsmasq_status(msg);
    cc_telemetry_network->wlan0_rssi = mavlink_msg_cc_telemetry_network_get_wlan0_rssi(msg);
    mavlink_msg_cc_telemetry_network_get_eth0_ip(msg, cc_telemetry_network->eth0_ip);
    mavlink_msg_cc_telemetry_network_get_eth0_netmask(msg, cc_telemetry_network->eth0_netmask);
    mavlink_msg_cc_telemetry_network_get_wlan0_ip(msg, cc_telemetry_network->wlan0_ip);
    mavlink_msg_cc_telemetry_network_get_wlan0_netmask(msg, cc_telemetry_network->wlan0_netmask);
    mavlink_msg_cc_telemetry_network_get_wlan0_ssid(msg, cc_telemetry_network->wlan0_ssid);
    mavlink_msg_cc_telemetry_network_get_ap_ip(msg, cc_telemetry_network->ap_ip);
    mavlink_msg_cc_telemetry_network_get_ap_netmask(msg, cc_telemetry_network->ap_netmask);
    mavlink_msg_cc_telemetry_network_get_ap_ssid(msg, cc_telemetry_network->ap_ssid);
    mavlink_msg_cc_telemetry_network_get_ap_wpa_passphrase(msg, cc_telemetry_network->ap_wpa_passphrase);
    mavlink_msg_cc_telemetry_network_get_ap_key_mgmt(msg, cc_telemetry_network->ap_key_mgmt);
    mavlink_msg_cc_telemetry_network_get_ap_hw_mode(msg, cc_telemetry_network->ap_hw_mode);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN? msg->len : MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN;
        memset(cc_telemetry_network, 0, MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_LEN);
    memcpy(cc_telemetry_network, _MAV_PAYLOAD(msg), len);
#endif
}
