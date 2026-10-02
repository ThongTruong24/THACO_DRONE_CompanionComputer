#include "bridge.hpp"
#include <nlohmann/json.hpp>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <csignal>
#include <cmath>
#include <iostream>

using json=nlohmann::json;
using namespace vision;
static volatile sig_atomic_t running=1;
static void signal_handler(int) { running=0; }
static double timestamp() {
    return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
}
// Bounded event pipe. Selection, connection state and each heartbeat are latest-only.
class Events {
    struct Item { json value; std::string key; };
    std::mutex mutex_;std::deque<Item> events_;
public:
    void put(json value,std::string key="") {
        value["timestamp"]=timestamp();std::lock_guard lock(mutex_);
        if(!key.empty())for(auto& event:events_)if(event.key==key) {event.value=std::move(value);return;}
        if(events_.size()==64)events_.pop_front();events_.push_back({std::move(value),std::move(key)});
    }
    std::string pop() {
        std::lock_guard lock(mutex_);if(events_.empty())return {};
        auto value=events_.front().value.dump()+"\n";events_.pop_front();return value;
    }
};
template<size_t N> static std::string field(const char (&data)[N]) {
    return std::string(data,strnlen(data,N));
}
int main(int argc,char** argv) {
    try {
        Config config;
        bool self_test=false;
        for(int i=1;i<argc;i++) {
            std::string key=argv[i];
            if(key=="--self-test") {self_test=true;continue;}
            if(i+1>=argc)throw std::invalid_argument("missing argument value");
            std::string value=argv[++i];
            if(key=="--host")config.host=value;
            else if(key=="--status-max-hz" || key=="--selection-ttl-s" || key=="--selection-stale-s") {
                size_t used=0;double number=std::stod(value,&used);
                if(used!=value.size())throw std::invalid_argument("numeric argument required");
                if(key=="--status-max-hz")config.status_max_hz=number;
                else if(key=="--selection-ttl-s")config.selection_ttl_s=number;
                else config.selection_stale_s=number;
            }
            else {
                size_t used=0;int number=std::stoi(value,&used);
                if(used!=value.size())throw std::invalid_argument("integer argument required");
                if(key=="--port")config.port=number;
                else if(key=="--system-id")config.system_id=number;
                else if(key=="--component-id")config.component_id=number;
                else if(key=="--camera-component-id")config.camera_component_id=number;
                else if(key=="--reconnect-min-ms")config.reconnect_min_ms=number;
                else if(key=="--reconnect-max-ms")config.reconnect_max_ms=number;
                else throw std::invalid_argument("unknown argument: "+key);
            }
        }
        Config checked(config.host,config.port,config.system_id,config.component_id);
        (void)checked;
        Events events;
        Transport* pointer=nullptr;
        auto endpoint=config.host+":"+std::to_string(config.port);
        Transport transport(config,[&](const mavlink_message_t& m) {
            json metadata={{"sysid",m.sysid},{"compid",m.compid},{"message_id",m.msgid},{"endpoint",endpoint}};
            if(m.msgid==MAVLINK_MSG_ID_HEARTBEAT) {
                metadata["type"]="heartbeat";metadata["name"]="HEARTBEAT";
                events.put(metadata,"heartbeat:"+std::to_string(m.sysid)+":"+std::to_string(m.compid));
            } else if(m.msgid==MAVLINK_MSG_ID_CC_CONFIG_VALUE) {
                mavlink_cc_config_value_t payload{};mavlink_msg_cc_config_value_decode(&m,&payload);
                metadata["type"]="config_value";metadata["name"]="CC_CONFIG_VALUE";
                metadata["request_id"]=payload.request_id;metadata["key"]=field(payload.key);metadata["value"]=field(payload.value);
                metadata["target_sysid"]=nullptr;metadata["target_compid"]=nullptr;events.put(metadata);
            } else if(m.msgid==MAVLINK_MSG_ID_COMMAND_LONG) {
                auto response=pointer->camera().handle(m);
                for(auto& packet:response.packets)pointer->send(std::move(packet));
                if(response.event) {
                    mavlink_command_long_t command{};mavlink_msg_command_long_decode(&m,&command);
                    const auto& event=*response.event;
                    metadata["type"]=event.clear?"clear_selection":"track_point";
                    metadata["name"]="COMMAND_LONG";metadata["command"]=command.command;
                    metadata["x"]=event.x;metadata["y"]=event.y;metadata["radius"]=event.radius;
                    metadata["generation"]=event.generation;
                    metadata["target_sysid"]=command.target_system;metadata["target_compid"]=command.target_component;
                    events.put(metadata,"selection");
                }
            }
        },[&](bool connected) {
            events.put({{"type","connection"},{"connected",connected},{"endpoint",endpoint}},"connection");
        });
        pointer=&transport;
        auto& codec=transport.primary_codec();
        if(self_test) {
            Decoder decoder;auto messages=decoder.feed(codec.config_get(0x5a170001,"telemetry.fc.baudrate"));
            if(messages.size()!=1 || messages[0].msgid!=42102)throw std::runtime_error("codec self-test failed");
            auto cameras=decoder.feed(transport.camera().heartbeat());
            if(cameras.size()!=1 || cameras[0].compid!=config.camera_component_id)throw std::runtime_error("camera codec self-test failed");
            std::cout<<json({{"status","ok"},{"dialect","thaco_common"},{"sysid",config.system_id},{"compid",config.component_id},
                            {"camera_compid",config.camera_component_id}}).dump()<<'\n';return 0;
        }
        signal(SIGINT,signal_handler);signal(SIGTERM,signal_handler);signal(SIGPIPE,SIG_IGN);
        fcntl(STDIN_FILENO,F_SETFL,fcntl(STDIN_FILENO,F_GETFL)|O_NONBLOCK);
        fcntl(STDOUT_FILENO,F_SETFL,fcntl(STDOUT_FILENO,F_GETFL)|O_NONBLOCK);
        events.put({{"type","connection"},{"connected",false},{"endpoint",endpoint}},"connection");transport.start();
        std::string input,output;size_t offset=0;
        while(running) {
            if(output.empty()) {output=events.pop();offset=0;}
            pollfd descriptors[2]={{STDIN_FILENO,POLLIN,0},{STDOUT_FILENO,static_cast<short>(output.empty()?0:POLLOUT),0}};
            if(poll(descriptors,2,20)<0&&errno!=EINTR)break;
            if(descriptors[0].revents&POLLIN) {
                char data[4096];auto count=read(STDIN_FILENO,data,sizeof(data));
                if(count==0)break;
                if(count>0)input.append(data,count);
                size_t end;
                while((end=input.find('\n'))!=std::string::npos) {
                    auto line=input.substr(0,end);input.erase(0,end+1);
                    try {
                        if(line.size()>16384)throw std::invalid_argument("IPC line exceeds 16 KiB");
                        auto request=json::parse(line);auto type=request.at("type").get<std::string>();
                        if(type=="get") {
                            auto id=request.at("request_id").get<uint32_t>();auto key=request.at("key").get<std::string>();
                            bool queued=transport.send(codec.config_get(id,key));
                            events.put({{"type","tx"},{"name","CC_CONFIG_GET"},{"message_id",42102},{"sysid",config.system_id},
                                {"compid",config.component_id},{"request_id",id},{"key",key},{"queued",queued},{"endpoint",endpoint},
                                {"target_sysid",nullptr},{"target_compid",nullptr}});
                        } else if(type=="selection_state") {
                            std::optional<std::array<float,4>> bbox;
                            if(!request.at("bbox").is_null())bbox=request.at("bbox").get<std::array<float,4>>();
                            auto width=request.at("width").get<int>(),height=request.at("height").get<int>();
                            if(width<0 || width>65535 || height<0 || height>65535)throw std::invalid_argument("invalid selection frame size");
                            transport.camera().update_selection(request.at("generation").get<uint64_t>(),bbox,width,height);
                        } else if(type=="telemetry") {
                            mavlink_cc_telemetry_vision_t payload{};
                            payload.confidence_thresh=request.at("confidence").get<float>();
                            payload.inference_fps=request.at("inference_fps").get<float>();
                            payload.input_width=request.at("width").get<uint16_t>();payload.input_height=request.at("height").get<uint16_t>();
                            payload.video_fps=request.at("fps").get<uint8_t>();payload.detections_count=request.at("count").get<uint8_t>();
                            payload.status_flags=1;
                            auto model=request.at("model").get<std::string>();auto source=std::string("rtsp");
                            std::copy_n(model.c_str(),std::min(model.size(),sizeof(payload.model_name)-1),payload.model_name);
                            std::copy(source.begin(),source.end(),payload.input_source);
                            transport.send(codec.telemetry(payload),true);
                        } else throw std::invalid_argument("unsupported IPC command");
                    } catch(const std::exception& error) {events.put({{"type","error"},{"error",error.what()}},"error");}
                }
                if(input.size()>16384) {input.clear();events.put({{"type","error"},{"error","unterminated IPC line exceeds 16 KiB"}},"error");}
            }
            if(descriptors[0].revents&(POLLHUP|POLLERR|POLLNVAL))break;
            if(descriptors[1].revents&(POLLHUP|POLLERR|POLLNVAL))break;
            if(!output.empty()&&(descriptors[1].revents&POLLOUT)) {
                auto count=write(STDOUT_FILENO,output.data()+offset,output.size()-offset);
                if(count>0) {offset+=count;if(offset==output.size())output.clear();}
                else if(count<0&&errno!=EAGAIN&&errno!=EWOULDBLOCK&&errno!=EINTR)break;
            }
        }
        transport.stop();return 0;
    } catch(const std::exception& error) {std::cerr<<"mavlink-bridge: "<<error.what()<<'\n';return 2;}
}
