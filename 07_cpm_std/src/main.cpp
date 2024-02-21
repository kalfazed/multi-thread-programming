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
int  min_count = 10;

queue<Package>     q_;
mutex              mtx_;
condition_variable cv_;

void* produce(void* args){
    while(true){
        { /* 
             * 我们通过在函数中设置block, 让unique_lock的生命周期只在这个block内
             * 当block结束的时候，自动释放unique_lock，实现上锁解锁
             */
            unique_lock<mutex> lock(mtx_);
            Package p;
            p.id = global_id;
            p.status = "newly produced";
            q_.push(p);
            LOG(PURPLE "\t[Producer]: Produce pushed package %d, queue size is %d", p.id, q_.size());
            global_id ++;
            if (q_.size() >= min_count){
                LOG(RED "[Producer]: Activate all blocked threads");
                cv_.notify_all();
            }
        }
        // 不要把这和sleep_for写在unique_lock的生命周期范围内，否则会出现死锁
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void* consume(void* args){
    while(true){
        {
            unique_lock<mutex> lock(mtx_);
            while (q_.empty()){
                LOG(RED "[Consumer]: The thread is blocked, waiting ...");
                cv_.wait(lock);
            }

            Package tmp = q_.front();
            q_.pop();
            LOG(DGREEN "\t[Consumer]: Nonblocking pop package %d, queue size is %d", tmp.id, q_.size());
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

/*
 * 使用c++ std库中的condition_variable, mutex, unique_lock, thread来实现CPM
 * unique_lock是mutex lock的智能指针版，内部用类似与unique_ptr的方式封装的
 * 解锁和上锁直接是通过unique_lock的生命周期来管理，生命周期结束自动释放，所以一般可以配合函数block来上锁/解锁
 * std::condition_variable的wait和notify_all分别对应c语言版本的pthread_cond_wait和pthread_cond_broadcast
 */
int main(){
    int producer_count = 1;
    int consumer_count = 2;
    min_count = 10;

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
