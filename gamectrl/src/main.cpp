#include <ros/ros.h>
#include "gctrl.hpp"

boost::asio::io_service udp_service;

int main(int argc, char **argv)
{
    
    ros::init(argc, argv, "communication");
    ros::NodeHandle node;
    bool params_seted = false; 
    GameCtrl gc(node, udp_service); 
    
    gc.start();
    boost::asio::io_service::work work(udp_service);udp_service.run();
    gc.stop();

    return 0;
}