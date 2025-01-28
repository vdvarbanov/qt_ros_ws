#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include <memory>
#include <stdio.h>
#include <iterator>
#include <algorithm>
#include <vector>

#include "pcl/memory.h"
#include "pcl-1.12/pcl/io/pcd_io.h"
#include "pcl-1.12/pcl/point_types.h"
#include "pcl_conversions/pcl_conversions.h"

#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/msg/point_cloud.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "std_msgs/msg/header.hpp"
#include </opt/ros/humble/include/sensor_msgs/sensor_msgs/point_cloud2_iterator.hpp>
#include "sensor_msgs/msg/point_field.hpp"
#include "/opt/ros/humble/include/image_geometry/image_geometry/pinhole_camera_model.h"
#include "/opt/ros/humble/include/image_geometry/image_geometry/visibility_control.hpp"
#include <boost/make_shared.hpp>
#include <pcl/common/transforms.h>
#include <pcl/visualization/pcl_visualizer.h>

using std::placeholders::_1;
using std::placeholders::_2;




class MSGToPCL2 : public rclcpp::Node
{
//TODO: check NodeOptions
    public:
    MSGToPCL2(std::string _node_name, std::string _sub_topic_name, std::string _pub_topic_name)
        :Node(_node_name), sub_topic_name{_sub_topic_name}, pub_topic_name{_pub_topic_name}
        {
            //cam_model.fromCameraInfo();
            transform_mat.translation()<< 0.0, base, 0.0;        //0.1 is base between laser and camera
              // The same rotation matrix as before; theta radians around Z axis
              //transform_2.rotate (Eigen::AngleAxisf (theta, Eigen::Vector3f::UnitZ()));
            
            subsciption_cam_info = this-> create_subscription<sensor_msgs::msg::CameraInfo>
            (   "camera_sensor/camera_info",
                10,
                std::bind(&MSGToPCL2::set_camera_model, this,_1)
               
            );
            
            
             RCLCPP_INFO(this->get_logger(), "sub_created");
            //TODO use service
            subsciption_lidar = this->create_subscription<sensor_msgs::msg::PointCloud2>
                (sub_topic_name,
                 10,
                 std::bind(&MSGToPCL2::on_receive,this,_1)           
                );
            
            publisher = this->create_publisher<sensor_msgs::msg::PointCloud2>
                (pub_topic_name,
                 10);
            //F = (width_pix/2)/(tan(deg2rad(hfov/2))) hfov = 1.02974 = 59deg
         
        }

    private:
    
    std::string topic_name;
    std::string sub_topic_name;
    std::string pub_topic_name;


    bool is_cam_set = false;
    //alpha -> angle of ray to mounted surface
    //std::vector<double> cotan_alpha_x {  (double) (cosf(1.3507963267949)/sinf(1.3507963267949))};
    std::vector<double> cotan_alpha_x {  (double) (cosf(1.3507963267949)/sinf(1.3507963267949))};
    std::vector<double> cotan_alpha_y {1, cotan_alpha_x[0]} ; //1->0 but want to try with infinity
    double base = 0.1;

    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr subsciption_cam_info;
    rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr subsciption_lidar;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr publisher;
    sensor_msgs::msg::PointCloud2 cloud_out_msg;

    pcl::PCLPointCloud2::Ptr cloud_in {new pcl::PCLPointCloud2()};
    pcl::PCLPointCloud2::Ptr cloud_out {new pcl::PCLPointCloud2()};
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr lidar_in {new pcl::PointCloud<pcl::PointXYZRGB> ()};
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr transformed_cloud {new pcl::PointCloud<pcl::PointXYZRGB> ()};
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr tmp_PCL {new pcl::PointCloud<pcl::PointXYZRGB> ()};
    Eigen::Affine3f transform_mat = Eigen::Affine3f::Identity();
    cv::Point3d point_tmp;
    size_t num_rays = cotan_alpha_x.size() * cotan_alpha_y.size();
    std::vector<cv::Point3d> points_tmp {num_rays};
    cv::Point2d uv_rect_tmp;
    cv::Point3d ray;
    image_geometry::PinholeCameraModel cam_model;
    /*fx=F*numpix_x
    F = (width_pix/2)/(tan(deg2rad(hfov/2)))*/

    void set_camera_model(sensor_msgs::msg::CameraInfo::SharedPtr cam_info)
    {
        if(!is_cam_set)
        {
           is_cam_set = cam_model.fromCameraInfo(cam_info);
        
           RCLCPP_INFO(this->get_logger(), "cx: %f", cam_model.cx());
           RCLCPP_INFO(this->get_logger(), "cy: %f", cam_model.cy());
           RCLCPP_INFO(this->get_logger(), "fx: %f", cam_model.fx());
           RCLCPP_INFO(this->get_logger(), "fy: %f", cam_model.fy());
        
        }
    }

    void on_receive(sensor_msgs::msg::PointCloud2::SharedPtr _msg)
    {
        //TODO: check for multiple points
        pcl::fromROSMsg(*_msg, *lidar_in);
        int cloud_height = _msg->height;
        int cloud_width = _msg->width;

        RCLCPP_INFO(this->get_logger(), "h: %i  w: %i", cloud_height, cloud_width);
        //transform lidar_in to cam xyz
        for (int i = 0; i < num_rays; ++i)
        {
            

           RCLCPP_INFO(this->get_logger(), "cx: %f", cam_model.cx());
           RCLCPP_INFO(this->get_logger(), "cy: %f", cam_model.cy());
           RCLCPP_INFO(this->get_logger(), "fx: %f", cam_model.fx());
           RCLCPP_INFO(this->get_logger(), "fy: %f", cam_model.fy());


            RCLCPP_INFO(this->get_logger(), "i: %i", i);

            RCLCPP_INFO(this->get_logger(), "x_laser_coors: %f", lidar_in->points[i].x);
            RCLCPP_INFO(this->get_logger(), "y_laser_coors: %f", lidar_in->points[i].y);
            RCLCPP_INFO(this->get_logger(), "z_laser_coors: %f", lidar_in->points[i].z);
            //transform laser coordinates to camera
            // ->translation and then rotation
            //TODO: translate to global coorinates, later
            pcl::transformPointCloud(*lidar_in, *transformed_cloud, transform_mat);
            point_tmp.x=(double)transformed_cloud->points[i].y *-1.0;
            point_tmp.x = (double)transformed_cloud->points[i].y *-1.0;
            point_tmp.y = (double)transformed_cloud->points[i].z;  //check if inversion is needed
            point_tmp.z = (double)transformed_cloud->points[i].x;

            RCLCPP_INFO(this->get_logger(), "point_tmp.x: %f",point_tmp.x);
            RCLCPP_INFO(this->get_logger(), "point_tmp.y: %f",point_tmp.y);
            RCLCPP_INFO(this->get_logger(), "point_tmp.z: %f",point_tmp.z);


            RCLCPP_INFO(this->get_logger(), "x transformed_laser_to_cam: %f", transformed_cloud->points[i].x);
            RCLCPP_INFO(this->get_logger(), "y transformed_laser_to_cam: %f", transformed_cloud->points[i].y);
            RCLCPP_INFO(this->get_logger(), "z transformed_laser_to_cam: %f", transformed_cloud->points[i].z);
            
            /* not sur if to go double cx = 960.5; double cy = 540.5; double fx = 1696.802685832259;*/
            
            uv_rect_tmp = cam_model.project3dToPixel(point_tmp);
            
            RCLCPP_INFO(this->get_logger(), "u->laser_projection to_X_cam_pixel: %f",(float)uv_rect_tmp.x);
            RCLCPP_INFO(this->get_logger(), "v->laser_projection_Y_cam_pixel: %f",(float)uv_rect_tmp.y);
            RCLCPP_INFO(this->get_logger(), "u -cx->diff_from_center_X: %f",((float)uv_rect_tmp.x-(float)cam_model.cx()));
            RCLCPP_INFO(this->get_logger(), "v -cy->diff_from_center_Y: %f",((float)uv_rect_tmp.y-(float)cam_model.cy()));
            RCLCPP_INFO(this->get_logger(), "cotan_alpha_x->laser_angle_X_axis: %f", cotan_alpha_x[0]);
            RCLCPP_INFO(this->get_logger(), "cotan_alpha_y_i->laser_angle_Y_axis: %f", cotan_alpha_y[i]);
            
            ray = cam_model.projectPixelTo3dRay(uv_rect_tmp);
            
            /*
            RCLCPP_INFO(this->get_logger(), "ray_x: %f",(float)ray.x);
            RCLCPP_INFO(this->get_logger(), "ray_y: %f",(float)ray.y);
            RCLCPP_INFO(this->get_logger(), "ray_z: %f",(float)ray.z);
            */
           /*
            float x_1 = (base*(uv_rect_tmp.x-cam_model.cx()))/(cam_model.fx() * cotan_alpha_x-(uv_rect_tmp.x-cam_model.cx()));
            //TODO: fix this!!!
            float y_1 = (base*(uv_rect_tmp.y-cam_model.cy()))/(cam_model.fy()*cotan_alpha_y[i]-(uv_rect_tmp.x-cam_model.cx()));
            float z_1 = (base*cam_model.fx())/(cam_model.fx()*cotan_alpha_x-(uv_rect_tmp.x-cam_model.cx()));
            */
            pcl::PointXYZRGB _tmp_point;
            _tmp_point.x = (float) (base*(uv_rect_tmp.x-cam_model.cx()))/(cam_model.fx() *cotan_alpha_x[0]-(uv_rect_tmp.x-cam_model.cx()));
            _tmp_point.y = (float) (base*(uv_rect_tmp.y-cam_model.cy()))/(cam_model.fy()*cotan_alpha_y[i]-(uv_rect_tmp.x-cam_model.cx()));
            _tmp_point.z = (float) (base*cam_model.fx())/(cam_model.fx()*cotan_alpha_x[0]-(uv_rect_tmp.x-cam_model.cx()));
            _tmp_point.rgb = 12.f;
            tmp_PCL->push_back(_tmp_point);
            
            
            RCLCPP_INFO(this->get_logger(), "X calculated: %f", _tmp_point.x);
            RCLCPP_INFO(this->get_logger(), "Y calculated: %f", _tmp_point.y);
            RCLCPP_INFO(this->get_logger(), "Z calculated: %f", _tmp_point.z);
            
        }
                // publisher->publish(tmp_pcl_msg);
       // publisher->publish();
        cloud_out_msg.header = std_msgs::msg::Header();
        cloud_out_msg.header.stamp = this->now();
        pcl::toROSMsg(*tmp_PCL, cloud_out_msg );
        publisher->publish(cloud_out_msg);
    }  
};

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MSGToPCL2>
        ("msg_to_pcl2", 
        "ray/laser/out",
        "custom_msg_pcl");
    rclcpp::spin(node);
    rclcpp::shutdown();

}

