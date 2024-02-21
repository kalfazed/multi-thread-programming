#include "logger.hpp"
#include <chrono>
#include <string>
#include <stdarg.h>
#include <thread>
#include <mutex>

int global_var = 0;
std::mutex mtx;


void task() {
    for (int i = 0; i < 100000; i ++){
        // 通过mutex lock将变量给lock住，从而对变量可以进行原子操作
        mtx.lock();
        global_var ++;
        global_var --;
        mtx.unlock();
    }
}


int main(){
    std::thread t1(task);
    std::thread t2(task);

    t1.join();
    t2.join();

    LOG("%d", global_var);

    return 0;
}
