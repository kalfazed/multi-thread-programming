#include "logger.hpp"
#include <chrono>
#include <string>
#include <thread>
#include <pthread.h>
#include <memory>

using namespace std;

struct data{
    int   id;
    int   count;
    int   sleepTime;
    string msg;
};

void* run(void* args){
    // 对void*做一个类型强制转换，这里建议使用static_cast来提高代码的安全性
    data* d = static_cast<data*>(args);

    for (int i = 0; i < d->count; i ++){
        LOG("Runing thread %ld: %s, count: %d", d->id, d->msg.c_str(), i);
        this_thread::sleep_for(chrono::milliseconds(d->sleepTime));
    }

    // 通过pthread_exit终止函数，并返回回收用的资源。类似于return
    d->msg = YELLOW "sub_thread finished " CLEAR;

    return nullptr;
}


int main(){
    char* msg_1 = DGREEN "sub_thread_100ms" CLEAR;
    char* msg_2 = PURPLE "sub_thread_200ms" CLEAR;

    pthread_t t1;
    pthread_t t2;

    // 使用smart_ptr来防止忘记free
    auto d1 = make_shared<data>(data{0, 30, 100, msg_1});
    auto d2 = make_shared<data>(data{1, 30, 200, msg_2});

    pthread_create(&t1, nullptr, run, d1.get());
    pthread_create(&t2, nullptr, run, d2.get());
    
    /*
     * pthread_join是可以用来回收资源的,它是一个阻塞函数
     * 当p1在调用p2的时候，p2的返回值会写入到pthread_join里的第二个参数
     * 这样，p1可以回收p2的返回值
     * 回收用的参数是void**，可以是指针的地址。
     * 但这里由于使用的是shared_ptr，所以可以不用明确回收的参数
     */
    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    // 执行结束以后，把回收下来的值取出
    LOG("thread %d: %s", d1->id, d1->msg.c_str());
    LOG("thread %d: %s", d2->id, d2->msg.c_str());

    return 0;
}
