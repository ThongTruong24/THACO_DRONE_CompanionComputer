#include "bridge.hpp"
#include <cmath>
#include <cstring>
#include <limits>

namespace vision {
using namespace std::chrono_literals;
namespace {
bool unit(float value) { return std::isfinite(value) && value>=0 && value<=1; }
}

CameraProtocol::CameraProtocol(Config config)
    :config_(std::move(config)),codec_(config_.system_id,config_.camera_component_id) {
    if(config_.camera_component_id<MAV_COMP_ID_CAMERA || config_.camera_component_id>MAV_COMP_ID_CAMERA6)
        throw std::invalid_argument("mavlink.camera_component_id must be in 100..105");
    if(config_.camera_component_id==config_.component_id)
        throw std::invalid_argument("camera alias must differ from primary Vision component");
    for(double value:{config_.status_max_hz,config_.selection_ttl_s,config_.selection_stale_s})
        if(!std::isfinite(value) || value<=0)
            throw std::invalid_argument("camera rates and selection timeouts must be finite and positive");
    if(config_.status_max_hz>10)
        throw std::invalid_argument("mavlink.tracking_status_max_hz must not exceed 10 Hz");
    if(config_.selection_ttl_s>86400 || config_.selection_stale_s>86400)
        throw std::invalid_argument("selection timeouts must not exceed one day");
}

Bytes CameraProtocol::heartbeat() { return codec_.heartbeat(MAV_TYPE_CAMERA); }

Bytes CameraProtocol::information(TimePoint now) {
    mavlink_camera_information_t info{};
    info.time_boot_ms=std::chrono::duration_cast<std::chrono::milliseconds>(now-boot_).count();
    info.focal_length=info.sensor_size_h=info.sensor_size_v=std::numeric_limits<float>::quiet_NaN();
    info.flags=CAMERA_CAP_FLAGS_HAS_TRACKING_POINT;
    info.resolution_h=width_; info.resolution_v=height_;
    constexpr char vendor[]="THACO", model[]="AgriDrone Vision Selector";
    std::memcpy(info.vendor_name,vendor,sizeof(vendor)); std::memcpy(info.model_name,model,sizeof(model));
    // No definition URI, associated gimbal, capture, zoom, storage or auto stream.
    return codec_.camera_information(info);
}

Bytes CameraProtocol::status(TimePoint now) {
    if(!point_pending_ || now>=expires_ || std::chrono::duration<double>(now-updated_).count()>config_.selection_stale_s)
        bbox_.reset();
    mavlink_camera_tracking_image_status_t s{};
    const float unknown=std::numeric_limits<float>::quiet_NaN();
    s.point_x=s.point_y=s.radius=s.rec_top_x=s.rec_top_y=s.rec_bottom_x=s.rec_bottom_y=unknown;
    s.tracking_status=CAMERA_TRACKING_STATUS_FLAGS_IDLE; s.tracking_mode=CAMERA_TRACKING_MODE_NONE;
    if(bbox_ && width_ && height_) {
        const auto& b=*bbox_;
        s.tracking_status=CAMERA_TRACKING_STATUS_FLAGS_ACTIVE; s.tracking_mode=CAMERA_TRACKING_MODE_POINT;
        s.target_data=CAMERA_TRACKING_TARGET_DATA_IN_STATUS|CAMERA_TRACKING_TARGET_DATA_RENDERED;
        s.point_x=(b[0]+b[2])/2; s.point_y=(b[1]+b[3])/2;
        // Circle around the selected CURRENT bbox; normalize by image WIDTH.
        s.radius=std::min(1.0f,std::hypot((b[2]-b[0])/2,(b[3]-b[1])*height_/width_/2));
    }
    return codec_.tracking_status(s);
}

CameraResponse CameraProtocol::handle(const mavlink_message_t& message,TimePoint now) {
    CameraResponse response;
    if(message.msgid!=MAVLINK_MSG_ID_COMMAND_LONG)return response;
    mavlink_command_long_t c{}; mavlink_msg_command_long_decode(&message,&c);
    if((c.target_system!=config_.system_id && c.target_system!=0) ||
       (c.target_component!=config_.camera_component_id && c.target_component!=0))return response;
    std::lock_guard lock(mutex_);
    uint8_t result=MAV_RESULT_ACCEPTED; uint32_t reply_id=0;
    switch(c.command) {
    case MAV_CMD_REQUEST_MESSAGE:
        if(c.param1==MAVLINK_MSG_ID_CAMERA_INFORMATION)reply_id=MAVLINK_MSG_ID_CAMERA_INFORMATION;
        else if(c.param1==MAVLINK_MSG_ID_CAMERA_TRACKING_IMAGE_STATUS)reply_id=MAVLINK_MSG_ID_CAMERA_TRACKING_IMAGE_STATUS;
        else result=MAV_RESULT_UNSUPPORTED;
        break;
    case MAV_CMD_REQUEST_CAMERA_INFORMATION:
        reply_id=MAVLINK_MSG_ID_CAMERA_INFORMATION; break;
    case MAV_CMD_CAMERA_TRACK_POINT:
        if(!unit(c.param1) || !unit(c.param2) || !unit(c.param3))result=MAV_RESULT_DENIED;
        else {
            bbox_.reset(); point_pending_=true;
            expires_=now+std::chrono::duration_cast<TimePoint::duration>(std::chrono::duration<double>(config_.selection_ttl_s));
            response.event=CameraEvent{false,c.param1,c.param2,c.param3,++generation_};
        }
        break;
    case MAV_CMD_CAMERA_STOP_TRACKING:
        bbox_.reset(); point_pending_=false;
        response.event=CameraEvent{true,0,0,0,++generation_}; break;
    case MAV_CMD_SET_MESSAGE_INTERVAL:
        if(c.param1!=MAVLINK_MSG_ID_CAMERA_TRACKING_IMAGE_STATUS)result=MAV_RESULT_UNSUPPORTED;
        else if(!std::isfinite(c.param2) || (c.param2<0 && c.param2!=-1))result=MAV_RESULT_DENIED;
        else if(c.param2==-1)interval_=0us;
        else {
            const double minimum=std::ceil(1000000/config_.status_max_hz);
            const double requested=c.param2==0?minimum:std::max<double>(minimum,c.param2);
            // Bound conversion and scheduling to one day; never overflow chrono.
            if(requested>86400000000.0)result=MAV_RESULT_DENIED;
            else {
                interval_=std::chrono::microseconds(static_cast<int64_t>(std::ceil(requested)));
                next_status_=std::max(now,last_status_+interval_);
            }
        }
        break;
    default: result=MAV_RESULT_UNSUPPORTED; break;
    }
    // QGC requestMessage waits for command acknowledgement AND requested data.
    response.packets.push_back(codec_.ack(c.command,result,message.sysid,message.compid));
    if(reply_id==MAVLINK_MSG_ID_CAMERA_INFORMATION)response.packets.push_back(information(now));
    else if(reply_id==MAVLINK_MSG_ID_CAMERA_TRACKING_IMAGE_STATUS)response.packets.push_back(status(now));
    return response;
}

void CameraProtocol::update_selection(uint64_t generation,std::optional<std::array<float,4>> bbox,
                                      uint16_t width,uint16_t height,TimePoint now) {
    if(bbox && (!width || !height || !std::all_of(bbox->begin(),bbox->end(),unit) ||
                (*bbox)[0]>(*bbox)[2] || (*bbox)[1]>(*bbox)[3]))
        throw std::invalid_argument("camera selection bbox must be finite normalized coordinates with valid dimensions");
    std::lock_guard lock(mutex_); width_=width; height_=height;
    if(generation!=generation_ || !point_pending_ || now>=expires_)return;
    bbox_=bbox; updated_=now;
}

std::optional<Bytes> CameraProtocol::poll(TimePoint now) {
    std::lock_guard lock(mutex_);
    if(interval_.count()==0 || now<next_status_)return {};
    last_status_=now;
    next_status_=now+interval_; // No catch-up bursts after congestion/reconnect.
    return status(now);
}

void CameraProtocol::reset() {
    // Link loss clears selection, but the logical camera remains the same.
    // Keep an enabled interval so reconnect reports IDLE and clears QGC's old
    // overlay even when QGC retains its camera object across a short outage.
    std::lock_guard lock(mutex_); bbox_.reset(); point_pending_=false; ++generation_;
}
}
