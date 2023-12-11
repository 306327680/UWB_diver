//
// Created by echo on 23-4-1.
//

#include "../include/main.h"
#include "ros/ros.h"
#include <serial/serial.h>
#include <unistd.h>
#include "std_msgs/Int32.h"
#include "nav_msgs/Odometry.h"

serial::Serial ser;
std::string port = "/dev/ttyUSB0";
int baudrate = 115200;
std::vector<std::string> split(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;//扩展字符串以方便操作
    int size = str.size();
    for (int i = 0; i < size; i++)
    {
        pos = str.find(pattern, i);
        if (pos < size)
        {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}
int main(int argc, char** argv)
{
    ros::init(argc, argv, "UWB_node");
    ros::NodeHandle nh("~");

    std::vector<std::string> split_result_sub;
    std::vector<std::string> split_result_sub_sub;

    try
    { // 设置串口属性，并打开串口
        ser.setPort(port);
        ser.setBaudrate(baudrate);
        serial::Timeout to = serial::Timeout::simpleTimeout(1000);
        ser.setTimeout(to);
        ser.open();
    }
    catch (serial::IOException& e)
    {
        ROS_ERROR_STREAM("Unable to open port ");
        return -1;
    }
    // 检测串口是否已经打开，并给出提示信息
    if(ser.isOpen()){
        ROS_INFO_STREAM("Serial Port initialized");
    }
    else{
        return -1;
    }
    nav_msgs::Odometry odom_to_pub;
    odom_to_pub.header.frame_id = "uwb";
    odom_to_pub.header.seq = 0;
    odom_to_pub.header.stamp = ros::Time::now();
    ros::Publisher encoder_pub = nh.advertise<nav_msgs::Odometry>("uwb_raw", 1);
    ros::Rate r(10); // 10 hz

    while(ros::ok()){
        std::string message;
        size_t n = ser.available();
        if(n!=0){
            uint8_t buffer[1024];
            n = ser.read(buffer, n);
            for(int i=0; i<n; i++){
                message.push_back(buffer[i]);
            }
        }
        std::vector<std::string> split_result;
        split_result = split(message,"\n");
        for (int i = 0; i <split_result.size() ; ++i) {
            std::vector<double> data_raw;
            std::vector<std::string> data_str;
            //have 7 result
            split_result_sub = split(split_result[i]," ");
            bool final = false;
            if(split_result_sub.size() == 13){
                for (int j = 0; j < split_result_sub.size(); ++j) {
                    char *ptr;
                    data_raw.push_back(std::strtod(split_result_sub[j].c_str(),&ptr));
                    data_str.push_back(split_result_sub[j].c_str());
                    final = true;
                    std::cout<<" "<<i<<" " <<split_result[i]<<std::endl;
                }
            } /*else{

            }*/
            if(final){
                int d_a = -1;
                int d_b = -1;
                int d_c = -1;
                if(data_str[2]!="ffffffff"){
                    d_a = std::stoi(data_str[2],nullptr,16);
                } else{
                     d_a = -1;
                }
                if(data_str[3]!="ffffffff"){
                    d_b = std::stoi(data_str[3],nullptr,16);
                } else{
                    d_b = -1;
                }
                if(data_str[4]!="ffffffff"){
                    d_c = std::stoi(data_str[4],nullptr,16);
                } else{
                    d_c = -1;
                }

                int i_a = std::stoi(data_str[10],nullptr,16);
                int i_b = std::stoi(data_str[11],nullptr,16);
                int i_c = std::stoi(data_str[12],nullptr,16);
                odom_to_pub.header.seq = odom_to_pub.header.seq + 1;
                odom_to_pub.header.stamp = ros::Time::now();
                odom_to_pub.pose.pose.position.x = d_a/1000.0;
                odom_to_pub.pose.pose.position.y = d_b/1000.0;
                odom_to_pub.pose.pose.position.z = d_c/1000.0;

                odom_to_pub.twist.twist.linear.x = i_a;
                odom_to_pub.twist.twist.linear.y = i_b;
                odom_to_pub.twist.twist.linear.z = i_c;
                std::cout<<" out: "<<" " <<i_a<<" " <<i_b<<" " <<i_b<<std::endl;

                encoder_pub.publish(odom_to_pub);
                continue;
            }
        }
        message = "";
        split_result.clear();

        r.sleep();
    }

    //关闭串口
    ser.close();

    return 0;

}
