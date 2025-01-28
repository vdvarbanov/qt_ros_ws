cmake_minimum_required(VERSION 3.8)
project(neptune_2_description)


# find dependencies
find_package(ament_cmake REQUIRED)

install( DIRECTORY urdf launch rviz meshes
         DESTINATION share/${PROJECT_NAME}/
      )

ament_package()
