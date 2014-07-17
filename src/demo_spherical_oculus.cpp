#include "image_handler.h"
#include "poly_omni.h"

#include <opencv2/core/core.hpp>

#include <iostream>
#include <string>

// //needed by demo from image files
//#include <boost/range/combine.hpp>
//#include <boost/tuple/tuple.hpp>


int main(int argc, char** argv){

    if (argc != 2) ROS_ERROR("Usage: demo_omni_oculus <rootpath_to_conf_files>");

    int sampling_ratio = atof(argv[1]);
    std::string conf_path = argv[2];

    ros::init(argc,argv, "demo_spherical_oculus");

    ros::NodeHandle nh;

    std::vector<std::string> path_yamls_cam;
    std::vector<std::string> topics_name;
    std::string maskCamera_1;
    std::string maskCamera_2;
    std::string extrinParam;

    const std::string cloudPtTopic = "/cloud_sphere";

    ros::Publisher pub_CloudSph = nh.advertise<sensor_msgs::PointCloud>(cloudPtTopic,0);

    std::string topic;
    nh.param("/spherical_vision/topic_left",topic,std::string("/left/image_raw"));
    topics_name.push_back(topic);
    nh.param("/spherical_vision/topic_right",topic,std::string("/right/image_raw"));
    topics_name.push_back(topic);

    path_yamls_cam.push_back(AddPath("etc/calib/proto2/Pal_intrinsicParam_cam1.yaml",conf_path));
    path_yamls_cam.push_back(AddPath("etc/calib/proto2/Pal_intrinsicParam_cam2.yaml",conf_path));

    extrinParam = AddPath("etc/calib/proto2/Pal_extrinsicParam.yaml",conf_path);

    maskCamera_1  = AddPath("etc/images/cam1/Img_mask1.jpg",conf_path);
    maskCamera_2  = AddPath("etc/images/cam2/Img_mask2.jpg",conf_path);

    PolyOmniCamera omniSys(topics_name,path_yamls_cam,extrinParam);

    omniSys.DispParam();

    omniSys.camera_1->LoadMask(maskCamera_1);
    omniSys.camera_2->LoadMask(maskCamera_2);

    omniSys.DownSample(sampling_ratio);

    omniSys.DispParam();

    sensor_msgs::PointCloud ptsCld;

//    double time;
//    char exit;

    omniSys.PartiallyFillMess(ptsCld);

    do
    {
        omniSys.camera_1->ReadFrame();
        omniSys.camera_2->ReadFrame();

//        time = (double)cv::getTickCount();

        omniSys.MessRGBSph(ptsCld);

        //std::cout << "time to comp sphere : "<<((double)cv::getTickCount() - time) / cv::getTickFrequency()<<std::endl<<std::endl;

//        time = (double)cv::getTickCount();

        pub_CloudSph.publish(ptsCld);

        //std::cout << "time to publish sphere : "<<((double)cv::getTickCount() - time) / cv::getTickFrequency()<<std::endl<<std::endl;

        ros::spinOnce();

        //TODO : exit on key pressing
//        exit = cv::waitKey(10);
//        if(exit == 27) break;

    }while(1);


    // Example of the demo from image folder

//    std::set<std::string> imagesR = loadFilesName("path_to_right_images");
//    std::set<std::string> imagesL = loadFilesName("path_to_left_images");

//    std::string file1,file2;

//    BOOST_FOREACH(boost::tie(file1,file2), boost::combine(imagesR,imagesL))
//    {
//        omniSys.camera_1->readImage(file1);
//        omniSys.camera_2->readImage(file2);

//        omniSys.MessRGBSph(ptsCld);

//        pub_CloudSph.publish(ptsCld);

//        ros::spinOnce();

//        exit = cv::waitKey(100);
//    }



    return 0;
}
