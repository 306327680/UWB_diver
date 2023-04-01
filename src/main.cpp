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
std::string port = "/dev/ttyACM0";
int baudrate = 230400;
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
    ros::Rate r(500); // 10 hz

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
            //have 7 result
            split_result_sub = split(split_result[i],",");
            bool final = false;
            if(split_result_sub.size() == 7){
                for (int j = 0; j < split_result_sub.size(); ++j) {
                    char *ptr;
                    data_raw.push_back(std::strtod(split_result_sub[j].c_str(),&ptr));
                    final = true;
                }
            } else{
                std::cout <<split_result_sub.size()<<" : "<<split_result[i]<<std::endl;
            }
            if(final){
                for (int i = 0; i < 6; ++i) {
//                std::cout<<" x "<<data_raw[i];
                }
                odom_to_pub.header.seq = odom_to_pub.header.seq + 1;
                odom_to_pub.header.stamp = ros::Time::now();
                odom_to_pub.pose.pose.position.x = data_raw[3]/100;
                odom_to_pub.pose.pose.position.y = data_raw[4]/100;
                odom_to_pub.pose.pose.position.z = data_raw[5]/100;
                odom_to_pub.pose.covariance[0] = data_raw[2]/100;
                odom_to_pub.pose.covariance[7] = data_raw[2]/100;
                odom_to_pub.pose.covariance[14] = data_raw[2]/100;
                odom_to_pub.twist.twist.linear.x = data_raw[0]/100;
                odom_to_pub.twist.twist.linear.y = data_raw[1]/100;
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