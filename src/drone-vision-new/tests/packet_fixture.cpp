#include "bridge.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <iterator>
using namespace vision;
int main(int argc,char** argv) {
    if(argc==2&&std::string(argv[1])=="decode") {
        Bytes bytes(std::istreambuf_iterator<char>(std::cin),{});
        Decoder decoder;nlohmann::json result=nlohmann::json::array();
        for(const auto& message:decoder.feed(bytes)) {
            nlohmann::json row={{"id",message.msgid},{"sysid",message.sysid},{"compid",message.compid}};
            if(message.msgid==MAVLINK_MSG_ID_COMMAND_ACK) {
                mavlink_command_ack_t ack{};mavlink_msg_command_ack_decode(&message,&ack);
                row["command"]=ack.command;row["result"]=ack.result;
                row["target_sysid"]=ack.target_system;row["target_compid"]=ack.target_component;
            }
            result.push_back(row);
        }
        std::cout<<result.dump();return 0;
    }
    if(argc!=5||std::string(argv[1])!="track")return 2;
    mavlink_status_t status{};mavlink_message_t message{};
    mavlink_msg_command_long_pack_status(255,190,&status,&message,std::stoi(argv[2]),std::stoi(argv[3]),
        MAV_CMD_CAMERA_TRACK_POINT,0,std::stof(argv[4]),.75f,.05f,0,0,0,0);
    uint8_t bytes[MAVLINK_MAX_PACKET_LEN];auto size=mavlink_msg_to_send_buffer(bytes,&message);
    std::cout.write(reinterpret_cast<char*>(bytes),size);
}
