#include "logger.hpp"
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>

using namespace std;

struct Package {
    int id;
    string status;
};

int  global_id = 0;
int  min_limit = 10;
int  max_limit = 50;

queue<Package>     q_;
mutex              mtx_;
condition_variable cv_max_limit;
condition_variable cv_min_limit;
bool over_limit = false;
bool beyond_min = false;

void* produce(void* args){
    while(true){
        {
            unique_lock<mutex> lock(mtx_);
            /*
             * 当queue的大小到达上限的时候阻塞producer, 
             * 等待consumer把数据pop到最小数目以后再开始push
             */
            cv_max_limit.wait(lock, [](){return q_.size() < max_limit;});

            Package p;
            p.id = global_id;
            p.status = "newly produced";
            q_.push(p);
            LOG(PURPLE "\t[Producer]: Produce pushed package %d, queue size is %d", p.id, q_.size());
            global_id ++;
            cv_min_limit.notify_all();

        }
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void* consume(void* args){
    while(true){
        {
            unique_lock<mutex> lock(mtx_);
            /*
             * 当queue的大小没有达到最小数量的时候不进行pop
             * 等待producer把queue给push到一定数量以后再开始pop
             */
            cv_min_limit.wait(lock, [](){return q_.size() > min_limit;});

            Package tmp = q_.front();
            q_.pop();
            LOG(DGREEN "\t[Consumer]: Nonblocking pop package %d, queue size is %d", tmp.id, q_.size());

            cv_max_limit.notify_all();
        }
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

/*
 * 我们设置一个新的案例：
 * 1. queue需要有produce的上限max_limit，和consume的下限min_limit
 * 2. producer的生产速度比consumer的消费速度可快可慢
 * 3. 当queue中的数据超过上限limit的时候，阻塞producer, 直到consumer把所有的元素全部pop掉之后在produce
 * 4. 当queue中没有元素的时候，阻塞consumer, 直到producer把queue里面的元素填充到下限min_count以后再开始pop
 *
 * 这个案例中需要有两个condition_variable来实现两种不同的阻塞
 * 
 * 当生产速度 > 消费速度时，queue里元素变化的速度有三个阶段
 * 1. 阻塞consumer, 等待queue元素达到最小值
 * 2. consumer, producer同时进行，知道queue元素达到最大值
 * 3. 阻塞producer, 此时pruducer的速度和consumer的速度一致
 *
 * 当生产速度 <= 消费速度时，queue里元素变化的速度有两个阶段
 * 1. 阻塞consumer, 等待queue元素达到最小值
 * 3. 阻塞consumer, 此时pruducer的速度和consumer的速度一致
 */

int main(){
    int producer_count = 5;
    int consumer_count = 1;
    min_limit = 10;
    max_limit = 50;

    vector<thread> producers;
    vector<thread> consumers;

    /* 初始化 */
    for (int i = 0; i < producer_count; i++) {
        producers.emplace_back(produce, nullptr);
    }

    for (int i = 0; i < consumer_count; i++) {
        consumers.emplace_back(consume, nullptr);
    }
    

    /* 同步 */
    for (auto& producer: producers){
        producer.join();
    }
    for (auto& consumer: consumers){
        consumer.join();
    }


    return 0;
}
