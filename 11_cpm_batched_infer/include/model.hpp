#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include <memory>
#include <string>
#include <future>
#include <vector>
#include <string>
#include "opencv2/opencv.hpp"

namespace model{

struct img{
    cv::Mat data;
    std::string path;
};

class Model{

public:
    virtual void forward() = 0;
};

std::shared_ptr<Model> create_model (std::string img_list, int batchSize);
    
}// namespace model
#endif __MODEL_HPP__
