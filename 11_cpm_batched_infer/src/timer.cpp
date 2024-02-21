#include <chrono>
#include <iostream>
#include <memory>
#include "timer.hpp"

#include "utils.hpp"

namespace timer {

Timer::Timer(){
    _timeElasped = 0;
    _cStart = std::chrono::high_resolution_clock::now();
    _cStop = std::chrono::high_resolution_clock::now();
}

Timer::~Timer(){
}

void Timer::start_cpu() {
    _cStart = std::chrono::high_resolution_clock::now();
}

void Timer::stop_cpu() {
    _cStop = std::chrono::high_resolution_clock::now();
}

} //namespace model
