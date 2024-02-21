#include "logger.hpp"
#include <chrono>
#include <string>
#include <thread>
#include <pthread.h>
#include <memory>

using namespace std;

struct data{
    int    id;
    int    count;
    int    sleepTime;
    string msg;
};

// 使用smart_ptr来防止忘记free
auto msg_1 = make_shared<string>(DGREEN "sub_thread_100ms" CLEAR);
auto msg_2 = make_shared<string>(PURPLE "sub_thread_200ms" CLEAR);
auto d1 = make_shared<data>(data{0, 30, 100, *msg_1});
auto d2 = make_shared<data>(data{1, 30, 200, *msg_2});


void* run(void* args){
    data* d = static_cast<data*>(args);

    for (int i = 0; i < d->count; i ++){
        LOG("Runing thread %ld: %s, count: %d", d->id, d->msg.c_str(), i);
        this_thread::sleep_for(chrono::milliseconds(d->sleepTime));
    }

    d->msg = YELLOW "sub_thread finished " CLEAR;
    return nullptr;
}

int main(){
    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, nullptr, run, d1.get());
    pthread_create(&t2, nullptr, run, d2.get());
    
    /*
     * 有的时候我们不希望在主线程中使用join, 原因是因为join会阻塞主线程的执行
     * 那么，我们可以选择将主线程和子线程分离开(detach)，各自执行各自的内容
     * 这样子线程的执行结束以后的资源回收是由其他线程来负责，不再由主线程管理
     * 但是需要注意的是，主线程在执行完毕以后会释放空间，这样detach出去的子线程的空间也会被释放
     * 所以需要pthread_exit来表示主线程结束之后资源也不要释放
     * 但是需要注意的是，由于子线程和主线程分离了，所以主线程结束之后会自动释放资源，包括msg_1和msg_2
     * 所以这两个资源需要是全局的，否则子线程分离后无法访问
    */
    pthread_detach(t1);
    pthread_detach(t2);

    pthread_exit(NULL);

    return 0;
}
