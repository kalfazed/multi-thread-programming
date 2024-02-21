#include <thread>
#include <queue>
#include <string>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <future>
#include "logger.hpp"
#include "pcm.hpp"

using namespace std;
namespace pcm{

void Producer(std::queue<Job> q){
    while(true){
        std::string str = s_base + std::to_string(iter++);
        Job job;
        {
            std::unique_lock<std::mutex> l(lock_);
            cv_.wait(l,[&](){return q_.size() < limit;});
            job.pro.reset(new std::promise<std::string>());
            job.name = str;
            q_.push(job);
            INFO("Producer produces %s, queue size is %d", str.c_str(), q_.size());
        }
        auto res =  job.pro->get_future().get();
        INFO("Producer result:  %s\n", res.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void Consumer(std::queue<Job> q){
    while(true){
        if (!q_.empty()){
            {
                std::lock_guard<std::mutex> l(lock_);
                auto pjob = q_.front();
                q_.pop();
                cv_.notify_one();
                INFO("Consumer consums  %s", pjob.name.c_str());
                auto result = pjob.name + "---infer";
                pjob.pro->set_value(result);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
        std::this_thread::yield();
    }
}

void VideoCapture(){
    while(true){
        string str = s_base + to_string(iter++);
        Job img;
        {
            unique_lock<mutex> l(lock_);
            cv_.wait(l, [&](){return images_.size() < limit;});
            img.pro.reset(new promise<string>());
            img.name = str;
            images_.push(img);
            INFO("Generated image %s", img.name.c_str());
        }
        auto item = img.pro->get_future().get();
        INFO("Infered image %s", item.c_str());
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void InferWorker(){
    while(true){
        if (!images_.empty()){
            {
                lock_guard <mutex> l(lock_);
                auto pJob = images_.front();
                images_.pop();
                cv_.notify_all();
                INFO("finished pop %s", pJob.name.c_str());
                string result = pJob.name + "----infer";
                pJob.pro->set_value(result);
            }
            this_thread::sleep_for(chrono::milliseconds(1000));
        }
        this_thread::yield();
    }
}

} // namespace pcm
