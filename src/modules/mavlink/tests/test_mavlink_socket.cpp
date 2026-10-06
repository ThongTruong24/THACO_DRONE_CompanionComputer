#include <gtest/gtest.h>
#include <rclcpp/rclcpp.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mavlink_main.h"

class MavlinkSocketTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		if (!rclcpp::ok()) {
			rclcpp::init(0, nullptr);
		}
	}
};

TEST_F(MavlinkSocketTest, BindsSpecificallyToLoopback)
{
	static int port_offset = 0;
	int local_port = 26601 + (port_offset++ * 2);
	int router_port = local_port - 1;

	auto mavlink_node = std::make_shared<Mavlink>(
				    "test_mavlink_socket_" + std::to_string(local_port),
				    "127.0.0.1", router_port, local_port);

	int fd = mavlink_node->get_socket_fd();
	ASSERT_GE(fd, 0);

	struct sockaddr_in bound_addr {};
	socklen_t len = sizeof(bound_addr);
	int ret = getsockname(fd, reinterpret_cast<struct sockaddr *>(&bound_addr), &len);
	ASSERT_EQ(ret, 0);

	EXPECT_EQ(bound_addr.sin_family, AF_INET);
	EXPECT_EQ(ntohs(bound_addr.sin_port), local_port);

	char ip[INET_ADDRSTRLEN] = {};
	inet_ntop(AF_INET, &bound_addr.sin_addr, ip, sizeof(ip));
	EXPECT_STREQ(ip, "127.0.0.1");
}
