#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <chrono>
#include <ratio>
#include <string>
#include "utils.hpp"

namespace timer{

class Timer {
public:
    using s  = std::ratio<1, 1>;
    using ms = std::ratio<1, 1000>;
    using us = std::ratio<1, 1000000>;
    using ns = std::ratio<1, 1000000000>;

public:
    Timer();
    ~Timer();

public:
    void start_cpu();
    void stop_cpu();

    template <typename span>
    void duration_cpu(std::string msg);

    template <typename span>
    void throughput_cpu(std::string msg, int size);

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _cStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> _cStop;
    float _timeElasped;
};

template <typename span>
void Timer::duration_cpu(std::string msg){
    std::string str;

    if(std::is_same<span, s>::value) { str = "s"; }
    else if(std::is_same<span, ms>::value) { str = "ms"; }
    else if(std::is_same<span, us>::value) { str = "us"; }
    else if(std::is_same<span, ns>::value) { str = "ns"; }

    std::chrono::duration<double, span> time = _cStop - _cStart;
    LOGV("%-60s uses %.6lf %s", msg.c_str(), time.count(), str.c_str());
}

template <typename span>
void Timer::throughput_cpu(std::string msg, int size){
    std::string str;

    if(std::is_same<span, s>::value) { str = "s"; }
    else if(std::is_same<span, ms>::value) { str = "ms"; }
    else if(std::is_same<span, us>::value) { str = "us"; }
    else if(std::is_same<span, ns>::value) { str = "ns"; }

    std::chrono::duration<double, span> time = _cStop - _cStart;
    LOG("%s throughput: %.6lf images/%s", msg.c_str(), size / time.count(), str.c_str());
}

} // namespace timer

#endif //__TIMER_HPP__
