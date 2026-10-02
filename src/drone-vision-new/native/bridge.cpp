#include "bridge.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <algorithm>

namespace vision {
using Clock=std::chrono::steady_clock;
using namespace std::chrono_literals;
Config::Config(std::string h,int p,int s,int c):host(std::move(h)),port(p),system_id(s),component_id(c) {
    in_addr address{};
    if(inet_pton(AF_INET,host.c_str(),&address)!=1 || p<1 || p>65535 || s<1 || s>255 || c<1 || c>255 || c==191)
        throw std::invalid_argument("MAVLink requires numeric IPv4 host, port 1..65535 and IDs 1..255");
}
bool Config::targets(int system,int component) const { return system==system_id && component==component_id; }
std::vector<mavlink_message_t> Decoder::feed(const Bytes& bytes) {
    std::vector<mavlink_message_t> messages;
    for(auto byte:bytes) {
        mavlink_message_t message{};mavlink_status_t output{};
        if(mavlink_frame_char_buffer(&state_,&status_,byte,&message,&output)==MAVLINK_FRAMING_OK)
            messages.push_back(message);
    }
    return messages;
}
void Decoder::reset() { state_={};status_={}; }
static Bytes serialize(const mavlink_message_t& m) {
    Bytes bytes(MAVLINK_MAX_PACKET_LEN);
    bytes.resize(mavlink_msg_to_send_buffer(bytes.data(),&m));return bytes;
}
Bytes Codec::heartbeat(uint8_t type) {
    std::lock_guard lock(mutex_);mavlink_message_t m{};
    mavlink_msg_heartbeat_pack_status(system_,component_,&status_,&m,type,
                                     MAV_AUTOPILOT_INVALID,0,0,MAV_STATE_ACTIVE);return serialize(m);
}
Bytes Codec::config_get(uint32_t id,const std::string& key) {
    if(key.empty()||key.size()>=64)throw std::invalid_argument("config key length must be 1..63");
    std::lock_guard lock(mutex_);mavlink_message_t m{};char field[64]{};
    std::copy(key.begin(),key.end(),field);
    mavlink_msg_cc_config_get_pack_status(system_,component_,&status_,&m,id,field);return serialize(m);
}
Bytes Codec::ack(uint16_t cmd,uint8_t result,uint8_t sys,uint8_t comp) {
    std::lock_guard lock(mutex_);mavlink_message_t m{};
    mavlink_msg_command_ack_pack_status(system_,component_,&status_,&m,cmd,result,0,0,sys,comp);return serialize(m);
}
Bytes Codec::telemetry(const mavlink_cc_telemetry_vision_t& payload) {
    std::lock_guard lock(mutex_);mavlink_message_t m{};
    mavlink_msg_cc_telemetry_vision_encode_status(system_,component_,&status_,&m,&payload);return serialize(m);
}
Bytes Codec::camera_information(const mavlink_camera_information_t& payload) {
    std::lock_guard lock(mutex_); mavlink_message_t m{};
    mavlink_msg_camera_information_encode_status(system_,component_,&status_,&m,&payload); return serialize(m);
}
Bytes Codec::tracking_status(const mavlink_camera_tracking_image_status_t& payload) {
    std::lock_guard lock(mutex_); mavlink_message_t m{};
    mavlink_msg_camera_tracking_image_status_encode_status(system_,component_,&status_,&m,&payload); return serialize(m);
}
bool PacketQueue::push(Bytes bytes,bool latest) {
    std::lock_guard lock(mutex_);
    if(latest) for(auto& item:queue_) if(item.latest) { item.bytes=std::move(bytes);return true; }
    if(queue_.size()>=capacity_)return false;
    queue_.push_back({std::move(bytes),latest});return true;
}
std::optional<Bytes> PacketQueue::pop() {
    std::lock_guard lock(mutex_);if(queue_.empty())return {};
    Bytes bytes=std::move(queue_.front().bytes);queue_.pop_front();return bytes;
}
size_t PacketQueue::size() const { std::lock_guard lock(mutex_);return queue_.size(); }
void PacketQueue::clear() { std::lock_guard lock(mutex_);queue_.clear(); }
Transport::Transport(Config c,std::function<void(const mavlink_message_t&)> m,std::function<void(bool)> connection)
    :config_(std::move(c)),codec_(config_.system_id,config_.component_id),camera_(config_),message_(std::move(m)),connection_(std::move(connection)) {
    if(config_.reconnect_min_ms<1||config_.reconnect_max_ms<config_.reconnect_min_ms||config_.reconnect_max_ms>60000||config_.connect_timeout_ms<1||config_.connect_timeout_ms>10000)
        throw std::invalid_argument("invalid reconnect/connect timeout configuration");
}
void Transport::start() {
    if(running_.exchange(true))return;
    outbound_.clear();worker_=std::thread(&Transport::run,this);
}
void Transport::stop() {
    running_=false;if(worker_.joinable())worker_.join();outbound_.clear();
}
bool Transport::send(Bytes bytes,bool latest) {
    if(!running_||!connected_)return false;
    return outbound_.push(std::move(bytes),latest);
}
void Transport::run() {
    int fd=-1;Decoder decoder;Bytes writing;size_t offset=0;
    Backoff backoff(config_.reconnect_min_ms,config_.reconnect_max_ms);
    auto retry=Clock::now(),heartbeat=Clock::now(),established=Clock::now();
    auto disconnect=[&] {
        if(fd>=0)close(fd);
        fd=-1;writing.clear();offset=0;decoder.reset();outbound_.clear();camera_.reset();
        if(connected_.exchange(false)&&connection_)connection_(false);
        retry=Clock::now()+std::chrono::milliseconds(backoff.next());
    };
    while(running_) {
        if(fd<0) {
            if(Clock::now()<retry) { std::this_thread::sleep_for(20ms);continue; }
            fd=socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,0);
            if(fd<0) { disconnect();continue; }
            sockaddr_in address{};address.sin_family=AF_INET;address.sin_port=htons(config_.port);
            inet_pton(AF_INET,config_.host.c_str(),&address.sin_addr);
            int result=connect(fd,reinterpret_cast<sockaddr*>(&address),sizeof(address));
            if(result<0&&errno==EINPROGRESS) {
                auto deadline=Clock::now()+std::chrono::milliseconds(config_.connect_timeout_ms);
                result=-1;
                while(running_&&Clock::now()<deadline) {
                    pollfd p{fd,POLLOUT,0};if(poll(&p,1,20)<=0)continue;
                    int error=0;socklen_t length=sizeof(error);
                    if(getsockopt(fd,SOL_SOCKET,SO_ERROR,&error,&length)==0&&error==0)result=0;
                    break;
                }
            }
            if(result!=0||!running_) { disconnect();continue; }
            decoder.reset();outbound_.clear();connected_=true;established=Clock::now();
            // Register both logical components on the SAME TCP session.
            writing=codec_.heartbeat();auto alias=camera_.heartbeat();
            writing.insert(writing.end(),alias.begin(),alias.end());offset=0;heartbeat=Clock::now()+1s;
            if(connection_)connection_(true);
        }
        if(Clock::now()-established>=1s)backoff.reset();
        if(writing.empty()) {
            if(auto packet=outbound_.pop())writing=std::move(*packet);
            else if(Clock::now()>=heartbeat) {
                writing=codec_.heartbeat();auto alias=camera_.heartbeat();
                writing.insert(writing.end(),alias.begin(),alias.end());heartbeat=Clock::now()+1s;
            } else if(auto packet=camera_.poll())writing=std::move(*packet);
            offset=0;
        }
        pollfd p{fd,static_cast<short>(POLLIN|(writing.empty()?0:POLLOUT)),0};
        int ready=poll(&p,1,20);
        if(ready<0) { if(errno!=EINTR)disconnect();continue; }
        if(p.revents&POLLIN) {
            uint8_t buffer[4096];
            auto n=recv(fd,buffer,sizeof(buffer),0);
            if(n==0) { disconnect();continue; }
            if(n<0&&errno!=EAGAIN&&errno!=EWOULDBLOCK&&errno!=EINTR) { disconnect();continue; }
            if(n>0)for(const auto& message:decoder.feed(Bytes(buffer,buffer+n)))message_(message);
        }
        if(p.revents&(POLLERR|POLLHUP|POLLNVAL)) { disconnect();continue; }
        if(!writing.empty()&&(p.revents&POLLOUT)) {
            auto n=::send(fd,writing.data()+offset,writing.size()-offset,MSG_NOSIGNAL);
            if(n>0) { offset+=n;if(offset==writing.size()) {writing.clear();offset=0;} }
            else if(n<0&&errno!=EAGAIN&&errno!=EWOULDBLOCK&&errno!=EINTR)disconnect();
        }
    }
    disconnect();
}
}
