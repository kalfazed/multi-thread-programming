#include "logger.hpp"
#include <algorithm>
#include <chrono>
#include <string>
#include <stdarg.h>
#include <thread>
#include <pthread.h>

using namespace std;

void* run1(void* args){
    while(1){
        LOG("Runing thread %ld: %s", pthread_self(), args);
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void* run2(void* args){
    while(1){
        LOG("Runing thread %ld: %s", pthread_self(), args);
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

int main(){
    char* msg_m = YELLOW "main_thread_1000ms" CLEAR;
    char* msg_1 = DGREEN "sub_thread_500ms" CLEAR;

    pthread_t t1;
    /*
     * pthread_create的几个参数
     * 1. 线程地址
     * 2. 线程属性
     * 3. 回调函数, 返回类型和参数都是void*
     * 4. 回调函数的参数
    */
    pthread_create(&t1, NULL, run1, msg_1);
    
    run2(msg_m);

    pthread_join(t1, NULL);

    return 0;
}
