#include "logger.hpp"
#include <chrono>
#include <string>
#include <stdarg.h>
#include <thread>

using namespace std;

void run1(string name){
    while(1){
        LOG("Runing thread %s", name.c_str());
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void run2(string name){
    while(1){
        LOG("Runing thread %s", name.c_str());
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

void run3(string name){
    while(1){
        LOG("Runing thread %s", name.c_str());
        this_thread::sleep_for(chrono::milliseconds(2000));
    }
}

int main(){
    thread t1(run1, DGREEN "thread_500ms"  CLEAR);
    thread t2(run2, RED    "thread_1000ms" CLEAR);
    thread t3(run3, PURPLE "thread_2000ms" CLEAR);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}
