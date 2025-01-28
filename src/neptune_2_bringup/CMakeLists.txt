cmake_minimum_required(VERSION 3.8)
project(neptune_2_bringup)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(image_geometry REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)

#find_package(PCL REQUIRED) 
set(PCL_ROOT  )
find_package(PCL REQUIRED) 
find_package(Eigen3 3.4 REQUIRED NO_MODULE)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
add_definitions(${OpenCV_DEFINITIONS})


include_directories(${PCL_INCLUDE_DIRS} include)
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_executable(msg_to_pcl2  src/msg_to_pcl2.cpp)

target_link_libraries( msg_to_pcl2
  ${Boost_SYSTEM_LIBRARY}
  ${PCL_LIBRARIES}
  ${PCL_LIBRARIES} Eigen3::Eigen)

ament_target_dependencies(
  msg_to_pcl2
  rclcpp std_msgs 
  sensor_msgs
  pcl_conversions
  image_geometry
  PCL
)

add_executable(cam_cap src/cam_cap.cpp)
target_link_libraries( cam_cap
  ${Boost_SYSTEM_LIBRARY}
  ${OpenCV_INCLUDE_DIRS}
  ${OpenCV_LIBRARY_DIRS}
  
  )
ament_target_dependencies(
  cam_cap
  rclcpp std_msgs 
  sensor_msgs
  image_geometry
  cv_bridge
  image_transport
  OpenCV
)





# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
install( 
  DIRECTORY launch rviz worlds
  DESTINATION  share/${PROJECT_NAME}/
)

install(TARGETS 
  msg_to_pcl2
  cam_cap
  DESTINATION lib/${PROJECT_NAME}
  )


ament_package()
