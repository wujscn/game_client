#include "gctrl.hpp"
#include "common/GcInfo.h"

using namespace std;
using boost::asio::ip::udp;


GameCtrl::GameCtrl(ros::NodeHandle &node, boost::asio::io_service &svc)
    :recv_socket_ (svc, udp::endpoint(udp::v4(), GAMECONTROLLER_DATA_PORT)),
    // recv_point_(ip::address::from_string("127.0.0.1"),GAMECONTROLLER_DATA_PORT),
    send_socket_(svc, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)),
    send_point_(boost::asio::ip::address_v4::broadcast(), GAMECONTROLLER_RETURN_PORT)
{
    gcPublisher = node.advertise<common::GcInfo>("/sensor/gctrl", 1);
    try
    {
        ros::param::get("id", playerId);
        ros::param::get("team_number", teamId);
    }
    catch (ros::InvalidNameException &e)
    {
        ROS_ERROR("%s", e.what());
    }
    teamIdx = -1;
    retData_.player = 1;
    retData_.message = GAMECONTROLLER_RETURN_MSG_ALIVE;
    retData_.team = 4;
}

bool GameCtrl::start()
{
    this->receive();
    return true;
}

void GameCtrl::receive()
{
    if(!ros::ok()) return;
    recv_socket_.async_receive_from(boost::asio::buffer((char *)&data_, gc_data_size), recv_point_,
                                    [this](boost::system::error_code ec, std::size_t bytes_recvd)
    {
        if (!ec && bytes_recvd > 0)
        {
            string recv_header;
            recv_header.append(data_.header, sizeof(RoboCupGameControlData::header));
            if (recv_header == GAMECONTROLLER_STRUCT_HEADER)
            {
                printf("teamid:%d\n",data_.teams[0].teamNumber);
                printf("data_.state:%d\n",data_.state);
                printf("data_.firstHalf:%d\n",data_.firstHalf);
                printf("data_.secondaryState:%d\n",data_.secondaryState);
                printf("data_.secsRemaining:%d\n",data_.secsRemaining);
                printf("data_.secondaryTime:%d\n",data_.secondaryTime);
                //teamIdx = data_.teams[0].teamNumber == teamId ? 0 : 1;
                printf("data_.kickOffTeam:%d\n",data_.kickOffTeam);
printf("data_.teams[teamIdx].players[playerId-1].penalty:%d\n",data_.teams[teamIdx].players[playerId-1].penalty);
                //}
                common::GcInfo info;
                info.state = data_.state;
                
                // printf("data_.state222:%d\n",data_.state);
                cout<<"gc.info.state:"<<info.state<<endl;
                info.firstHalf = data_.firstHalf;
                info.kickOffTeam= data_.kickOffTeam;
                info.secondaryState= data_.secondaryState;
                info.secsRemaining = data_.secsRemaining;
                info.secondaryTime = data_.secondaryTime;
                info.penalty = data_.teams[teamIdx].players[playerId-1].penalty;
                
                cout<<"gc.info.firstHalf:"<<info.firstHalf<<endl;
                gcPublisher.publish(info); 
                 
                send_socket_.async_send_to(boost::asio::buffer((char *)(&retData_), 
                    sizeof(RoboCupGameControlReturnData)), send_point_,
                    [this](boost::system::error_code ec, std::size_t bytes_sent) {});
            }
        }
         receive();
    });
}

void GameCtrl::stop()
{
    recv_socket_.cancel();
    recv_socket_.close();
}
