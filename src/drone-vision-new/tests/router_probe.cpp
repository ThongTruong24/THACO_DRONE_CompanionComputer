#include "bridge.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <condition_variable>
#include <iostream>
#include <iomanip>

using namespace vision;
using namespace std::chrono_literals;
void require(bool condition,const char* error) {if(!condition)throw std::runtime_error(error);}
int main(int argc,char** argv) {
    try {
        require(argc==4,"usage: router-probe TCP_PORT UDP_PORT router|cc-agent");
        int tcp_port=std::stoi(argv[1]),udp_port=std::stoi(argv[2]);
        bool agent=std::string(argv[3])=="cc-agent";
        std::mutex mutex;std::condition_variable changed;std::vector<mavlink_message_t> received;
        Transport transport(Config("127.0.0.1",tcp_port,1,192),[&](const auto& message) {
            std::lock_guard lock(mutex);received.push_back(message);changed.notify_all();
        });
        transport.start();
        auto until=std::chrono::steady_clock::now()+3s;
        while(!transport.connected()&&std::chrono::steady_clock::now()<until)std::this_thread::sleep_for(10ms);
        require(transport.connected(),"new Transport did not connect to real router");
        int udp=socket(AF_INET,SOCK_DGRAM,0);require(udp>=0,"UDP participant socket failed");
        sockaddr_in address{};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
        require(bind(udp,(sockaddr*)&address,sizeof(address))==0,"UDP participant bind failed");
        address.sin_port=htons(udp_port);require(connect(udp,(sockaddr*)&address,sizeof(address))==0,"UDP participant connect failed");
        Codec other(20,200);auto packet=other.heartbeat();
        require(send(udp,packet.data(),packet.size(),0)==(ssize_t)packet.size(),"UDP heartbeat send failed");
        auto wait=[&](uint32_t id,uint8_t system,uint8_t component) {
            std::unique_lock lock(mutex);
            auto find=[&]() {return std::find_if(received.begin(),received.end(),[&](const auto& m){return m.msgid==id&&m.sysid==system&&m.compid==component;});};
            require(changed.wait_for(lock,5s,[&]{return find()!=received.end();}),"real routed MAVLink receive timeout");
            auto iterator=find();auto result=*iterator;received.erase(iterator);return result;
        };
        auto heartbeat=wait(0,20,200);mavlink_heartbeat_t hb{};mavlink_msg_heartbeat_decode(&heartbeat,&hb);
        require(hb.type==MAV_TYPE_ONBOARD_CONTROLLER&&hb.autopilot==MAV_AUTOPILOT_INVALID&&hb.system_status==MAV_STATE_ACTIVE,"UDP -> router -> new TCP client payload mismatch");
        if(agent)wait(0,1,191);
        Codec local(1,192);auto request=local.config_get(0x5a170001,"telemetry.fc.baudrate");
        auto started=std::chrono::steady_clock::now();
        std::cout<<std::fixed<<std::setprecision(6)<<"TX timestamp="<<std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count()
                 <<" name=CC_CONFIG_GET id=42102 sysid=1 compid=192 target_sysid=none target_compid=none request_id=0x5a170001 endpoint=127.0.0.1:"<<tcp_port<<'\n';
        require(transport.send(request),"new TCP client TX queue rejected read-only GET");
        Decoder decoder;bool delivered=false;until=std::chrono::steady_clock::now()+5s;
        while(!delivered&&std::chrono::steady_clock::now()<until) {
            pollfd p{udp,POLLIN,0};if(poll(&p,1,100)<=0)continue;
            uint8_t bytes[4096];auto count=recv(udp,bytes,sizeof(bytes),0);require(count>0,"UDP participant receive failed");
            for(const auto& message:decoder.feed(Bytes(bytes,bytes+count)))if(message.msgid==42102) {
                mavlink_cc_config_get_t get{};mavlink_msg_cc_config_get_decode(&message,&get);
                require(message.sysid==1&&message.compid==192&&get.request_id==0x5a170001&&std::string(get.key)=="telemetry.fc.baudrate","TCP -> router -> UDP payload mismatch");delivered=true;
            }
        }
        require(delivered,"real router did not forward TCP GET to UDP participant");
        if(agent) {
            auto response=wait(42107,1,191);mavlink_cc_config_value_t value{};mavlink_msg_cc_config_value_decode(&response,&value);
            require(value.request_id==0x5a170001&&std::string(value.key)=="telemetry.fc.baudrate"&&std::string(value.value)=="921600","unchanged cc-agent response payload mismatch");
            std::cout<<"RX timestamp="<<std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count()
                     <<" name=CC_CONFIG_VALUE id=42107 sysid=1 compid=191 target_sysid=none target_compid=none value=921600 latency_ms="
                     <<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count()<<'\n';
        }
        close(udp);transport.stop();std::cout<<"PASS real router bidirectional "<<(agent?"+ unchanged cc-agent roundtrip":"TCP/UDP routing")<<'\n';
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
