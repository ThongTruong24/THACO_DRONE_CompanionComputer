#include <gtest/gtest.h>
#include "bridge.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <condition_variable>

using namespace vision;
using namespace std::chrono_literals;
TEST(TcpTransport, FragmentationCoalescingReconnectAndShutdown) {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(server, 0);
    sockaddr_in address{}; address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ASSERT_EQ(bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)), 0);
    ASSERT_EQ(listen(server, 2), 0);
    socklen_t length = sizeof(address);
    ASSERT_EQ(getsockname(server, reinterpret_cast<sockaddr*>(&address), &length), 0);
    std::mutex mutex; std::condition_variable cv;
    std::vector<mavlink_message_t> received;
    Config config("127.0.0.1", ntohs(address.sin_port), 1, 192);
    config.reconnect_min_ms = 20; config.reconnect_max_ms = 100;
    Transport transport(config, [&](const mavlink_message_t& m) {
        std::lock_guard lock(mutex); received.push_back(m); cv.notify_all();
    });
    transport.start();
    auto accept_client = [&]() {
        pollfd p{server, POLLIN, 0};
        if (poll(&p, 1, 3000) <= 0) return -1;
        return accept(server, nullptr, nullptr);
    };
    int peer = accept_client(); ASSERT_GE(peer, 0);
    Codec remote(255, 190), local(1, 192); Decoder decoder;
    // Both identities must be present on this single accepted TCP connection.
    pollfd p{peer, POLLIN, 0}; ASSERT_GT(poll(&p, 1, 2000), 0);
    uint8_t data[4096]; auto count = recv(peer, data, sizeof(data), 0);
    auto frames = decoder.feed(Bytes(data, data + count));
    ASSERT_FALSE(frames.empty()); EXPECT_EQ(frames[0].msgid, 0u);
    EXPECT_EQ(frames[0].sysid, 1); EXPECT_EQ(frames[0].compid, 192);
    bool alias_seen=false;
    for(const auto& m:frames)if(m.msgid==0 && m.sysid==1 && m.compid==105)alias_seen=true;
    auto heartbeat_deadline=std::chrono::steady_clock::now()+2s;
    while(!alias_seen && std::chrono::steady_clock::now()<heartbeat_deadline) {
        if(poll(&p,1,100)<=0)continue;
        count=recv(peer,data,sizeof(data),0); ASSERT_GT(count,0);
        for(const auto& m:decoder.feed(Bytes(data,data+count)))
            if(m.msgid==0 && m.sysid==1 && m.compid==105)alias_seen=true;
    }
    EXPECT_TRUE(alias_seen);
    auto packet = remote.config_get(1234, "telemetry.fc.baudrate");
    ASSERT_EQ(send(peer, packet.data(), 5, MSG_NOSIGNAL), 5);
    { std::unique_lock lock(mutex); EXPECT_FALSE(cv.wait_for(lock, 50ms, [&]{return !received.empty();})); }
    ASSERT_EQ(send(peer, packet.data()+5, packet.size()-5, MSG_NOSIGNAL), static_cast<ssize_t>(packet.size()-5));
    auto two = remote.heartbeat(); auto second = remote.config_get(5678, "vision.model");
    two.insert(two.end(), second.begin(), second.end());
    ASSERT_EQ(send(peer, two.data(), two.size(), MSG_NOSIGNAL), static_cast<ssize_t>(two.size()));
    { std::unique_lock lock(mutex); ASSERT_TRUE(cv.wait_for(lock, 2s, [&]{return received.size() >= 3;}));
      EXPECT_EQ(received[0].msgid, 42102u); EXPECT_EQ(received[0].sysid, 255); EXPECT_EQ(received[0].compid, 190);
      mavlink_cc_config_get_t payload{}; mavlink_msg_cc_config_get_decode(&received[0], &payload);
      EXPECT_EQ(payload.request_id, 1234u); EXPECT_EQ(std::string(payload.key), "telemetry.fc.baudrate");
      EXPECT_EQ(received[1].msgid, 0u); EXPECT_EQ(received[2].msgid, 42102u); }
    ASSERT_TRUE(transport.send(local.config_get(0x5a170001, "telemetry.fc.baudrate")));
    bool found = false; auto until = std::chrono::steady_clock::now()+2s;
    while (!found && std::chrono::steady_clock::now()<until) {
        if (poll(&p,1,100)<=0) continue;
        count=recv(peer,data,sizeof(data),0); ASSERT_GT(count,0);
        for (const auto& m:decoder.feed(Bytes(data,data+count))) if (m.msgid==42102) {
            mavlink_cc_config_get_t payload{};mavlink_msg_cc_config_get_decode(&m,&payload);
            EXPECT_EQ(payload.request_id,0x5a170001u);EXPECT_EQ(m.sysid,1);EXPECT_EQ(m.compid,192);found=true;
        }
    }
    EXPECT_TRUE(found);
    // Disconnect with an unfinished frame; it must not pollute the next connection.
    send(peer, packet.data(), 5, MSG_NOSIGNAL); close(peer);
    peer = accept_client(); ASSERT_GE(peer, 0);
    auto heartbeat = remote.heartbeat(); send(peer, heartbeat.data(), heartbeat.size(), MSG_NOSIGNAL);
    { std::unique_lock lock(mutex); ASSERT_TRUE(cv.wait_for(lock,2s,[&]{return received.size()>=4;}));
      EXPECT_EQ(received.back().msgid,0u); }
    auto started=std::chrono::steady_clock::now();
    transport.stop(); EXPECT_LT(std::chrono::steady_clock::now()-started,500ms);
    transport.start(); close(peer); peer=accept_client(); ASSERT_GE(peer,0); transport.stop();
    close(peer); close(server);
}
TEST(TcpTransport, UnreachableEndpointDoesNotBlockProducer) {
    int reservation=socket(AF_INET,SOCK_STREAM,0);sockaddr_in a{};a.sin_family=AF_INET;
    a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);ASSERT_EQ(bind(reservation,(sockaddr*)&a,sizeof(a)),0);
    socklen_t n=sizeof(a);getsockname(reservation,(sockaddr*)&a,&n);
    // A bound, non-listening socket refuses connections without relying on a fixed port.
    Transport transport(Config("127.0.0.1",ntohs(a.sin_port),1,192),[](const auto&){});
    transport.start();Codec codec(1,192);auto start=std::chrono::steady_clock::now();
    for(int i=0;i<1000;i++)transport.send(codec.heartbeat(),true);
    EXPECT_LT(std::chrono::steady_clock::now()-start,200ms);
    EXPECT_LE(transport.pending(),32u);transport.stop();close(reservation);
}
