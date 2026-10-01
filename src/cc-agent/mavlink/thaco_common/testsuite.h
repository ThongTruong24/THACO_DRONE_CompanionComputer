/** @file
 *    @brief MAVLink comm protocol testsuite generated from thaco_common.xml
 *    @see https://mavlink.io/en/
 */
#pragma once
#ifndef THACO_COMMON_TESTSUITE_H
#define THACO_COMMON_TESTSUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAVLINK_TEST_ALL
#define MAVLINK_TEST_ALL
static void mavlink_test_common(uint8_t, uint8_t, mavlink_message_t *last_msg);
static void mavlink_test_thaco_common(uint8_t, uint8_t, mavlink_message_t *last_msg);

static void mavlink_test_all(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_common(system_id, component_id, last_msg);
    mavlink_test_thaco_common(system_id, component_id, last_msg);
}
#endif

#include "../common/testsuite.h"


static void mavlink_test_thaco_external_xyz_trigger(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_thaco_external_xyz_trigger_t packet_in = {
        963497464,963497672
    };
    mavlink_thaco_external_xyz_trigger_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.trigger_id = packet_in.trigger_id;
        packet1.time_boot_ms = packet_in.time_boot_ms;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_thaco_external_xyz_trigger_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_thaco_external_xyz_trigger_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_thaco_external_xyz_trigger_pack(system_id, component_id, &msg , packet1.trigger_id , packet1.time_boot_ms );
    mavlink_msg_thaco_external_xyz_trigger_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_thaco_external_xyz_trigger_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.trigger_id , packet1.time_boot_ms );
    mavlink_msg_thaco_external_xyz_trigger_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_thaco_external_xyz_trigger_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_thaco_external_xyz_trigger_send(MAVLINK_COMM_1 , packet1.trigger_id , packet1.time_boot_ms );
    mavlink_msg_thaco_external_xyz_trigger_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("THACO_EXTERNAL_XYZ_TRIGGER") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_THACO_EXTERNAL_XYZ_TRIGGER) != NULL);
#endif
}

static void mavlink_test_cc_telemetry_links(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_TELEMETRY_LINKS >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_telemetry_links_t packet_in = {
        17.0,45.0,73.0,101.0,129.0,963498504,963498712,963498920,963499128,269.0,297.0,325.0,353.0,381.0,963500376,963500584,963500792,963501000,221,32,99,"XYZABCDEFGHIJKL","NOPQRSTUVWXYZAB","DEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX"
    };
    mavlink_cc_telemetry_links_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.fc_tx_rate = packet_in.fc_tx_rate;
        packet1.fc_rx_rate = packet_in.fc_rx_rate;
        packet1.fc_tx_rate_max = packet_in.fc_tx_rate_max;
        packet1.fc_tx_rate_multi = packet_in.fc_tx_rate_multi;
        packet1.fc_rx_loss = packet_in.fc_rx_loss;
        packet1.fc_tx_err = packet_in.fc_tx_err;
        packet1.fc_bytes_rx = packet_in.fc_bytes_rx;
        packet1.fc_bytes_tx = packet_in.fc_bytes_tx;
        packet1.fc_baudrate = packet_in.fc_baudrate;
        packet1.siyi_tx_rate = packet_in.siyi_tx_rate;
        packet1.siyi_rx_rate = packet_in.siyi_rx_rate;
        packet1.siyi_tx_rate_max = packet_in.siyi_tx_rate_max;
        packet1.siyi_tx_rate_multi = packet_in.siyi_tx_rate_multi;
        packet1.siyi_rx_loss = packet_in.siyi_rx_loss;
        packet1.siyi_tx_err = packet_in.siyi_tx_err;
        packet1.siyi_bytes_rx = packet_in.siyi_bytes_rx;
        packet1.siyi_bytes_tx = packet_in.siyi_bytes_tx;
        packet1.siyi_baudrate = packet_in.siyi_baudrate;
        packet1.fc_status = packet_in.fc_status;
        packet1.siyi_status = packet_in.siyi_status;
        packet1.transport_type = packet_in.transport_type;
        
        mav_array_memcpy(packet1.fc_port, packet_in.fc_port, sizeof(char)*16);
        mav_array_memcpy(packet1.siyi_port, packet_in.siyi_port, sizeof(char)*16);
        mav_array_memcpy(packet1.available_ports, packet_in.available_ports, sizeof(char)*48);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_TELEMETRY_LINKS_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_links_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_telemetry_links_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_links_pack(system_id, component_id, &msg , packet1.fc_tx_rate , packet1.fc_rx_rate , packet1.fc_tx_rate_max , packet1.fc_tx_rate_multi , packet1.fc_rx_loss , packet1.fc_tx_err , packet1.fc_bytes_rx , packet1.fc_bytes_tx , packet1.fc_baudrate , packet1.siyi_tx_rate , packet1.siyi_rx_rate , packet1.siyi_tx_rate_max , packet1.siyi_tx_rate_multi , packet1.siyi_rx_loss , packet1.siyi_tx_err , packet1.siyi_bytes_rx , packet1.siyi_bytes_tx , packet1.siyi_baudrate , packet1.fc_status , packet1.siyi_status , packet1.transport_type , packet1.fc_port , packet1.siyi_port , packet1.available_ports );
    mavlink_msg_cc_telemetry_links_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_links_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.fc_tx_rate , packet1.fc_rx_rate , packet1.fc_tx_rate_max , packet1.fc_tx_rate_multi , packet1.fc_rx_loss , packet1.fc_tx_err , packet1.fc_bytes_rx , packet1.fc_bytes_tx , packet1.fc_baudrate , packet1.siyi_tx_rate , packet1.siyi_rx_rate , packet1.siyi_tx_rate_max , packet1.siyi_tx_rate_multi , packet1.siyi_rx_loss , packet1.siyi_tx_err , packet1.siyi_bytes_rx , packet1.siyi_bytes_tx , packet1.siyi_baudrate , packet1.fc_status , packet1.siyi_status , packet1.transport_type , packet1.fc_port , packet1.siyi_port , packet1.available_ports );
    mavlink_msg_cc_telemetry_links_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_telemetry_links_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_links_send(MAVLINK_COMM_1 , packet1.fc_tx_rate , packet1.fc_rx_rate , packet1.fc_tx_rate_max , packet1.fc_tx_rate_multi , packet1.fc_rx_loss , packet1.fc_tx_err , packet1.fc_bytes_rx , packet1.fc_bytes_tx , packet1.fc_baudrate , packet1.siyi_tx_rate , packet1.siyi_rx_rate , packet1.siyi_tx_rate_max , packet1.siyi_tx_rate_multi , packet1.siyi_rx_loss , packet1.siyi_tx_err , packet1.siyi_bytes_rx , packet1.siyi_bytes_tx , packet1.siyi_baudrate , packet1.fc_status , packet1.siyi_status , packet1.transport_type , packet1.fc_port , packet1.siyi_port , packet1.available_ports );
    mavlink_msg_cc_telemetry_links_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_TELEMETRY_LINKS") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_TELEMETRY_LINKS) != NULL);
#endif
}

static void mavlink_test_cc_telemetry_camera(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_telemetry_camera_t packet_in = {
        17235,17339,17443,17547,17651,17755,17859,17963,18067,187,254,65,132,199,10,77,144,211,"BCDEFGHIJKLMNOPQRSTUVWX","ZABCDEFGHIJ","LMNOPQRSTUVWXYZABCD","FGHIJKLMNOPQRST","VWXYZ","BCDEF","HIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZAB","DEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX","ZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL"
    };
    mavlink_cc_telemetry_camera_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.video_width = packet_in.video_width;
        packet1.video_height = packet_in.video_height;
        packet1.depth_width = packet_in.depth_width;
        packet1.depth_height = packet_in.depth_height;
        packet1.rotation = packet_in.rotation;
        packet1.bitrate_kbps = packet_in.bitrate_kbps;
        packet1.bitrate_max_kbps = packet_in.bitrate_max_kbps;
        packet1.vbv_buffer_kb = packet_in.vbv_buffer_kb;
        packet1.rtsp_port = packet_in.rtsp_port;
        packet1.video_fps = packet_in.video_fps;
        packet1.depth_fps = packet_in.depth_fps;
        packet1.camera_id = packet_in.camera_id;
        packet1.is_default = packet_in.is_default;
        packet1.profile_mode = packet_in.profile_mode;
        packet1.enable_emitter = packet_in.enable_emitter;
        packet1.camera_status = packet_in.camera_status;
        packet1.error_code = packet_in.error_code;
        packet1.usb_speed_mode = packet_in.usb_speed_mode;
        
        mav_array_memcpy(packet1.camera_name, packet_in.camera_name, sizeof(char)*24);
        mav_array_memcpy(packet1.camera_type, packet_in.camera_type, sizeof(char)*12);
        mav_array_memcpy(packet1.connection_port, packet_in.connection_port, sizeof(char)*20);
        mav_array_memcpy(packet1.serial_number, packet_in.serial_number, sizeof(char)*16);
        mav_array_memcpy(packet1.codec, packet_in.codec, sizeof(char)*6);
        mav_array_memcpy(packet1.encoder_mode, packet_in.encoder_mode, sizeof(char)*6);
        mav_array_memcpy(packet1.rtsp_url_qgc, packet_in.rtsp_url_qgc, sizeof(char)*48);
        mav_array_memcpy(packet1.rtsp_url_controller, packet_in.rtsp_url_controller, sizeof(char)*48);
        mav_array_memcpy(packet1.rtsp_url_laptop, packet_in.rtsp_url_laptop, sizeof(char)*40);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_camera_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_telemetry_camera_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_camera_pack(system_id, component_id, &msg , packet1.video_width , packet1.video_height , packet1.depth_width , packet1.depth_height , packet1.rotation , packet1.bitrate_kbps , packet1.bitrate_max_kbps , packet1.vbv_buffer_kb , packet1.rtsp_port , packet1.video_fps , packet1.depth_fps , packet1.camera_id , packet1.is_default , packet1.profile_mode , packet1.enable_emitter , packet1.camera_status , packet1.error_code , packet1.usb_speed_mode , packet1.camera_name , packet1.camera_type , packet1.connection_port , packet1.serial_number , packet1.codec , packet1.encoder_mode , packet1.rtsp_url_qgc , packet1.rtsp_url_controller , packet1.rtsp_url_laptop );
    mavlink_msg_cc_telemetry_camera_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_camera_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.video_width , packet1.video_height , packet1.depth_width , packet1.depth_height , packet1.rotation , packet1.bitrate_kbps , packet1.bitrate_max_kbps , packet1.vbv_buffer_kb , packet1.rtsp_port , packet1.video_fps , packet1.depth_fps , packet1.camera_id , packet1.is_default , packet1.profile_mode , packet1.enable_emitter , packet1.camera_status , packet1.error_code , packet1.usb_speed_mode , packet1.camera_name , packet1.camera_type , packet1.connection_port , packet1.serial_number , packet1.codec , packet1.encoder_mode , packet1.rtsp_url_qgc , packet1.rtsp_url_controller , packet1.rtsp_url_laptop );
    mavlink_msg_cc_telemetry_camera_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_telemetry_camera_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_camera_send(MAVLINK_COMM_1 , packet1.video_width , packet1.video_height , packet1.depth_width , packet1.depth_height , packet1.rotation , packet1.bitrate_kbps , packet1.bitrate_max_kbps , packet1.vbv_buffer_kb , packet1.rtsp_port , packet1.video_fps , packet1.depth_fps , packet1.camera_id , packet1.is_default , packet1.profile_mode , packet1.enable_emitter , packet1.camera_status , packet1.error_code , packet1.usb_speed_mode , packet1.camera_name , packet1.camera_type , packet1.connection_port , packet1.serial_number , packet1.codec , packet1.encoder_mode , packet1.rtsp_url_qgc , packet1.rtsp_url_controller , packet1.rtsp_url_laptop );
    mavlink_msg_cc_telemetry_camera_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_TELEMETRY_CAMERA") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_TELEMETRY_CAMERA) != NULL);
#endif
}

static void mavlink_test_cc_telemetry_network(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_telemetry_network_t packet_in = {
        963497464,963497672,963497880,963498088,963498296,963498504,77,144,211,22,89,156,223,34,101,168,235,46,"KLMNOPQRSTUVWXY","ABCDEFGHIJKLMNO","QRSTUVWXYZABCDE","GHIJKLMNOPQRSTU","WXYZABCDEFGHIJKLMNOPQRSTUVWXYZA","CDEFGHIJKLMNOPQ","STUVWXYZABCDEFG","IJKLMNOPQRSTUVWXYZABCDEFGHIJKLM","OPQRSTUVWXYZABC","EFGHIJKLMNO","QRS"
    };
    mavlink_cc_telemetry_network_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.uap0_rx_kb = packet_in.uap0_rx_kb;
        packet1.uap0_tx_kb = packet_in.uap0_tx_kb;
        packet1.wlan0_rx_kb = packet_in.wlan0_rx_kb;
        packet1.wlan0_tx_kb = packet_in.wlan0_tx_kb;
        packet1.eth0_rx_kb = packet_in.eth0_rx_kb;
        packet1.eth0_tx_kb = packet_in.eth0_tx_kb;
        packet1.ap_channel = packet_in.ap_channel;
        packet1.ap_ieee80211n = packet_in.ap_ieee80211n;
        packet1.ap_wmm_enabled = packet_in.ap_wmm_enabled;
        packet1.ap_wpa = packet_in.ap_wpa;
        packet1.ap_client_count = packet_in.ap_client_count;
        packet1.ap_status = packet_in.ap_status;
        packet1.wlan0_status = packet_in.wlan0_status;
        packet1.eth0_status = packet_in.eth0_status;
        packet1.eth0_is_static = packet_in.eth0_is_static;
        packet1.wlan0_dhcp = packet_in.wlan0_dhcp;
        packet1.dnsmasq_status = packet_in.dnsmasq_status;
        packet1.wlan0_rssi = packet_in.wlan0_rssi;
        
        mav_array_memcpy(packet1.eth0_ip, packet_in.eth0_ip, sizeof(char)*16);
        mav_array_memcpy(packet1.eth0_netmask, packet_in.eth0_netmask, sizeof(char)*16);
        mav_array_memcpy(packet1.wlan0_ip, packet_in.wlan0_ip, sizeof(char)*16);
        mav_array_memcpy(packet1.wlan0_netmask, packet_in.wlan0_netmask, sizeof(char)*16);
        mav_array_memcpy(packet1.wlan0_ssid, packet_in.wlan0_ssid, sizeof(char)*32);
        mav_array_memcpy(packet1.ap_ip, packet_in.ap_ip, sizeof(char)*16);
        mav_array_memcpy(packet1.ap_netmask, packet_in.ap_netmask, sizeof(char)*16);
        mav_array_memcpy(packet1.ap_ssid, packet_in.ap_ssid, sizeof(char)*32);
        mav_array_memcpy(packet1.ap_wpa_passphrase, packet_in.ap_wpa_passphrase, sizeof(char)*16);
        mav_array_memcpy(packet1.ap_key_mgmt, packet_in.ap_key_mgmt, sizeof(char)*12);
        mav_array_memcpy(packet1.ap_hw_mode, packet_in.ap_hw_mode, sizeof(char)*4);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_network_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_telemetry_network_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_network_pack(system_id, component_id, &msg , packet1.uap0_rx_kb , packet1.uap0_tx_kb , packet1.wlan0_rx_kb , packet1.wlan0_tx_kb , packet1.eth0_rx_kb , packet1.eth0_tx_kb , packet1.ap_channel , packet1.ap_ieee80211n , packet1.ap_wmm_enabled , packet1.ap_wpa , packet1.ap_client_count , packet1.ap_status , packet1.wlan0_status , packet1.eth0_status , packet1.eth0_is_static , packet1.wlan0_dhcp , packet1.dnsmasq_status , packet1.wlan0_rssi , packet1.eth0_ip , packet1.eth0_netmask , packet1.wlan0_ip , packet1.wlan0_netmask , packet1.wlan0_ssid , packet1.ap_ip , packet1.ap_netmask , packet1.ap_ssid , packet1.ap_wpa_passphrase , packet1.ap_key_mgmt , packet1.ap_hw_mode );
    mavlink_msg_cc_telemetry_network_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_network_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.uap0_rx_kb , packet1.uap0_tx_kb , packet1.wlan0_rx_kb , packet1.wlan0_tx_kb , packet1.eth0_rx_kb , packet1.eth0_tx_kb , packet1.ap_channel , packet1.ap_ieee80211n , packet1.ap_wmm_enabled , packet1.ap_wpa , packet1.ap_client_count , packet1.ap_status , packet1.wlan0_status , packet1.eth0_status , packet1.eth0_is_static , packet1.wlan0_dhcp , packet1.dnsmasq_status , packet1.wlan0_rssi , packet1.eth0_ip , packet1.eth0_netmask , packet1.wlan0_ip , packet1.wlan0_netmask , packet1.wlan0_ssid , packet1.ap_ip , packet1.ap_netmask , packet1.ap_ssid , packet1.ap_wpa_passphrase , packet1.ap_key_mgmt , packet1.ap_hw_mode );
    mavlink_msg_cc_telemetry_network_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_telemetry_network_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_network_send(MAVLINK_COMM_1 , packet1.uap0_rx_kb , packet1.uap0_tx_kb , packet1.wlan0_rx_kb , packet1.wlan0_tx_kb , packet1.eth0_rx_kb , packet1.eth0_tx_kb , packet1.ap_channel , packet1.ap_ieee80211n , packet1.ap_wmm_enabled , packet1.ap_wpa , packet1.ap_client_count , packet1.ap_status , packet1.wlan0_status , packet1.eth0_status , packet1.eth0_is_static , packet1.wlan0_dhcp , packet1.dnsmasq_status , packet1.wlan0_rssi , packet1.eth0_ip , packet1.eth0_netmask , packet1.wlan0_ip , packet1.wlan0_netmask , packet1.wlan0_ssid , packet1.ap_ip , packet1.ap_netmask , packet1.ap_ssid , packet1.ap_wpa_passphrase , packet1.ap_key_mgmt , packet1.ap_hw_mode );
    mavlink_msg_cc_telemetry_network_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_TELEMETRY_NETWORK") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_TELEMETRY_NETWORK) != NULL);
#endif
}

static void mavlink_test_cc_telemetry_vision(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_TELEMETRY_VISION >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_telemetry_vision_t packet_in = {
        17.0,45.0,17651,17755,41,108,175,"PQRSTUVWXYZABCDEFGHIJKL","NOPQRSTUVWX"
    };
    mavlink_cc_telemetry_vision_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.confidence_thresh = packet_in.confidence_thresh;
        packet1.inference_fps = packet_in.inference_fps;
        packet1.input_width = packet_in.input_width;
        packet1.input_height = packet_in.input_height;
        packet1.video_fps = packet_in.video_fps;
        packet1.detections_count = packet_in.detections_count;
        packet1.status_flags = packet_in.status_flags;
        
        mav_array_memcpy(packet1.model_name, packet_in.model_name, sizeof(char)*24);
        mav_array_memcpy(packet1.input_source, packet_in.input_source, sizeof(char)*12);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_TELEMETRY_VISION_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_vision_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_telemetry_vision_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_vision_pack(system_id, component_id, &msg , packet1.confidence_thresh , packet1.inference_fps , packet1.input_width , packet1.input_height , packet1.video_fps , packet1.detections_count , packet1.status_flags , packet1.model_name , packet1.input_source );
    mavlink_msg_cc_telemetry_vision_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_vision_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.confidence_thresh , packet1.inference_fps , packet1.input_width , packet1.input_height , packet1.video_fps , packet1.detections_count , packet1.status_flags , packet1.model_name , packet1.input_source );
    mavlink_msg_cc_telemetry_vision_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_telemetry_vision_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_vision_send(MAVLINK_COMM_1 , packet1.confidence_thresh , packet1.inference_fps , packet1.input_width , packet1.input_height , packet1.video_fps , packet1.detections_count , packet1.status_flags , packet1.model_name , packet1.input_source );
    mavlink_msg_cc_telemetry_vision_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_TELEMETRY_VISION") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_TELEMETRY_VISION) != NULL);
#endif
}

static void mavlink_test_cc_telemetry_system(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_telemetry_system_t packet_in = {
        963497464,17,84,151,218,29
    };
    mavlink_cc_telemetry_system_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.system_uptime_s = packet_in.system_uptime_s;
        packet1.cpu_usage = packet_in.cpu_usage;
        packet1.ram_usage = packet_in.ram_usage;
        packet1.disk_usage = packet_in.disk_usage;
        packet1.cpu_temp = packet_in.cpu_temp;
        packet1.system_status = packet_in.system_status;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_system_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_telemetry_system_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_system_pack(system_id, component_id, &msg , packet1.system_uptime_s , packet1.cpu_usage , packet1.ram_usage , packet1.disk_usage , packet1.cpu_temp , packet1.system_status );
    mavlink_msg_cc_telemetry_system_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_system_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.system_uptime_s , packet1.cpu_usage , packet1.ram_usage , packet1.disk_usage , packet1.cpu_temp , packet1.system_status );
    mavlink_msg_cc_telemetry_system_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_telemetry_system_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_telemetry_system_send(MAVLINK_COMM_1 , packet1.system_uptime_s , packet1.cpu_usage , packet1.ram_usage , packet1.disk_usage , packet1.cpu_temp , packet1.system_status );
    mavlink_msg_cc_telemetry_system_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_TELEMETRY_SYSTEM") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_TELEMETRY_SYSTEM) != NULL);
#endif
}

static void mavlink_test_cc_config_begin(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_CONFIG_BEGIN >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_config_begin_t packet_in = {
        963497464,963497672
    };
    mavlink_cc_config_begin_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.request_id = packet_in.request_id;
        packet1.timeout_s = packet_in.timeout_s;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_CONFIG_BEGIN_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_begin_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_config_begin_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_begin_pack(system_id, component_id, &msg , packet1.request_id , packet1.timeout_s );
    mavlink_msg_cc_config_begin_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_begin_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.request_id , packet1.timeout_s );
    mavlink_msg_cc_config_begin_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_config_begin_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_begin_send(MAVLINK_COMM_1 , packet1.request_id , packet1.timeout_s );
    mavlink_msg_cc_config_begin_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_CONFIG_BEGIN") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_CONFIG_BEGIN) != NULL);
#endif
}

static void mavlink_test_cc_config_set(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_CONFIG_SET >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_config_set_t packet_in = {
        963497464,963497672,"IJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRS","UVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ"
    };
    mavlink_cc_config_set_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.request_id = packet_in.request_id;
        packet1.transaction_id = packet_in.transaction_id;
        
        mav_array_memcpy(packet1.key, packet_in.key, sizeof(char)*64);
        mav_array_memcpy(packet1.value, packet_in.value, sizeof(char)*128);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_CONFIG_SET_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_set_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_config_set_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_set_pack(system_id, component_id, &msg , packet1.request_id , packet1.transaction_id , packet1.key , packet1.value );
    mavlink_msg_cc_config_set_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_set_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.request_id , packet1.transaction_id , packet1.key , packet1.value );
    mavlink_msg_cc_config_set_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_config_set_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_set_send(MAVLINK_COMM_1 , packet1.request_id , packet1.transaction_id , packet1.key , packet1.value );
    mavlink_msg_cc_config_set_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_CONFIG_SET") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_CONFIG_SET) != NULL);
#endif
}

static void mavlink_test_cc_config_get(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_CONFIG_GET >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_config_get_t packet_in = {
        963497464,"EFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNO"
    };
    mavlink_cc_config_get_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.request_id = packet_in.request_id;
        
        mav_array_memcpy(packet1.key, packet_in.key, sizeof(char)*64);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_CONFIG_GET_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_get_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_config_get_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_get_pack(system_id, component_id, &msg , packet1.request_id , packet1.key );
    mavlink_msg_cc_config_get_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_get_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.request_id , packet1.key );
    mavlink_msg_cc_config_get_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_config_get_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_get_send(MAVLINK_COMM_1 , packet1.request_id , packet1.key );
    mavlink_msg_cc_config_get_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_CONFIG_GET") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_CONFIG_GET) != NULL);
#endif
}

static void mavlink_test_cc_config_apply(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_CONFIG_APPLY >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_config_apply_t packet_in = {
        963497464,963497672
    };
    mavlink_cc_config_apply_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.request_id = packet_in.request_id;
        packet1.transaction_id = packet_in.transaction_id;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_CONFIG_APPLY_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_CONFIG_APPLY_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_apply_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_config_apply_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_apply_pack(system_id, component_id, &msg , packet1.request_id , packet1.transaction_id );
    mavlink_msg_cc_config_apply_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_apply_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.request_id , packet1.transaction_id );
    mavlink_msg_cc_config_apply_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_config_apply_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_apply_send(MAVLINK_COMM_1 , packet1.request_id , packet1.transaction_id );
    mavlink_msg_cc_config_apply_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_CONFIG_APPLY") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_CONFIG_APPLY) != NULL);
#endif
}

static void mavlink_test_cc_config_confirm(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_CONFIG_CONFIRM >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_config_confirm_t packet_in = {
        963497464,963497672
    };
    mavlink_cc_config_confirm_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.request_id = packet_in.request_id;
        packet1.transaction_id = packet_in.transaction_id;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_CONFIG_CONFIRM_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_confirm_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_config_confirm_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_confirm_pack(system_id, component_id, &msg , packet1.request_id , packet1.transaction_id );
    mavlink_msg_cc_config_confirm_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_confirm_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.request_id , packet1.transaction_id );
    mavlink_msg_cc_config_confirm_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_config_confirm_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_confirm_send(MAVLINK_COMM_1 , packet1.request_id , packet1.transaction_id );
    mavlink_msg_cc_config_confirm_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_CONFIG_CONFIRM") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_CONFIG_CONFIRM) != NULL);
#endif
}

static void mavlink_test_cc_config_rollback(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_CONFIG_ROLLBACK >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_config_rollback_t packet_in = {
        963497464,963497672
    };
    mavlink_cc_config_rollback_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.request_id = packet_in.request_id;
        packet1.transaction_id = packet_in.transaction_id;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_CONFIG_ROLLBACK_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_CONFIG_ROLLBACK_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_rollback_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_config_rollback_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_rollback_pack(system_id, component_id, &msg , packet1.request_id , packet1.transaction_id );
    mavlink_msg_cc_config_rollback_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_rollback_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.request_id , packet1.transaction_id );
    mavlink_msg_cc_config_rollback_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_config_rollback_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_rollback_send(MAVLINK_COMM_1 , packet1.request_id , packet1.transaction_id );
    mavlink_msg_cc_config_rollback_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_CONFIG_ROLLBACK") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_CONFIG_ROLLBACK) != NULL);
#endif
}

static void mavlink_test_cc_config_ack(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_CONFIG_ACK >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_config_ack_t packet_in = {
        963497464,963497672,17651,163,230,"MNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI"
    };
    mavlink_cc_config_ack_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.request_id = packet_in.request_id;
        packet1.transaction_id = packet_in.transaction_id;
        packet1.error_code = packet_in.error_code;
        packet1.result = packet_in.result;
        packet1.apply_type = packet_in.apply_type;
        
        mav_array_memcpy(packet1.message, packet_in.message, sizeof(char)*50);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_CONFIG_ACK_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_ack_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_config_ack_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_ack_pack(system_id, component_id, &msg , packet1.request_id , packet1.transaction_id , packet1.result , packet1.apply_type , packet1.error_code , packet1.message );
    mavlink_msg_cc_config_ack_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_ack_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.request_id , packet1.transaction_id , packet1.result , packet1.apply_type , packet1.error_code , packet1.message );
    mavlink_msg_cc_config_ack_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_config_ack_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_ack_send(MAVLINK_COMM_1 , packet1.request_id , packet1.transaction_id , packet1.result , packet1.apply_type , packet1.error_code , packet1.message );
    mavlink_msg_cc_config_ack_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_CONFIG_ACK") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_CONFIG_ACK) != NULL);
#endif
}

static void mavlink_test_cc_config_value(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_CONFIG_VALUE >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_config_value_t packet_in = {
        963497464,"EFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNO","QRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM"
    };
    mavlink_cc_config_value_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.request_id = packet_in.request_id;
        
        mav_array_memcpy(packet1.key, packet_in.key, sizeof(char)*64);
        mav_array_memcpy(packet1.value, packet_in.value, sizeof(char)*128);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_CONFIG_VALUE_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_value_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_config_value_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_value_pack(system_id, component_id, &msg , packet1.request_id , packet1.key , packet1.value );
    mavlink_msg_cc_config_value_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_value_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.request_id , packet1.key , packet1.value );
    mavlink_msg_cc_config_value_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_config_value_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_value_send(MAVLINK_COMM_1 , packet1.request_id , packet1.key , packet1.value );
    mavlink_msg_cc_config_value_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_CONFIG_VALUE") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_CONFIG_VALUE) != NULL);
#endif
}

static void mavlink_test_cc_config_status(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_CC_CONFIG_STATUS >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_cc_config_status_t packet_in = {
        963497464,17443,151,218,"IJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDE"
    };
    mavlink_cc_config_status_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.transaction_id = packet_in.transaction_id;
        packet1.error_code = packet_in.error_code;
        packet1.state = packet_in.state;
        packet1.progress = packet_in.progress;
        
        mav_array_memcpy(packet1.message, packet_in.message, sizeof(char)*50);
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_CC_CONFIG_STATUS_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_status_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_cc_config_status_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_status_pack(system_id, component_id, &msg , packet1.transaction_id , packet1.state , packet1.progress , packet1.error_code , packet1.message );
    mavlink_msg_cc_config_status_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_status_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.transaction_id , packet1.state , packet1.progress , packet1.error_code , packet1.message );
    mavlink_msg_cc_config_status_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_cc_config_status_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_cc_config_status_send(MAVLINK_COMM_1 , packet1.transaction_id , packet1.state , packet1.progress , packet1.error_code , packet1.message );
    mavlink_msg_cc_config_status_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("CC_CONFIG_STATUS") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_CC_CONFIG_STATUS) != NULL);
#endif
}

static void mavlink_test_thaco_common(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_thaco_external_xyz_trigger(system_id, component_id, last_msg);
    mavlink_test_cc_telemetry_links(system_id, component_id, last_msg);
    mavlink_test_cc_telemetry_camera(system_id, component_id, last_msg);
    mavlink_test_cc_telemetry_network(system_id, component_id, last_msg);
    mavlink_test_cc_telemetry_vision(system_id, component_id, last_msg);
    mavlink_test_cc_telemetry_system(system_id, component_id, last_msg);
    mavlink_test_cc_config_begin(system_id, component_id, last_msg);
    mavlink_test_cc_config_set(system_id, component_id, last_msg);
    mavlink_test_cc_config_get(system_id, component_id, last_msg);
    mavlink_test_cc_config_apply(system_id, component_id, last_msg);
    mavlink_test_cc_config_confirm(system_id, component_id, last_msg);
    mavlink_test_cc_config_rollback(system_id, component_id, last_msg);
    mavlink_test_cc_config_ack(system_id, component_id, last_msg);
    mavlink_test_cc_config_value(system_id, component_id, last_msg);
    mavlink_test_cc_config_status(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // THACO_COMMON_TESTSUITE_H
