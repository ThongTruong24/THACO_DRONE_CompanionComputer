#include "router_supervisor.hpp"
#include <iostream>
int main() {
    drone::router::RouterConfig config;
    std::cout<<drone::router::RouterSupervisor::generate_config_content(config,false,false);
}
