#include "logger.hpp"
#include <chrono>
#include <string>
#include <stdarg.h>
#include <thread>
#include <future>
#include <queue>
#include <condition_variable>
#include <mutex>

using namespace std;

struct Data{
    int id;
    shared_ptr<promise<string>> status;
};

queue<Data>        q1_;
queue<Data>        q2_;
queue<Data>        q3_;
queue<Data>        q4_;

mutex              mtx_;
condition_variable cv_;
int                max_limit = 10;
int                global_id = 0;

void produce(int duration){
    while(true){
        LOG(PURPLE"\tProducer produces task %d", global_id);

        // 模拟producer端处理preprocess
        this_thread::sleep_for(chrono::milliseconds(100));
        LOG(DGREEN"\t\tPreprocess finished");

        // 模拟producer端调用N个consumer处理infer
        Data data1, data2, data3, data4;
        {
            unique_lock<mutex> lock(mtx_);
            data1.status.reset(new std::promise<string>());
            data2.status.reset(new std::promise<string>());
            data3.status.reset(new std::promise<string>());
            data4.status.reset(new std::promise<string>());

            data1.id = global_id;
            data2.id = global_id;
            data3.id = global_id;
            data4.id = global_id;

            q1_.push(data1);
            q2_.push(data2);
            q3_.push(data3);
            q4_.push(data4);
        }
        auto st1 = data1.status->get_future().get();
        auto st2 = data2.status->get_future().get();
        auto st3 = data3.status->get_future().get();
        auto st4 = data4.status->get_future().get();

        // 模拟producer端处理postprocess
        this_thread::sleep_for(chrono::milliseconds(200));
        global_id ++;
        LOG(DGREEN"\t\tPostprocess finished");


        LOG(PURPLE"\tFinished task %d, %s, %s, %s, %s\n", 
            global_id, st1.c_str(), st2.c_str(), st3.c_str(), st4.c_str());
    }
}

void consume(queue<Data>& q, string label, int duration){
    while(true){
        if (!q.empty()){
            int id;
            Data cData;
            {
                unique_lock<mutex> lock(mtx_);
                cData = q.front();
                id = cData.id;
                q.pop();
            }

            // 模拟consumer处理infer, 处理完infer之后更新data的status
            this_thread::sleep_for(chrono::milliseconds(duration));
            auto msg = label + "[" + to_string(cData.id) + "]";
            cData.status->set_value(msg);
            LOG(DGREEN"\t\t%s finished task[%d]", label.c_str(), id);
        }
    }
}



/*
 * 我们考虑一个场景，当producer生产完一个数据之后，consumer就开始处理这个数据
 * 生产数据和处理这个数据的过程，需要通过mutex lock来锁住，这个是我们到目前为止所作的事情
 * 但是，如果producer希望后续的处理得到consumer的这个数据的话，
 * 就说明需要某种机制，让consumer处理完数据以后去把结果告诉给producer，因此需要实现线程间的通信
 * future, promise就是负责这个的
 *
 * 场景是这样的：
 * - 我们让producer分配n个数据，之后等待这n个数据被各个consumer处理, 等待期间需要同步
 * - n个consumer分别处理n个数据, 并将处理完的数据写回
 * - producer每检测到其中一个数据更新了，取消这个数据的等待
 * - 等所有的数据被consumer处理完了，producer就可以处理producer的后续计算
 * 以上就是一次任务的执行(1 producer N consumer)。通过future-promise实现并行并大大提高一个任务的吞吐量
 *
 * 这个案例的实现是对每一个task分配一个对应的queue和线程，之后进行异步处理
 * 是否可以有更好的办法去处理呢？比如说只用一个queue来处理多个task的异步执行
*/

int main(){
    int producer_count = 1;

    thread producer(produce, 500);

    thread consumer1(consume, ref(q1_), "detection", 200);
    thread consumer2(consume, ref(q2_), "segmentation", 200);
    thread consumer3(consume, ref(q3_), "depth estimation", 100);
    thread consumer4(consume, ref(q4_), "pose estimation", 100);

    producer.join();
    consumer1.join();
    consumer2.join();
    consumer3.join();
    consumer4.join();

    return 0;
}
