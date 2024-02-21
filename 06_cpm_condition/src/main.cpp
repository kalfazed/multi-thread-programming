#include "logger.hpp"
#include <chrono>
#include <string>
#include <stdarg.h>
#include <thread>
#include <mutex>
#include <memory>
#include "pthread.h"
#include "condition_variable"


/*
 * 条件变量可以和互斥锁配合起来实现“生产者-消费者模型”
 *  - 条件变量负责阻塞
 *  - 互斥锁负责同步
 *
 * 比如有一个队列，生产者负责往队列里放数据，消费者负责从队列中读取数据
 * 当队列里的数据如果空了，消费者就需要停止取出数据; 同样的，当队列里的数据满了，生产者需要停止放数据
 * 判断生产者和消费者与队列中数据的交互与否，可以通过”条件变量“(pthread_cond_t)来判断
 * pthread_cond_t可以用来实现阻塞线程的功能，以及存储唤醒被阻塞的线程的id
 *
 * 跟pthread相关的函数有:
 *
 *  pthread_cond_init              (初始化)
 *  pthread_cond_destroy           (销毁)
 *  pthread_cond_wait              (阻塞线程)
 *  pthread_cond_timedwait         (阻塞线程一定的时间，时间到了停止阻塞)
 *  pthread_cond_signal            (唤醒阻塞在条件变量上的线程)
 *  pthread_cond_broadcast         (唤醒阻塞在条件变量上的所有线程)
*/

using namespace std;

struct Package {
    int id;
    string status;
    shared_ptr<Package> next;
};

pthread_cond_t cond;
pthread_mutex_t mtx;

// 全局变量
int  min_count = 0;
int  global_id = 0;
int  count = 0;
auto head = make_shared<Package>();


void* produce(void* args){
    // 无限的每隔一秒种往链表中添加元素
    while(1){
        pthread_mutex_lock(&mtx);
        {
            auto pack = make_shared<Package>();
            pack->id  = global_id;
            pack->status = "newly produced";
            pack->next = head;
            head = pack;
            global_id ++;
            count ++;
            LOG(PURPLE "\t[Producer]: Produce package %d, current length is %d", pack->id, count);

            if (count >= 10){
                LOG(RED "[Producer]: Activate all blocked threads");
                pthread_cond_broadcast(&cond);
            }
        }
        pthread_mutex_unlock(&mtx);
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void* consume(void* args){
    /*
     * 每隔0.5s就从链表中取出元素。
     * 由于设定的是consume会比produce的快，所以肯定会出现consume空的时候
     * 当链表为空的时候就不要再取出元素了, 阻塞consumer线程
    */
    while(1){
        pthread_mutex_lock(&mtx);
        {
            /*
             * 当链表为空的时候，就阻塞consumer线程。解除阻塞是在produser线程里进行的
             * 如果某一个线程被阻塞了，那么pthread_cond_wait会解除对应的mutex lock，防止死锁的出现
             * 当线程不再被阻塞了，那么抢到时间片的线程会自动给mutex lock上锁，实现原子操作。
             */
            while(head->next == nullptr){
                LOG(RED "[Consumer]: The thread is blocked, waiting ...");
                pthread_cond_wait(&cond, &mtx);
            }

            auto pack = head;
            count --;
            head = head->next;
            LOG(DGREEN "\t[Consumer]: Nonblocking consume package %d, current length is %d", pack->id, count);
        }
        pthread_mutex_unlock(&mtx);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

}

/*
 * 实现一个Consumer-Producer-Model(CPM)的一个基本操作
 * 这里设定的场景是:
 *
 * 1. 消费者和生产者同时处理同一个链表
 * 2. 当链表为空，消费者停止消费并阻塞
 * 3. 当链表里的数据的个数大于某一个值的时候，生产者解除阻塞
 * 4. 消费者消费的速度要快于生产者
 * 5. 生产者生产不设置上限
 * 6. 生产者和消费者可以设置多个来并行
 *
 * 这个CPM案例producer会通过pthread_cond_broadcast来解除consumer的block
 *  (比如说，当链表中的数据大于某个值的时候再consume)
 * 同样的，consumer也可以通过pthread_cond_broadcast来解除producer的block
 *  (比如说，当链表中的数据小于某个值的时候再produce)
 * 需要注意的是，这种情况下需要使用两个不同的pthread_cond才行
 */

int main(){
    pthread_cond_init(&cond, nullptr);
    pthread_mutex_init(&mtx, nullptr);

    /* 这里根据情况修改测试的大小*/
    int producer_count = 1;
    int consumer_count = 2;
    min_count = 10;

    pthread_t* producers = (pthread_t*)malloc(producer_count * sizeof(pthread_t));
    pthread_t* consumers = (pthread_t*)malloc(consumer_count * sizeof(pthread_t));

    // 初始化
    for(int i = 0; i < producer_count; i ++){
        pthread_create(&producers[i], nullptr, produce, nullptr);
    }
    for(int i = 0; i < consumer_count; i ++){
        pthread_create(&consumers[i], nullptr, consume, nullptr);
    }

    // 同步
    for(int i = 0; i < producer_count; i ++){
        pthread_join(producers[i], nullptr);
    }

    for(int i = 0; i < consumer_count; i ++){
        pthread_join(consumers[i], nullptr);
    }

    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond);
    return 0;
}
