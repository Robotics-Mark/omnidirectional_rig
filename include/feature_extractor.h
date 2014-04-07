#ifndef PAL_VISUAL_LOCALIZATION_FEATURE_EXTRACTOR_H_
#define PAL_VISUAL_LOCALIZATION_FEATURE_EXTRACTOR_H_

#include <vector>
#include <opencv2/core/core.hpp>
#include <vocabulary_tree/tree_builder.h>
#include <sensor_msgs/Image.h>
#include <siftfast/siftfast.h>

#include <eigen3/Eigen/Core>


namespace pal {
  namespace slam {


    typedef Eigen::Matrix<float, 1, 128> Feature;
    typedef std::vector<Feature, Eigen::aligned_allocator<Feature> > FeatureVector;

    class FeatureExtractor
    {
    public:
      FeatureExtractor();
      ~FeatureExtractor();

      static cv::Mat getImageFromMessage(const sensor_msgs::ImageConstPtr& sensor_image);

      FeatureVector processFile(const std::string& filename);
      FeatureVector processImage(const cv::Mat& image);

    private:
      Keypoint extractKeypoints(const cv::Mat& image);
    };

  }
}

#endif  // PAL_VISUAL_LOCALIZATION_FEATURE_EXTRACTOR_H_
