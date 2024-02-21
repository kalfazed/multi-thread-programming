#include "logger.hpp"
#include "model.hpp"
#include "opencv2/opencv.hpp"
#include <string>

using namespace std;

// 20张同样的照片表示一个batch
string imgPaths[20] = {
    "data/source/car.jpg",
    "data/source/airport.jpg",
    "data/source/bedroom.jpg",
    "data/source/crossroad.jpg",
    "data/source/crowd.jpg",
    "data/source/car.jpg",
    "data/source/airport.jpg",
    "data/source/bedroom.jpg",
    "data/source/crossroad.jpg",
    "data/source/crowd.jpg",
    "data/source/car.jpg",
    "data/source/airport.jpg",
    "data/source/bedroom.jpg",
    "data/source/crossroad.jpg",
    "data/source/crowd.jpg",
    "data/source/car.jpg",
    "data/source/airport.jpg",
    "data/source/bedroom.jpg",
    "data/source/crossroad.jpg",
    "data/source/crowd.jpg",
};

int main(){
    logger::set_log_level(logger::LogLevel::Info);

    auto producer = model::create_model(imgPaths, 20);
    auto results  = producer->commits();

    for (auto& res: results){
        model::img info = res.get();
    };
    producer->stop();
}
