/** @file
 *	@brief MAVLink comm testsuite protocol generated from thaco.xml
 *	@see http://mavlink.org
 */

#pragma once

#include <gtest/gtest.h>
#include "thaco.hpp"

#ifdef TEST_INTEROP
using namespace mavlink;
#undef MAVLINK_HELPER
#include "mavlink.h"
#endif


TEST(thaco, THACO_EXTERNAL_XYZ_TRIGGER)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::THACO_EXTERNAL_XYZ_TRIGGER packet_in{};
    packet_in.trigger_id = 963497464;
    packet_in.time_boot_ms = 963497672;

    mavlink::thaco::msg::THACO_EXTERNAL_XYZ_TRIGGER packet1{};
    mavlink::thaco::msg::THACO_EXTERNAL_XYZ_TRIGGER packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.trigger_id, packet2.trigger_id);
    EXPECT_EQ(packet1.time_boot_ms, packet2.time_boot_ms);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, THACO_EXTERNAL_XYZ_TRIGGER)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_thaco_external_xyz_trigger_t packet_c {
         963497464, 963497672
    };

    mavlink::thaco::msg::THACO_EXTERNAL_XYZ_TRIGGER packet_in{};
    packet_in.trigger_id = 963497464;
    packet_in.time_boot_ms = 963497672;

    mavlink::thaco::msg::THACO_EXTERNAL_XYZ_TRIGGER packet2{};

    mavlink_msg_thaco_external_xyz_trigger_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.trigger_id, packet2.trigger_id);
    EXPECT_EQ(packet_in.time_boot_ms, packet2.time_boot_ms);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_SERIAL_LINK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_SERIAL_LINK packet_in{};
    packet_in.rx_rate = 17.0;
    packet_in.tx_rate = 45.0;
    packet_in.rx_loss = 73.0;
    packet_in.rx_bytes = 963498088;
    packet_in.tx_bytes = 963498296;
    packet_in.rx_errors = 963498504;
    packet_in.baudrate = 963498712;
    packet_in.link_index = 89;
    packet_in.link_count = 156;
    packet_in.status = 223;
    packet_in.name = to_char_array("FGHIJKLMNOPQRST");
    packet_in.port = to_char_array("VWXYZABCDEFGHIJ");

    mavlink::thaco::msg::CC_SERIAL_LINK packet1{};
    mavlink::thaco::msg::CC_SERIAL_LINK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.rx_rate, packet2.rx_rate);
    EXPECT_EQ(packet1.tx_rate, packet2.tx_rate);
    EXPECT_EQ(packet1.rx_loss, packet2.rx_loss);
    EXPECT_EQ(packet1.rx_bytes, packet2.rx_bytes);
    EXPECT_EQ(packet1.tx_bytes, packet2.tx_bytes);
    EXPECT_EQ(packet1.rx_errors, packet2.rx_errors);
    EXPECT_EQ(packet1.baudrate, packet2.baudrate);
    EXPECT_EQ(packet1.link_index, packet2.link_index);
    EXPECT_EQ(packet1.link_count, packet2.link_count);
    EXPECT_EQ(packet1.status, packet2.status);
    EXPECT_EQ(packet1.name, packet2.name);
    EXPECT_EQ(packet1.port, packet2.port);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_SERIAL_LINK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_serial_link_t packet_c {
         17.0, 45.0, 73.0, 963498088, 963498296, 963498504, 963498712, 89, 156, 223, "FGHIJKLMNOPQRST", "VWXYZABCDEFGHIJ"
    };

    mavlink::thaco::msg::CC_SERIAL_LINK packet_in{};
    packet_in.rx_rate = 17.0;
    packet_in.tx_rate = 45.0;
    packet_in.rx_loss = 73.0;
    packet_in.rx_bytes = 963498088;
    packet_in.tx_bytes = 963498296;
    packet_in.rx_errors = 963498504;
    packet_in.baudrate = 963498712;
    packet_in.link_index = 89;
    packet_in.link_count = 156;
    packet_in.status = 223;
    packet_in.name = to_char_array("FGHIJKLMNOPQRST");
    packet_in.port = to_char_array("VWXYZABCDEFGHIJ");

    mavlink::thaco::msg::CC_SERIAL_LINK packet2{};

    mavlink_msg_cc_serial_link_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.rx_rate, packet2.rx_rate);
    EXPECT_EQ(packet_in.tx_rate, packet2.tx_rate);
    EXPECT_EQ(packet_in.rx_loss, packet2.rx_loss);
    EXPECT_EQ(packet_in.rx_bytes, packet2.rx_bytes);
    EXPECT_EQ(packet_in.tx_bytes, packet2.tx_bytes);
    EXPECT_EQ(packet_in.rx_errors, packet2.rx_errors);
    EXPECT_EQ(packet_in.baudrate, packet2.baudrate);
    EXPECT_EQ(packet_in.link_index, packet2.link_index);
    EXPECT_EQ(packet_in.link_count, packet2.link_count);
    EXPECT_EQ(packet_in.status, packet2.status);
    EXPECT_EQ(packet_in.name, packet2.name);
    EXPECT_EQ(packet_in.port, packet2.port);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_TELEMETRY_CAMERA)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_TELEMETRY_CAMERA packet_in{};
    packet_in.video_width = 17235;
    packet_in.video_height = 17339;
    packet_in.depth_width = 17443;
    packet_in.depth_height = 17547;
    packet_in.rotation = 17651;
    packet_in.bitrate_kbps = 17755;
    packet_in.bitrate_max_kbps = 17859;
    packet_in.vbv_buffer_kb = 17963;
    packet_in.rtsp_port = 18067;
    packet_in.video_fps = 187;
    packet_in.depth_fps = 254;
    packet_in.camera_id = 65;
    packet_in.is_default = 132;
    packet_in.profile_mode = 199;
    packet_in.enable_emitter = 10;
    packet_in.camera_status = 77;
    packet_in.error_code = 144;
    packet_in.usb_speed_mode = 211;
    packet_in.camera_name = to_char_array("BCDEFGHIJKLMNOPQRSTUVWX");
    packet_in.camera_type = to_char_array("ZABCDEFGHIJ");
    packet_in.connection_port = to_char_array("LMNOPQRSTUVWXYZABCD");
    packet_in.serial_number = to_char_array("FGHIJKLMNOPQRST");
    packet_in.codec = to_char_array("VWXYZ");
    packet_in.encoder_mode = to_char_array("BCDEF");
    packet_in.rtsp_url_qgc = to_char_array("HIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZAB");
    packet_in.rtsp_url_controller = to_char_array("DEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX");
    packet_in.rtsp_url_laptop = to_char_array("ZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL");

    mavlink::thaco::msg::CC_TELEMETRY_CAMERA packet1{};
    mavlink::thaco::msg::CC_TELEMETRY_CAMERA packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.video_width, packet2.video_width);
    EXPECT_EQ(packet1.video_height, packet2.video_height);
    EXPECT_EQ(packet1.depth_width, packet2.depth_width);
    EXPECT_EQ(packet1.depth_height, packet2.depth_height);
    EXPECT_EQ(packet1.rotation, packet2.rotation);
    EXPECT_EQ(packet1.bitrate_kbps, packet2.bitrate_kbps);
    EXPECT_EQ(packet1.bitrate_max_kbps, packet2.bitrate_max_kbps);
    EXPECT_EQ(packet1.vbv_buffer_kb, packet2.vbv_buffer_kb);
    EXPECT_EQ(packet1.rtsp_port, packet2.rtsp_port);
    EXPECT_EQ(packet1.video_fps, packet2.video_fps);
    EXPECT_EQ(packet1.depth_fps, packet2.depth_fps);
    EXPECT_EQ(packet1.camera_id, packet2.camera_id);
    EXPECT_EQ(packet1.is_default, packet2.is_default);
    EXPECT_EQ(packet1.profile_mode, packet2.profile_mode);
    EXPECT_EQ(packet1.enable_emitter, packet2.enable_emitter);
    EXPECT_EQ(packet1.camera_status, packet2.camera_status);
    EXPECT_EQ(packet1.error_code, packet2.error_code);
    EXPECT_EQ(packet1.usb_speed_mode, packet2.usb_speed_mode);
    EXPECT_EQ(packet1.camera_name, packet2.camera_name);
    EXPECT_EQ(packet1.camera_type, packet2.camera_type);
    EXPECT_EQ(packet1.connection_port, packet2.connection_port);
    EXPECT_EQ(packet1.serial_number, packet2.serial_number);
    EXPECT_EQ(packet1.codec, packet2.codec);
    EXPECT_EQ(packet1.encoder_mode, packet2.encoder_mode);
    EXPECT_EQ(packet1.rtsp_url_qgc, packet2.rtsp_url_qgc);
    EXPECT_EQ(packet1.rtsp_url_controller, packet2.rtsp_url_controller);
    EXPECT_EQ(packet1.rtsp_url_laptop, packet2.rtsp_url_laptop);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_TELEMETRY_CAMERA)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_telemetry_camera_t packet_c {
         17235, 17339, 17443, 17547, 17651, 17755, 17859, 17963, 18067, 187, 254, 65, 132, 199, 10, 77, 144, 211, "BCDEFGHIJKLMNOPQRSTUVWX", "ZABCDEFGHIJ", "LMNOPQRSTUVWXYZABCD", "FGHIJKLMNOPQRST", "VWXYZ", "BCDEF", "HIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZAB", "DEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX", "ZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL"
    };

    mavlink::thaco::msg::CC_TELEMETRY_CAMERA packet_in{};
    packet_in.video_width = 17235;
    packet_in.video_height = 17339;
    packet_in.depth_width = 17443;
    packet_in.depth_height = 17547;
    packet_in.rotation = 17651;
    packet_in.bitrate_kbps = 17755;
    packet_in.bitrate_max_kbps = 17859;
    packet_in.vbv_buffer_kb = 17963;
    packet_in.rtsp_port = 18067;
    packet_in.video_fps = 187;
    packet_in.depth_fps = 254;
    packet_in.camera_id = 65;
    packet_in.is_default = 132;
    packet_in.profile_mode = 199;
    packet_in.enable_emitter = 10;
    packet_in.camera_status = 77;
    packet_in.error_code = 144;
    packet_in.usb_speed_mode = 211;
    packet_in.camera_name = to_char_array("BCDEFGHIJKLMNOPQRSTUVWX");
    packet_in.camera_type = to_char_array("ZABCDEFGHIJ");
    packet_in.connection_port = to_char_array("LMNOPQRSTUVWXYZABCD");
    packet_in.serial_number = to_char_array("FGHIJKLMNOPQRST");
    packet_in.codec = to_char_array("VWXYZ");
    packet_in.encoder_mode = to_char_array("BCDEF");
    packet_in.rtsp_url_qgc = to_char_array("HIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZAB");
    packet_in.rtsp_url_controller = to_char_array("DEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX");
    packet_in.rtsp_url_laptop = to_char_array("ZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL");

    mavlink::thaco::msg::CC_TELEMETRY_CAMERA packet2{};

    mavlink_msg_cc_telemetry_camera_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.video_width, packet2.video_width);
    EXPECT_EQ(packet_in.video_height, packet2.video_height);
    EXPECT_EQ(packet_in.depth_width, packet2.depth_width);
    EXPECT_EQ(packet_in.depth_height, packet2.depth_height);
    EXPECT_EQ(packet_in.rotation, packet2.rotation);
    EXPECT_EQ(packet_in.bitrate_kbps, packet2.bitrate_kbps);
    EXPECT_EQ(packet_in.bitrate_max_kbps, packet2.bitrate_max_kbps);
    EXPECT_EQ(packet_in.vbv_buffer_kb, packet2.vbv_buffer_kb);
    EXPECT_EQ(packet_in.rtsp_port, packet2.rtsp_port);
    EXPECT_EQ(packet_in.video_fps, packet2.video_fps);
    EXPECT_EQ(packet_in.depth_fps, packet2.depth_fps);
    EXPECT_EQ(packet_in.camera_id, packet2.camera_id);
    EXPECT_EQ(packet_in.is_default, packet2.is_default);
    EXPECT_EQ(packet_in.profile_mode, packet2.profile_mode);
    EXPECT_EQ(packet_in.enable_emitter, packet2.enable_emitter);
    EXPECT_EQ(packet_in.camera_status, packet2.camera_status);
    EXPECT_EQ(packet_in.error_code, packet2.error_code);
    EXPECT_EQ(packet_in.usb_speed_mode, packet2.usb_speed_mode);
    EXPECT_EQ(packet_in.camera_name, packet2.camera_name);
    EXPECT_EQ(packet_in.camera_type, packet2.camera_type);
    EXPECT_EQ(packet_in.connection_port, packet2.connection_port);
    EXPECT_EQ(packet_in.serial_number, packet2.serial_number);
    EXPECT_EQ(packet_in.codec, packet2.codec);
    EXPECT_EQ(packet_in.encoder_mode, packet2.encoder_mode);
    EXPECT_EQ(packet_in.rtsp_url_qgc, packet2.rtsp_url_qgc);
    EXPECT_EQ(packet_in.rtsp_url_controller, packet2.rtsp_url_controller);
    EXPECT_EQ(packet_in.rtsp_url_laptop, packet2.rtsp_url_laptop);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_TELEMETRY_NETWORK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_TELEMETRY_NETWORK packet_in{};
    packet_in.uap0_rx_kb = 963497464;
    packet_in.uap0_tx_kb = 963497672;
    packet_in.wlan0_rx_kb = 963497880;
    packet_in.wlan0_tx_kb = 963498088;
    packet_in.eth0_rx_kb = 963498296;
    packet_in.eth0_tx_kb = 963498504;
    packet_in.ap_channel = 77;
    packet_in.ap_ieee80211n = 144;
    packet_in.ap_wmm_enabled = 211;
    packet_in.ap_wpa = 22;
    packet_in.ap_client_count = 89;
    packet_in.ap_status = 156;
    packet_in.wlan0_status = 223;
    packet_in.eth0_status = 34;
    packet_in.eth0_is_static = 101;
    packet_in.wlan0_dhcp = 168;
    packet_in.dnsmasq_status = 235;
    packet_in.wlan0_rssi = 46;
    packet_in.eth0_ip = to_char_array("KLMNOPQRSTUVWXY");
    packet_in.eth0_netmask = to_char_array("ABCDEFGHIJKLMNO");
    packet_in.wlan0_ip = to_char_array("QRSTUVWXYZABCDE");
    packet_in.wlan0_netmask = to_char_array("GHIJKLMNOPQRSTU");
    packet_in.wlan0_ssid = to_char_array("WXYZABCDEFGHIJKLMNOPQRSTUVWXYZA");
    packet_in.ap_ip = to_char_array("CDEFGHIJKLMNOPQ");
    packet_in.ap_netmask = to_char_array("STUVWXYZABCDEFG");
    packet_in.ap_ssid = to_char_array("IJKLMNOPQRSTUVWXYZABCDEFGHIJKLM");
    packet_in.ap_wpa_passphrase = to_char_array("OPQRSTUVWXYZABC");
    packet_in.ap_key_mgmt = to_char_array("EFGHIJKLMNO");
    packet_in.ap_hw_mode = to_char_array("QRS");

    mavlink::thaco::msg::CC_TELEMETRY_NETWORK packet1{};
    mavlink::thaco::msg::CC_TELEMETRY_NETWORK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.uap0_rx_kb, packet2.uap0_rx_kb);
    EXPECT_EQ(packet1.uap0_tx_kb, packet2.uap0_tx_kb);
    EXPECT_EQ(packet1.wlan0_rx_kb, packet2.wlan0_rx_kb);
    EXPECT_EQ(packet1.wlan0_tx_kb, packet2.wlan0_tx_kb);
    EXPECT_EQ(packet1.eth0_rx_kb, packet2.eth0_rx_kb);
    EXPECT_EQ(packet1.eth0_tx_kb, packet2.eth0_tx_kb);
    EXPECT_EQ(packet1.ap_channel, packet2.ap_channel);
    EXPECT_EQ(packet1.ap_ieee80211n, packet2.ap_ieee80211n);
    EXPECT_EQ(packet1.ap_wmm_enabled, packet2.ap_wmm_enabled);
    EXPECT_EQ(packet1.ap_wpa, packet2.ap_wpa);
    EXPECT_EQ(packet1.ap_client_count, packet2.ap_client_count);
    EXPECT_EQ(packet1.ap_status, packet2.ap_status);
    EXPECT_EQ(packet1.wlan0_status, packet2.wlan0_status);
    EXPECT_EQ(packet1.eth0_status, packet2.eth0_status);
    EXPECT_EQ(packet1.eth0_is_static, packet2.eth0_is_static);
    EXPECT_EQ(packet1.wlan0_dhcp, packet2.wlan0_dhcp);
    EXPECT_EQ(packet1.dnsmasq_status, packet2.dnsmasq_status);
    EXPECT_EQ(packet1.wlan0_rssi, packet2.wlan0_rssi);
    EXPECT_EQ(packet1.eth0_ip, packet2.eth0_ip);
    EXPECT_EQ(packet1.eth0_netmask, packet2.eth0_netmask);
    EXPECT_EQ(packet1.wlan0_ip, packet2.wlan0_ip);
    EXPECT_EQ(packet1.wlan0_netmask, packet2.wlan0_netmask);
    EXPECT_EQ(packet1.wlan0_ssid, packet2.wlan0_ssid);
    EXPECT_EQ(packet1.ap_ip, packet2.ap_ip);
    EXPECT_EQ(packet1.ap_netmask, packet2.ap_netmask);
    EXPECT_EQ(packet1.ap_ssid, packet2.ap_ssid);
    EXPECT_EQ(packet1.ap_wpa_passphrase, packet2.ap_wpa_passphrase);
    EXPECT_EQ(packet1.ap_key_mgmt, packet2.ap_key_mgmt);
    EXPECT_EQ(packet1.ap_hw_mode, packet2.ap_hw_mode);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_TELEMETRY_NETWORK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_telemetry_network_t packet_c {
         963497464, 963497672, 963497880, 963498088, 963498296, 963498504, 77, 144, 211, 22, 89, 156, 223, 34, 101, 168, 235, 46, "KLMNOPQRSTUVWXY", "ABCDEFGHIJKLMNO", "QRSTUVWXYZABCDE", "GHIJKLMNOPQRSTU", "WXYZABCDEFGHIJKLMNOPQRSTUVWXYZA", "CDEFGHIJKLMNOPQ", "STUVWXYZABCDEFG", "IJKLMNOPQRSTUVWXYZABCDEFGHIJKLM", "OPQRSTUVWXYZABC", "EFGHIJKLMNO", "QRS"
    };

    mavlink::thaco::msg::CC_TELEMETRY_NETWORK packet_in{};
    packet_in.uap0_rx_kb = 963497464;
    packet_in.uap0_tx_kb = 963497672;
    packet_in.wlan0_rx_kb = 963497880;
    packet_in.wlan0_tx_kb = 963498088;
    packet_in.eth0_rx_kb = 963498296;
    packet_in.eth0_tx_kb = 963498504;
    packet_in.ap_channel = 77;
    packet_in.ap_ieee80211n = 144;
    packet_in.ap_wmm_enabled = 211;
    packet_in.ap_wpa = 22;
    packet_in.ap_client_count = 89;
    packet_in.ap_status = 156;
    packet_in.wlan0_status = 223;
    packet_in.eth0_status = 34;
    packet_in.eth0_is_static = 101;
    packet_in.wlan0_dhcp = 168;
    packet_in.dnsmasq_status = 235;
    packet_in.wlan0_rssi = 46;
    packet_in.eth0_ip = to_char_array("KLMNOPQRSTUVWXY");
    packet_in.eth0_netmask = to_char_array("ABCDEFGHIJKLMNO");
    packet_in.wlan0_ip = to_char_array("QRSTUVWXYZABCDE");
    packet_in.wlan0_netmask = to_char_array("GHIJKLMNOPQRSTU");
    packet_in.wlan0_ssid = to_char_array("WXYZABCDEFGHIJKLMNOPQRSTUVWXYZA");
    packet_in.ap_ip = to_char_array("CDEFGHIJKLMNOPQ");
    packet_in.ap_netmask = to_char_array("STUVWXYZABCDEFG");
    packet_in.ap_ssid = to_char_array("IJKLMNOPQRSTUVWXYZABCDEFGHIJKLM");
    packet_in.ap_wpa_passphrase = to_char_array("OPQRSTUVWXYZABC");
    packet_in.ap_key_mgmt = to_char_array("EFGHIJKLMNO");
    packet_in.ap_hw_mode = to_char_array("QRS");

    mavlink::thaco::msg::CC_TELEMETRY_NETWORK packet2{};

    mavlink_msg_cc_telemetry_network_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.uap0_rx_kb, packet2.uap0_rx_kb);
    EXPECT_EQ(packet_in.uap0_tx_kb, packet2.uap0_tx_kb);
    EXPECT_EQ(packet_in.wlan0_rx_kb, packet2.wlan0_rx_kb);
    EXPECT_EQ(packet_in.wlan0_tx_kb, packet2.wlan0_tx_kb);
    EXPECT_EQ(packet_in.eth0_rx_kb, packet2.eth0_rx_kb);
    EXPECT_EQ(packet_in.eth0_tx_kb, packet2.eth0_tx_kb);
    EXPECT_EQ(packet_in.ap_channel, packet2.ap_channel);
    EXPECT_EQ(packet_in.ap_ieee80211n, packet2.ap_ieee80211n);
    EXPECT_EQ(packet_in.ap_wmm_enabled, packet2.ap_wmm_enabled);
    EXPECT_EQ(packet_in.ap_wpa, packet2.ap_wpa);
    EXPECT_EQ(packet_in.ap_client_count, packet2.ap_client_count);
    EXPECT_EQ(packet_in.ap_status, packet2.ap_status);
    EXPECT_EQ(packet_in.wlan0_status, packet2.wlan0_status);
    EXPECT_EQ(packet_in.eth0_status, packet2.eth0_status);
    EXPECT_EQ(packet_in.eth0_is_static, packet2.eth0_is_static);
    EXPECT_EQ(packet_in.wlan0_dhcp, packet2.wlan0_dhcp);
    EXPECT_EQ(packet_in.dnsmasq_status, packet2.dnsmasq_status);
    EXPECT_EQ(packet_in.wlan0_rssi, packet2.wlan0_rssi);
    EXPECT_EQ(packet_in.eth0_ip, packet2.eth0_ip);
    EXPECT_EQ(packet_in.eth0_netmask, packet2.eth0_netmask);
    EXPECT_EQ(packet_in.wlan0_ip, packet2.wlan0_ip);
    EXPECT_EQ(packet_in.wlan0_netmask, packet2.wlan0_netmask);
    EXPECT_EQ(packet_in.wlan0_ssid, packet2.wlan0_ssid);
    EXPECT_EQ(packet_in.ap_ip, packet2.ap_ip);
    EXPECT_EQ(packet_in.ap_netmask, packet2.ap_netmask);
    EXPECT_EQ(packet_in.ap_ssid, packet2.ap_ssid);
    EXPECT_EQ(packet_in.ap_wpa_passphrase, packet2.ap_wpa_passphrase);
    EXPECT_EQ(packet_in.ap_key_mgmt, packet2.ap_key_mgmt);
    EXPECT_EQ(packet_in.ap_hw_mode, packet2.ap_hw_mode);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_TELEMETRY_VISION)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_TELEMETRY_VISION packet_in{};
    packet_in.confidence_thresh = 17.0;
    packet_in.inference_fps = 45.0;
    packet_in.input_width = 17651;
    packet_in.input_height = 17755;
    packet_in.video_fps = 41;
    packet_in.detections_count = 108;
    packet_in.status_flags = 175;
    packet_in.model_name = to_char_array("PQRSTUVWXYZABCDEFGHIJKL");
    packet_in.input_source = to_char_array("NOPQRSTUVWX");

    mavlink::thaco::msg::CC_TELEMETRY_VISION packet1{};
    mavlink::thaco::msg::CC_TELEMETRY_VISION packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.confidence_thresh, packet2.confidence_thresh);
    EXPECT_EQ(packet1.inference_fps, packet2.inference_fps);
    EXPECT_EQ(packet1.input_width, packet2.input_width);
    EXPECT_EQ(packet1.input_height, packet2.input_height);
    EXPECT_EQ(packet1.video_fps, packet2.video_fps);
    EXPECT_EQ(packet1.detections_count, packet2.detections_count);
    EXPECT_EQ(packet1.status_flags, packet2.status_flags);
    EXPECT_EQ(packet1.model_name, packet2.model_name);
    EXPECT_EQ(packet1.input_source, packet2.input_source);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_TELEMETRY_VISION)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_telemetry_vision_t packet_c {
         17.0, 45.0, 17651, 17755, 41, 108, 175, "PQRSTUVWXYZABCDEFGHIJKL", "NOPQRSTUVWX"
    };

    mavlink::thaco::msg::CC_TELEMETRY_VISION packet_in{};
    packet_in.confidence_thresh = 17.0;
    packet_in.inference_fps = 45.0;
    packet_in.input_width = 17651;
    packet_in.input_height = 17755;
    packet_in.video_fps = 41;
    packet_in.detections_count = 108;
    packet_in.status_flags = 175;
    packet_in.model_name = to_char_array("PQRSTUVWXYZABCDEFGHIJKL");
    packet_in.input_source = to_char_array("NOPQRSTUVWX");

    mavlink::thaco::msg::CC_TELEMETRY_VISION packet2{};

    mavlink_msg_cc_telemetry_vision_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.confidence_thresh, packet2.confidence_thresh);
    EXPECT_EQ(packet_in.inference_fps, packet2.inference_fps);
    EXPECT_EQ(packet_in.input_width, packet2.input_width);
    EXPECT_EQ(packet_in.input_height, packet2.input_height);
    EXPECT_EQ(packet_in.video_fps, packet2.video_fps);
    EXPECT_EQ(packet_in.detections_count, packet2.detections_count);
    EXPECT_EQ(packet_in.status_flags, packet2.status_flags);
    EXPECT_EQ(packet_in.model_name, packet2.model_name);
    EXPECT_EQ(packet_in.input_source, packet2.input_source);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_TELEMETRY_SYSTEM)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_TELEMETRY_SYSTEM packet_in{};
    packet_in.system_uptime_s = 963497464;
    packet_in.cpu_usage = 17;
    packet_in.ram_usage = 84;
    packet_in.disk_usage = 151;
    packet_in.cpu_temp = -38;
    packet_in.system_status = 29;

    mavlink::thaco::msg::CC_TELEMETRY_SYSTEM packet1{};
    mavlink::thaco::msg::CC_TELEMETRY_SYSTEM packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.system_uptime_s, packet2.system_uptime_s);
    EXPECT_EQ(packet1.cpu_usage, packet2.cpu_usage);
    EXPECT_EQ(packet1.ram_usage, packet2.ram_usage);
    EXPECT_EQ(packet1.disk_usage, packet2.disk_usage);
    EXPECT_EQ(packet1.cpu_temp, packet2.cpu_temp);
    EXPECT_EQ(packet1.system_status, packet2.system_status);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_TELEMETRY_SYSTEM)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_telemetry_system_t packet_c {
         963497464, 17, 84, 151, -38, 29
    };

    mavlink::thaco::msg::CC_TELEMETRY_SYSTEM packet_in{};
    packet_in.system_uptime_s = 963497464;
    packet_in.cpu_usage = 17;
    packet_in.ram_usage = 84;
    packet_in.disk_usage = 151;
    packet_in.cpu_temp = -38;
    packet_in.system_status = 29;

    mavlink::thaco::msg::CC_TELEMETRY_SYSTEM packet2{};

    mavlink_msg_cc_telemetry_system_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.system_uptime_s, packet2.system_uptime_s);
    EXPECT_EQ(packet_in.cpu_usage, packet2.cpu_usage);
    EXPECT_EQ(packet_in.ram_usage, packet2.ram_usage);
    EXPECT_EQ(packet_in.disk_usage, packet2.disk_usage);
    EXPECT_EQ(packet_in.cpu_temp, packet2.cpu_temp);
    EXPECT_EQ(packet_in.system_status, packet2.system_status);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_AI_VISION_CONTROL)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_AI_VISION_CONTROL packet_in{};
    packet_in.bounding_box = 5;
    packet_in.tracking = 72;
    packet_in.following = 139;

    mavlink::thaco::msg::CC_AI_VISION_CONTROL packet1{};
    mavlink::thaco::msg::CC_AI_VISION_CONTROL packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.bounding_box, packet2.bounding_box);
    EXPECT_EQ(packet1.tracking, packet2.tracking);
    EXPECT_EQ(packet1.following, packet2.following);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_AI_VISION_CONTROL)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_ai_vision_control_t packet_c {
         5, 72, 139
    };

    mavlink::thaco::msg::CC_AI_VISION_CONTROL packet_in{};
    packet_in.bounding_box = 5;
    packet_in.tracking = 72;
    packet_in.following = 139;

    mavlink::thaco::msg::CC_AI_VISION_CONTROL packet2{};

    mavlink_msg_cc_ai_vision_control_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.bounding_box, packet2.bounding_box);
    EXPECT_EQ(packet_in.tracking, packet2.tracking);
    EXPECT_EQ(packet_in.following, packet2.following);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_CONFIG_BEGIN)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_CONFIG_BEGIN packet_in{};
    packet_in.request_id = 963497464;
    packet_in.timeout_s = 963497672;

    mavlink::thaco::msg::CC_CONFIG_BEGIN packet1{};
    mavlink::thaco::msg::CC_CONFIG_BEGIN packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.timeout_s, packet2.timeout_s);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_CONFIG_BEGIN)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_config_begin_t packet_c {
         963497464, 963497672
    };

    mavlink::thaco::msg::CC_CONFIG_BEGIN packet_in{};
    packet_in.request_id = 963497464;
    packet_in.timeout_s = 963497672;

    mavlink::thaco::msg::CC_CONFIG_BEGIN packet2{};

    mavlink_msg_cc_config_begin_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.timeout_s, packet2.timeout_s);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_CONFIG_SET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_CONFIG_SET packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;
    packet_in.key = to_char_array("IJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRS");
    packet_in.value = to_char_array("UVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ");

    mavlink::thaco::msg::CC_CONFIG_SET packet1{};
    mavlink::thaco::msg::CC_CONFIG_SET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.transaction_id, packet2.transaction_id);
    EXPECT_EQ(packet1.key, packet2.key);
    EXPECT_EQ(packet1.value, packet2.value);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_CONFIG_SET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_config_set_t packet_c {
         963497464, 963497672, "IJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRS", "UVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ"
    };

    mavlink::thaco::msg::CC_CONFIG_SET packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;
    packet_in.key = to_char_array("IJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRS");
    packet_in.value = to_char_array("UVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQ");

    mavlink::thaco::msg::CC_CONFIG_SET packet2{};

    mavlink_msg_cc_config_set_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.transaction_id, packet2.transaction_id);
    EXPECT_EQ(packet_in.key, packet2.key);
    EXPECT_EQ(packet_in.value, packet2.value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_CONFIG_GET)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_CONFIG_GET packet_in{};
    packet_in.request_id = 963497464;
    packet_in.key = to_char_array("EFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNO");

    mavlink::thaco::msg::CC_CONFIG_GET packet1{};
    mavlink::thaco::msg::CC_CONFIG_GET packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.key, packet2.key);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_CONFIG_GET)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_config_get_t packet_c {
         963497464, "EFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNO"
    };

    mavlink::thaco::msg::CC_CONFIG_GET packet_in{};
    packet_in.request_id = 963497464;
    packet_in.key = to_char_array("EFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNO");

    mavlink::thaco::msg::CC_CONFIG_GET packet2{};

    mavlink_msg_cc_config_get_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.key, packet2.key);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_CONFIG_APPLY)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_CONFIG_APPLY packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;

    mavlink::thaco::msg::CC_CONFIG_APPLY packet1{};
    mavlink::thaco::msg::CC_CONFIG_APPLY packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.transaction_id, packet2.transaction_id);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_CONFIG_APPLY)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_config_apply_t packet_c {
         963497464, 963497672
    };

    mavlink::thaco::msg::CC_CONFIG_APPLY packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;

    mavlink::thaco::msg::CC_CONFIG_APPLY packet2{};

    mavlink_msg_cc_config_apply_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.transaction_id, packet2.transaction_id);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_CONFIG_CONFIRM)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_CONFIG_CONFIRM packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;

    mavlink::thaco::msg::CC_CONFIG_CONFIRM packet1{};
    mavlink::thaco::msg::CC_CONFIG_CONFIRM packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.transaction_id, packet2.transaction_id);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_CONFIG_CONFIRM)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_config_confirm_t packet_c {
         963497464, 963497672
    };

    mavlink::thaco::msg::CC_CONFIG_CONFIRM packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;

    mavlink::thaco::msg::CC_CONFIG_CONFIRM packet2{};

    mavlink_msg_cc_config_confirm_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.transaction_id, packet2.transaction_id);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_CONFIG_ROLLBACK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_CONFIG_ROLLBACK packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;

    mavlink::thaco::msg::CC_CONFIG_ROLLBACK packet1{};
    mavlink::thaco::msg::CC_CONFIG_ROLLBACK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.transaction_id, packet2.transaction_id);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_CONFIG_ROLLBACK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_config_rollback_t packet_c {
         963497464, 963497672
    };

    mavlink::thaco::msg::CC_CONFIG_ROLLBACK packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;

    mavlink::thaco::msg::CC_CONFIG_ROLLBACK packet2{};

    mavlink_msg_cc_config_rollback_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.transaction_id, packet2.transaction_id);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_CONFIG_ACK)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_CONFIG_ACK packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;
    packet_in.result = 163;
    packet_in.apply_type = 230;
    packet_in.error_code = 17651;
    packet_in.message = to_char_array("MNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI");

    mavlink::thaco::msg::CC_CONFIG_ACK packet1{};
    mavlink::thaco::msg::CC_CONFIG_ACK packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.transaction_id, packet2.transaction_id);
    EXPECT_EQ(packet1.result, packet2.result);
    EXPECT_EQ(packet1.apply_type, packet2.apply_type);
    EXPECT_EQ(packet1.error_code, packet2.error_code);
    EXPECT_EQ(packet1.message, packet2.message);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_CONFIG_ACK)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_config_ack_t packet_c {
         963497464, 963497672, 17651, 163, 230, "MNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI"
    };

    mavlink::thaco::msg::CC_CONFIG_ACK packet_in{};
    packet_in.request_id = 963497464;
    packet_in.transaction_id = 963497672;
    packet_in.result = 163;
    packet_in.apply_type = 230;
    packet_in.error_code = 17651;
    packet_in.message = to_char_array("MNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI");

    mavlink::thaco::msg::CC_CONFIG_ACK packet2{};

    mavlink_msg_cc_config_ack_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.transaction_id, packet2.transaction_id);
    EXPECT_EQ(packet_in.result, packet2.result);
    EXPECT_EQ(packet_in.apply_type, packet2.apply_type);
    EXPECT_EQ(packet_in.error_code, packet2.error_code);
    EXPECT_EQ(packet_in.message, packet2.message);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_CONFIG_VALUE)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_CONFIG_VALUE packet_in{};
    packet_in.request_id = 963497464;
    packet_in.key = to_char_array("EFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNO");
    packet_in.value = to_char_array("QRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM");

    mavlink::thaco::msg::CC_CONFIG_VALUE packet1{};
    mavlink::thaco::msg::CC_CONFIG_VALUE packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.request_id, packet2.request_id);
    EXPECT_EQ(packet1.key, packet2.key);
    EXPECT_EQ(packet1.value, packet2.value);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_CONFIG_VALUE)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_config_value_t packet_c {
         963497464, "EFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNO", "QRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM"
    };

    mavlink::thaco::msg::CC_CONFIG_VALUE packet_in{};
    packet_in.request_id = 963497464;
    packet_in.key = to_char_array("EFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNO");
    packet_in.value = to_char_array("QRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLM");

    mavlink::thaco::msg::CC_CONFIG_VALUE packet2{};

    mavlink_msg_cc_config_value_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.request_id, packet2.request_id);
    EXPECT_EQ(packet_in.key, packet2.key);
    EXPECT_EQ(packet_in.value, packet2.value);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif

TEST(thaco, CC_CONFIG_STATUS)
{
    mavlink::mavlink_message_t msg;
    mavlink::MsgMap map1(msg);
    mavlink::MsgMap map2(msg);

    mavlink::thaco::msg::CC_CONFIG_STATUS packet_in{};
    packet_in.transaction_id = 963497464;
    packet_in.state = 151;
    packet_in.progress = 218;
    packet_in.error_code = 17443;
    packet_in.message = to_char_array("IJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDE");

    mavlink::thaco::msg::CC_CONFIG_STATUS packet1{};
    mavlink::thaco::msg::CC_CONFIG_STATUS packet2{};

    packet1 = packet_in;

    //std::cout << packet1.to_yaml() << std::endl;

    packet1.serialize(map1);

    mavlink::mavlink_finalize_message(&msg, 1, 1, packet1.MIN_LENGTH, packet1.LENGTH, packet1.CRC_EXTRA);

    packet2.deserialize(map2);

    EXPECT_EQ(packet1.transaction_id, packet2.transaction_id);
    EXPECT_EQ(packet1.state, packet2.state);
    EXPECT_EQ(packet1.progress, packet2.progress);
    EXPECT_EQ(packet1.error_code, packet2.error_code);
    EXPECT_EQ(packet1.message, packet2.message);
}

#ifdef TEST_INTEROP
TEST(thaco_interop, CC_CONFIG_STATUS)
{
    mavlink_message_t msg;

    // to get nice print
    memset(&msg, 0, sizeof(msg));

    mavlink_cc_config_status_t packet_c {
         963497464, 17443, 151, 218, "IJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDE"
    };

    mavlink::thaco::msg::CC_CONFIG_STATUS packet_in{};
    packet_in.transaction_id = 963497464;
    packet_in.state = 151;
    packet_in.progress = 218;
    packet_in.error_code = 17443;
    packet_in.message = to_char_array("IJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDE");

    mavlink::thaco::msg::CC_CONFIG_STATUS packet2{};

    mavlink_msg_cc_config_status_encode(1, 1, &msg, &packet_c);

    // simulate message-handling callback
    [&packet2](const mavlink_message_t *cmsg) {
        MsgMap map2(cmsg);

        packet2.deserialize(map2);
    } (&msg);

    EXPECT_EQ(packet_in.transaction_id, packet2.transaction_id);
    EXPECT_EQ(packet_in.state, packet2.state);
    EXPECT_EQ(packet_in.progress, packet2.progress);
    EXPECT_EQ(packet_in.error_code, packet2.error_code);
    EXPECT_EQ(packet_in.message, packet2.message);

#ifdef PRINT_MSG
    PRINT_MSG(msg);
#endif
}
#endif
