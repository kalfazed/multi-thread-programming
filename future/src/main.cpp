#include <thread>
#include "pcm.hpp"

using namespace std;

void pcm_test(){
    thread t1(pcm::Producer, pcm::q_);
    thread t2(pcm::Consumer, pcm::q_);
    t1.join();
    t2.join();
}

void video_infer_test(){
    thread t1(pcm::VideoCapture);
    thread t2(pcm::InferWorker);
    t1.join();
    t2.join();
}

int main(){
    pcm_test();
    // video_infer_test();
    return 0;
}
