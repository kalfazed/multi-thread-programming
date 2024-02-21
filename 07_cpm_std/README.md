# 使用c++进行多线程编程

## 涉及到的知识点
- 使用thread库的函数
- 线程的开启和同步
- 双线程，多线程异步执行的方法
- 生产者/消费者模型
- 使用conditional variable进行约束
- 使用future和promise进行线程结果的返回

## 整体的流程是这样的
### Goal: 
case1: 两个线程
- 一个线程负责给queue push东西
- 一个线程负责给queue pop东西。两个线程的速度不一样

case2: 两个线程
- 一个线程是master-thread负责获取图像
- 另外一个线程是worker-thread负责处理图像，并且将图像进行处理之后返回给master-thread

case3: 多个线程
- 一个线程是master-thread负责获取图像
- 另外两个线程是worker-thread负责处理图像，并且将图像进行处理之后返回给master-thread

### 说明
- this_thread::yield()的作用是让出当前线程的cpu时间片，可以让其他线程使用，可以防止资源浪费
- 对于shared object需要通过mutex lock来进行维护
- 当生产者生产的速度远大于消费者的消费速度的话，需要使用conditional variable来进行等待
- conditional variable
    - wait()
    - notify_one()
    - cv.wait(lock, [](){return ready;})等价于while(!ready){cv.wait(lock);}
    - 需要注意的是，conditional variable一起绑定的lock必须是unique_lock, 而不是lock_guard
- 异步执行
    - 我们希望做到的是，有一个master thread和很多个worker thread。这些worker thread统一性的获取master thread的图像，之后各自做处理。处理结束以后返回给master thread，在master thread进行接下来的操作。比如绘制bbox, 绘制segmentation mask, 绘制keypoint
