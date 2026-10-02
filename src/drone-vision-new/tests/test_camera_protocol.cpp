#include <gtest/gtest.h>
#include "bridge.hpp"
#include <cmath>
#include <limits>

using namespace vision;
using namespace std::chrono_literals;

namespace {
mavlink_message_t command(uint16_t id, float p1=0, float p2=0, float p3=0,
                          uint8_t system=1, uint8_t component=105) {
    mavlink_message_t m{}; mavlink_status_t status{};
    mavlink_msg_command_long_pack_status(255,190,&status,&m,system,component,id,0,p1,p2,p3,0,0,0,0);
    return m;
}
mavlink_message_t decode(const Bytes& bytes) {
    Decoder decoder; auto messages=decoder.feed(bytes);
    EXPECT_EQ(messages.size(),1u);
    return messages.empty()?mavlink_message_t{}:messages.front();
}
void expect_ack(const CameraResponse& response, uint16_t cmd, uint8_t result) {
    ASSERT_FALSE(response.packets.empty());
    auto m=decode(response.packets.front());
    EXPECT_EQ(m.msgid,MAVLINK_MSG_ID_COMMAND_ACK);
    EXPECT_EQ(m.sysid,1); EXPECT_EQ(m.compid,105);
    mavlink_command_ack_t ack{}; mavlink_msg_command_ack_decode(&m,&ack);
    EXPECT_EQ(ack.command,cmd); EXPECT_EQ(ack.result,result);
    EXPECT_EQ(ack.target_system,255); EXPECT_EQ(ack.target_component,190);
}
}

TEST(CameraProtocol, SeparateHeartbeatAndOnlyImplementedCapabilities) {
    Codec primary(1,192); CameraProtocol camera(Config{});
    auto primary_message=decode(primary.heartbeat()), alias=decode(camera.heartbeat());
    EXPECT_EQ(primary_message.compid,192); EXPECT_EQ(alias.compid,105);
    EXPECT_EQ(alias.sysid,1);
    mavlink_heartbeat_t hb{}; mavlink_msg_heartbeat_decode(&primary_message,&hb);
    EXPECT_EQ(hb.type,MAV_TYPE_ONBOARD_CONTROLLER);
    mavlink_msg_heartbeat_decode(&alias,&hb);
    EXPECT_EQ(hb.type,MAV_TYPE_CAMERA); EXPECT_EQ(hb.autopilot,MAV_AUTOPILOT_INVALID);
    EXPECT_EQ(hb.base_mode,0); EXPECT_EQ(hb.custom_mode,0); EXPECT_EQ(hb.system_status,MAV_STATE_ACTIVE);
    for(auto cmd:{MAV_CMD_REQUEST_MESSAGE,MAV_CMD_REQUEST_CAMERA_INFORMATION}) {
        auto response=camera.handle(command(cmd,cmd==MAV_CMD_REQUEST_MESSAGE?259:1));
        expect_ack(response,cmd,MAV_RESULT_ACCEPTED);
        ASSERT_EQ(response.packets.size(),2u);
        auto message=decode(response.packets[1]);
        EXPECT_EQ(message.seq,static_cast<uint8_t>(decode(response.packets[0]).seq+1));
        EXPECT_EQ(message.msgid,259u); EXPECT_EQ(message.compid,105);
        mavlink_camera_information_t info{}; mavlink_msg_camera_information_decode(&message,&info);
        EXPECT_EQ(info.flags,static_cast<uint32_t>(CAMERA_CAP_FLAGS_HAS_TRACKING_POINT));
        EXPECT_STREQ(reinterpret_cast<char*>(info.vendor_name),"THACO");
        EXPECT_STREQ(reinterpret_cast<char*>(info.model_name),"AgriDrone Vision Selector");
        EXPECT_EQ(info.camera_device_id,0); EXPECT_EQ(info.gimbal_device_id,0);
        EXPECT_EQ(info.cam_definition_uri[0],0); EXPECT_TRUE(std::isnan(info.focal_length));
    }
}

TEST(CameraProtocol, ConfigurationTargetsAndBroadcasts) {
    for(int id:{99,106,191,192,256}) {
        Config c; c.camera_component_id=id;
        EXPECT_THROW(CameraProtocol camera(c),std::invalid_argument);
    }
    for(int id=100;id<=105;++id) {
        Config c; c.camera_component_id=id; CameraProtocol camera(c);
        EXPECT_EQ(decode(camera.heartbeat()).compid,id);
    }
    CameraProtocol camera(Config{});
    for(auto target:{std::pair{1,192},std::pair{2,105},std::pair{1,104}}) {
        auto r=camera.handle(command(MAV_CMD_CAMERA_TRACK_POINT,.5,.5,.1,target.first,target.second));
        EXPECT_TRUE(r.packets.empty()); EXPECT_FALSE(r.event.has_value());
    }
    for(auto target:{std::pair{0,105},std::pair{1,0},std::pair{0,0}}) {
        auto r=camera.handle(command(MAV_CMD_REQUEST_MESSAGE,259,0,0,target.first,target.second));
        expect_ack(r,MAV_CMD_REQUEST_MESSAGE,MAV_RESULT_ACCEPTED);
    }
}

TEST(CameraProtocol, PointValidationAndStopRejectStaleSelectionUpdates) {
    CameraProtocol camera(Config{}); auto now=std::chrono::steady_clock::now();
    for(float invalid:{-0.01f,1.01f,std::numeric_limits<float>::quiet_NaN(),
                       std::numeric_limits<float>::infinity()}) {
        for(int parameter=0;parameter<3;++parameter) {
            float p[3]={.5f,.5f,.1f}; p[parameter]=invalid;
            auto r=camera.handle(command(MAV_CMD_CAMERA_TRACK_POINT,p[0],p[1],p[2]),now);
            expect_ack(r,MAV_CMD_CAMERA_TRACK_POINT,MAV_RESULT_DENIED); EXPECT_FALSE(r.event);
        }
    }
    auto point=camera.handle(command(MAV_CMD_CAMERA_TRACK_POINT,.5,.5,.1),now);
    expect_ack(point,MAV_CMD_CAMERA_TRACK_POINT,MAV_RESULT_ACCEPTED);
    ASSERT_TRUE(point.event); EXPECT_FALSE(point.event->clear);
    EXPECT_FLOAT_EQ(point.event->x,.5f); EXPECT_FLOAT_EQ(point.event->radius,.1f);
    auto interval=camera.handle(command(MAV_CMD_SET_MESSAGE_INTERVAL,275,500000),now);
    expect_ack(interval,MAV_CMD_SET_MESSAGE_INTERVAL,MAV_RESULT_ACCEPTED);
    camera.update_selection(point.event->generation,std::array<float,4>{.4f,.3f,.6f,.7f},640,360,now);
    auto active=camera.poll(now); ASSERT_TRUE(active);
    auto message=decode(*active); EXPECT_EQ(message.compid,105);
    mavlink_camera_tracking_image_status_t status{}; mavlink_msg_camera_tracking_image_status_decode(&message,&status);
    EXPECT_EQ(status.tracking_status,CAMERA_TRACKING_STATUS_FLAGS_ACTIVE);
    EXPECT_EQ(status.tracking_mode,CAMERA_TRACKING_MODE_POINT);
    EXPECT_EQ(status.target_data,CAMERA_TRACKING_TARGET_DATA_IN_STATUS|CAMERA_TRACKING_TARGET_DATA_RENDERED);
    EXPECT_FLOAT_EQ(status.point_x,.5f); EXPECT_FLOAT_EQ(status.point_y,.5f);
    EXPECT_TRUE(std::isnan(status.rec_top_x));
    auto stop=camera.handle(command(MAV_CMD_CAMERA_STOP_TRACKING),now+100ms);
    expect_ack(stop,MAV_CMD_CAMERA_STOP_TRACKING,MAV_RESULT_ACCEPTED); ASSERT_TRUE(stop.event);
    EXPECT_TRUE(stop.event->clear);
    camera.update_selection(point.event->generation,std::array<float,4>{.4f,.3f,.6f,.7f},640,360,now+200ms);
    auto idle=camera.poll(now+500ms); ASSERT_TRUE(idle);
    message=decode(*idle); mavlink_msg_camera_tracking_image_status_decode(&message,&status);
    EXPECT_EQ(status.tracking_status,CAMERA_TRACKING_STATUS_FLAGS_IDLE);
    EXPECT_EQ(status.tracking_mode,CAMERA_TRACKING_MODE_NONE); EXPECT_TRUE(std::isnan(status.point_x));
}

TEST(CameraProtocol, RateCapDisableTimeoutLossAndUnsupportedRequests) {
    Config c; c.selection_ttl_s=1; c.selection_stale_s=.4;
    CameraProtocol camera(c); auto now=std::chrono::steady_clock::now();
    auto point=camera.handle(command(MAV_CMD_CAMERA_TRACK_POINT,.5,.5,.1),now);
    camera.update_selection(point.event->generation,std::array<float,4>{.4f,.4f,.6f,.6f},100,100,now);
    camera.handle(command(MAV_CMD_SET_MESSAGE_INTERVAL,275,1),now);
    ASSERT_TRUE(camera.poll(now)); EXPECT_FALSE(camera.poll(now+499ms));
    camera.handle(command(MAV_CMD_SET_MESSAGE_INTERVAL,275,1),now+100ms);
    EXPECT_FALSE(camera.poll(now+100ms)); // Repeated requests cannot bypass rate cap.
    auto packet=camera.poll(now+500ms); ASSERT_TRUE(packet);
    mavlink_camera_tracking_image_status_t s{}; auto m=decode(*packet);
    mavlink_msg_camera_tracking_image_status_decode(&m,&s); EXPECT_EQ(s.tracking_mode,CAMERA_TRACKING_MODE_NONE);
    camera.update_selection(point.event->generation,std::array<float,4>{.4f,.4f,.6f,.6f},100,100,now+1100ms);
    packet=camera.poll(now+1100ms); ASSERT_TRUE(packet); m=decode(*packet);
    mavlink_msg_camera_tracking_image_status_decode(&m,&s); EXPECT_EQ(s.tracking_mode,CAMERA_TRACKING_MODE_NONE);
    auto fresh=camera.handle(command(MAV_CMD_CAMERA_TRACK_POINT,.5,.5,.1),now+1200ms);
    camera.update_selection(fresh.event->generation,std::nullopt,100,100,now+1200ms);
    auto one_shot=camera.handle(command(MAV_CMD_REQUEST_MESSAGE,275),now+1200ms);
    ASSERT_EQ(one_shot.packets.size(),2u); m=decode(one_shot.packets[1]);
    mavlink_msg_camera_tracking_image_status_decode(&m,&s); EXPECT_EQ(s.tracking_mode,CAMERA_TRACKING_MODE_NONE);
    expect_ack(camera.handle(command(MAV_CMD_SET_MESSAGE_INTERVAL,275,-1),now),511,MAV_RESULT_ACCEPTED);
    EXPECT_FALSE(camera.poll(now+10s));
    expect_ack(camera.handle(command(MAV_CMD_SET_MESSAGE_INTERVAL,275,-2)),511,MAV_RESULT_DENIED);
    for(float invalid:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()})
        expect_ack(camera.handle(command(MAV_CMD_SET_MESSAGE_INTERVAL,275,invalid)),511,MAV_RESULT_DENIED);
    Config faster_config; faster_config.status_max_hz=4;
    CameraProtocol faster(faster_config);
    expect_ack(faster.handle(command(MAV_CMD_SET_MESSAGE_INTERVAL,275,0),now),511,MAV_RESULT_ACCEPTED);
    ASSERT_TRUE(faster.poll(now)); EXPECT_FALSE(faster.poll(now+249ms));
    EXPECT_TRUE(faster.poll(now+250ms)); // Zero restores the configured default.
    for(int id:{269,999}) expect_ack(camera.handle(command(MAV_CMD_REQUEST_MESSAGE,id)),512,MAV_RESULT_UNSUPPORTED);
    expect_ack(camera.handle(command(MAV_CMD_CAMERA_TRACK_RECTANGLE)),MAV_CMD_CAMERA_TRACK_RECTANGLE,MAV_RESULT_UNSUPPORTED);
}

TEST(CameraProtocol, ReconnectClearsOldSelectionAndKeepsRequestedIdleFeedback) {
    CameraProtocol camera(Config{}); auto now=std::chrono::steady_clock::now();
    auto point=camera.handle(command(MAV_CMD_CAMERA_TRACK_POINT,.5,.5,.1),now);
    camera.handle(command(MAV_CMD_SET_MESSAGE_INTERVAL,275,500000),now);
    camera.update_selection(point.event->generation,std::array<float,4>{.4f,.4f,.6f,.6f},100,100,now);
    ASSERT_TRUE(camera.poll(now)); camera.reset();
    camera.update_selection(point.event->generation,std::array<float,4>{.4f,.4f,.6f,.6f},100,100,now+100ms);
    auto packet=camera.poll(now+500ms); ASSERT_TRUE(packet);
    auto message=decode(*packet); mavlink_camera_tracking_image_status_t status{};
    mavlink_msg_camera_tracking_image_status_decode(&message,&status);
    EXPECT_EQ(status.tracking_mode,CAMERA_TRACKING_MODE_NONE);
    EXPECT_EQ(status.tracking_status,CAMERA_TRACKING_STATUS_FLAGS_IDLE);
    camera.handle(command(MAV_CMD_SET_MESSAGE_INTERVAL,275,-1),now); camera.reset();
    EXPECT_FALSE(camera.poll(now+5s));
}
