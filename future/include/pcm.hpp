#ifndef PCM_HPP
#define PCM_HPP

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <future>


namespace pcm{

struct Job{
    std::shared_ptr<std::promise<std::string>> pro;
    std::string name;
};

void Producer(std::queue<Job> q);
void Consumer(std::queue<Job> q);
void VideoCapture();
void InferWorker();

static std::queue<Job> q_;
static std::queue<Job> images_;
static std::string s_base = "image";
static int iter = 0;
static int limit = 10;
static std::mutex lock_;
static std::condition_variable cv_;

}; // namespace pcm

#endif