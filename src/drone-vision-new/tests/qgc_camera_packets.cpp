// Intentionally independent of bridge.hpp / Raspberry's generated dialect.
// CMake builds this ONLY with explicitly supplied QGC "all" generated headers.
#include <mavlink.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <iterator>
#include <cstring>

using json=nlohmann::json;
int main(int argc,char** argv) {
    if(argc==2 && std::string(argv[1])=="metadata") {
        std::cout<<json({{"dialect","all"},{"revision",QGC_DIALECT_REVISION},
                        {"information_crc",MAVLINK_MSG_ID_CAMERA_INFORMATION_CRC},
                        {"status_crc",MAVLINK_MSG_ID_CAMERA_TRACKING_IMAGE_STATUS_CRC}}).dump(); return 0;
    }
    if(argc==2 && std::string(argv[1])=="decode") {
        std::vector<uint8_t> bytes(std::istreambuf_iterator<char>(std::cin),{});
        mavlink_message_t state{},m{}; mavlink_status_t parser{},output{}; json rows=json::array();
        for(auto byte:bytes)if(mavlink_frame_char_buffer(&state,&parser,byte,&m,&output)==MAVLINK_FRAMING_OK) {
            json row={{"id",static_cast<uint32_t>(m.msgid)},{"sysid",m.sysid},{"compid",m.compid},{"sequence",m.seq}};
            if(m.msgid==MAVLINK_MSG_ID_HEARTBEAT) {
                mavlink_heartbeat_t p{}; mavlink_msg_heartbeat_decode(&m,&p);
                row["type"]=p.type; row["autopilot"]=p.autopilot;
            } else if(m.msgid==MAVLINK_MSG_ID_COMMAND_ACK) {
                mavlink_command_ack_t p{}; mavlink_msg_command_ack_decode(&m,&p);
                row["command"]=p.command; row["result"]=p.result;
                row["target_system"]=p.target_system; row["target_component"]=p.target_component;
            } else if(m.msgid==MAVLINK_MSG_ID_CAMERA_INFORMATION) {
                mavlink_camera_information_t p{}; mavlink_msg_camera_information_decode(&m,&p);
                row["flags"]=p.flags; row["width"]=p.resolution_h; row["height"]=p.resolution_v;
                row["vendor"]=std::string(reinterpret_cast<char*>(p.vendor_name),strnlen(reinterpret_cast<char*>(p.vendor_name),32));
                row["model"]=std::string(reinterpret_cast<char*>(p.model_name),strnlen(reinterpret_cast<char*>(p.model_name),32));
            } else if(m.msgid==MAVLINK_MSG_ID_CAMERA_TRACKING_IMAGE_STATUS) {
                mavlink_camera_tracking_image_status_t p{}; mavlink_msg_camera_tracking_image_status_decode(&m,&p);
                row["tracking_status"]=p.tracking_status; row["tracking_mode"]=p.tracking_mode;
                row["target_data"]=p.target_data; row["x"]=p.point_x; row["y"]=p.point_y; row["radius"]=p.radius;
            } else if(m.msgid==MAVLINK_MSG_ID_CC_TELEMETRY_VISION) {
                mavlink_cc_telemetry_vision_t p{}; mavlink_msg_cc_telemetry_vision_decode(&m,&p);
                row["width"]=p.input_width; row["height"]=p.input_height; row["count"]=p.detections_count;
            }
            rows.push_back(row);
        }
        std::cout<<rows.dump(); return 0;
    }
    mavlink_message_t m{}; mavlink_status_t status{};
    if(argc==2 && std::string(argv[1])=="heartbeat")
        mavlink_msg_heartbeat_pack_status(255,190,&status,&m,MAV_TYPE_GCS,MAV_AUTOPILOT_INVALID,0,0,MAV_STATE_ACTIVE);
    else if(argc>=5 && argc<=12 && std::string(argv[1])=="command") {
        float p[7]={}; for(int i=5;i<argc;++i)p[i-5]=std::stof(argv[i]);
        mavlink_msg_command_long_pack_status(255,190,&status,&m,std::stoi(argv[2]),std::stoi(argv[3]),std::stoi(argv[4]),
                                            0,p[0],p[1],p[2],p[3],p[4],p[5],p[6]);
    } else return 2;
    uint8_t bytes[MAVLINK_MAX_PACKET_LEN];auto n=mavlink_msg_to_send_buffer(bytes,&m);
    std::cout.write(reinterpret_cast<char*>(bytes),n);
}
