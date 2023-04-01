cmake_minimum_required(VERSION 3.0.2)
project(UWB_diver)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
        serial
        nav_msgs
)

catkin_package(

)

add_executable(${PROJECT_NAME} src/main.cpp)
add_dependencies(${PROJECT_NAME} ${catkin_EXPORTED_TARGETS} )
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} )
include_directories(
        # include
        ${catkin_INCLUDE_DIRS}
)

