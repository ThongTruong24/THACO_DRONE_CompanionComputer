#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>
#include <rclcpp/rclcpp.hpp>
#include "mavlink_router/mavlink_router_node.hpp"

namespace fs = std::filesystem;
using namespace cc;

class MavlinkRouterNodeTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		rclcpp::init(0, nullptr);
	}

	static void TearDownTestSuite()
	{
		rclcpp::shutdown();
	}

	void SetUp() override
	{
		test_dir_ = fs::temp_directory_path() / ("mlr_node_test_" + std::to_string(getpid()));
		fs::create_directories(test_dir_);

		mock_routerd_ = test_dir_ / "mock_routerd.sh";
		{
			std::ofstream out(mock_routerd_);
			out << "#!/bin/sh\nwhile true; do sleep 0.1; done\n";
		}
		chmod(mock_routerd_.c_str(), 0755);

		conf_path_ = test_dir_ / "mavlink-router.conf";
		active_links_ = test_dir_ / "active.json";
		default_links_ = test_dir_ / "default.json";

		{
			std::ofstream out(active_links_);
			out << R"({"links":[{"name":"Init","port":"/dev/ttyUSB0","baud":115200}]})";
		}

		router_ = std::make_shared<MavlinkRouter>(mock_routerd_.string(), conf_path_.string());
		router_->start(active_links_.string(), default_links_.string());

		node_ = std::make_shared<MavlinkRouterNode>(router_);
	}

	void TearDown() override
	{
		router_->stop();
		node_.reset();
		fs::remove_all(test_dir_);
	}

	fs::path test_dir_;
	fs::path mock_routerd_;
	fs::path conf_path_;
	fs::path active_links_;
	fs::path default_links_;
	std::shared_ptr<MavlinkRouter> router_;
	std::shared_ptr<MavlinkRouterNode> node_;
};

TEST_F(MavlinkRouterNodeTest, ApplyLinksService)
{
	auto client_node = std::make_shared<rclcpp::Node>("test_apply_client");
	auto client = client_node->create_client<cc_msgs::srv::ApplyLinks>("/cc/router/apply_links");

	ASSERT_TRUE(client->wait_for_service(std::chrono::seconds(2)));

	auto req = std::make_shared<cc_msgs::srv::ApplyLinks::Request>();
	cc_msgs::msg::Link item;
	item.name = "Telemetry";
	item.port = "/dev/ttyUSB1";
	item.baud = 57600;
	req->config.links.push_back(item);

	auto future = client->async_send_request(req);

	rclcpp::executors::SingleThreadedExecutor exec;
	exec.add_node(node_);
	exec.add_node(client_node);

	while (future.wait_for(std::chrono::milliseconds(50)) != std::future_status::ready) {
		exec.spin_some();
	}

	auto resp = future.get();
	EXPECT_TRUE(resp->success);
	EXPECT_EQ(resp->message, "Links applied");

	EXPECT_EQ(router_->get_current_config().links.at(0).name, "Telemetry");
	EXPECT_EQ(router_->get_current_config().links.at(0).baud, 57600u);
}

TEST_F(MavlinkRouterNodeTest, LinkConfigTopicUpdatesRouter)
{
	auto pub_node = std::make_shared<rclcpp::Node>("test_link_pub");
	auto pub = pub_node->create_publisher<cc_msgs::msg::LinkConfig>(
			   "/cc/link_config", rclcpp::QoS(1).transient_local().reliable());

	cc_msgs::msg::LinkConfig msg;
	cc_msgs::msg::Link item;
	item.name = "FromTopic";
	item.port = "/dev/ttyUSB2";
	item.baud = 921600;
	msg.links.push_back(item);

	pub->publish(msg);

	rclcpp::executors::SingleThreadedExecutor exec;
	exec.add_node(node_);
	exec.add_node(pub_node);

	auto start_time = std::chrono::steady_clock::now();

	while (std::chrono::duration_cast<std::chrono::milliseconds>(
		       std::chrono::steady_clock::now() - start_time).count() < 1000) {
		exec.spin_some();

		if (!router_->get_current_config().links.empty() &&
		    router_->get_current_config().links[0].name == "FromTopic") {
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	EXPECT_EQ(router_->get_current_config().links.at(0).name, "FromTopic");
}
