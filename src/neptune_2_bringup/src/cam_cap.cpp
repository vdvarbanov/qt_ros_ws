#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include "std_msgs/msg/header.hpp"
#include <chrono>
#include "/opt/ros/humble/include/cv_bridge/cv_bridge/cv_bridge.h" // cv_bridge converts between ROS 2 image messages and OpenCV image representations.
#include "/opt/ros/humble/include/cv_bridge/cv_bridge/cv_bridge_export.h"
#include <image_transport/image_transport.hpp> // Using image_transport allows us to publish and subscribe to compressed image streams in ROS2
#include <opencv2/opencv.hpp> // We include everything about OpenCV as we don't care much about compilation time at the moment.
#include <image_geometry/pinhole_camera_model.h>

using std::placeholders::_1;
using std::placeholders::_2;

//TODO: rename back to processs
class CamCap : public rclcpp::Node
{
    public:
    CamCap(std::string _node_name, std::string _topic_name) 
        : Node(_node_name)
    {
        
            subsciption_cam_info = this-> create_subscription<sensor_msgs::msg::CameraInfo>
                ("camera_sensor/camera_info",
                 10,
                 std::bind(&CamCap::set_camera_model, this, _1)
                );
        
            subscription_image = this->create_subscription<sensor_msgs::msg::Image>
                (_topic_name,
                 10,
                 std::bind(&CamCap::receive_img, this, _1)
                ); 
    }


    void set_camera_model(sensor_msgs::msg::CameraInfo::SharedPtr cam_info)
    {
        if(!is_cam_set)
        {
           is_cam_set = cam_model.fromCameraInfo(cam_info);
        
        }
    }


    void receive_img(sensor_msgs::msg::Image::SharedPtr _msg)
    {
        ros_image = _msg;
        cv_bridge::CvImagePtr cv_img_ptr = cv_bridge::toCvCopy(ros_image,
                                                               sensor_msgs::image_encodings::BGR8);
        cv_image = cv_img_ptr->image;
        cv::imshow(win_title, cv_image);
        cv::waitKey(5);  
    }

    private:
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_image;
    cv::Mat cv_image{cam_model.fullResolution(), CV_8UC3 }; //cam_model.fullResolution()
    sensor_msgs::msg::Image::SharedPtr ros_image;
    cv::String win_title = "received_image";
    
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr subsciption_cam_info;
    image_geometry::PinholeCameraModel cam_model;
    bool is_cam_set = false;
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CamCap>("cam_process",
                                             "/camera_sensor/image_raw");
    rclcpp::spin(node);
    rclcpp::shutdown();
}
