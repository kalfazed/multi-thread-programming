## 问题所在
这个版本的CPM是我们在之前的CPM基础上进一步更新

我们回顾一下上一个版本的CPM
1. model在初始化的时候把所有的图片放在一个batch里，准备初始化
2. model在commit的时候当前的所有的图片作为一个batch，分给n个消费者进行处理
3. main端手动通过get来进行多线程的同步

## 我们想要实现的功能
在这个版本的CPM中，我们希望在forward里面做这么几件事情
1. model初始化的时候，把所有的图片做成一个list
2. 生产者从list中读取一个batch的图片，准备commit
     - 生产者读取1个batchSize的数据, 之后将他们push到jobQueue中
3. 多个消费者从jobQueue中Pop出Job, 对Job做处理后，将Job中的promise给set_value
4. 对当前生产者所生产的所有job进行get，做同步
5. 生产者读取下一个batch, 循环到第二步，直到所有的图片读完

有一点不同的是，我们需要让model自动进行get处理，这个逻辑是放在forward里面的。
这样只要我们创建好了一个图片list，以及指定batchSize以后，
model就自动的为我们创建线程，自动同步，执行完某一个batch的推理以后，自动寻找下一个batch的图片进行推理。
在此期间不会再重新分配线程，重复一直使用分配好的线程，这样可以避免分配&销毁线程的开销。也就是线程池。
这个逻辑可以让main写的更加简单一点

## throughput test
为了测速，我们这里给添加一个测量throughput的方法。作为参考。各个batch的throughput如下
|---|---|
|batchSize|throughput|
|1|40.55 images/s|
|2|76.73 images/s|
|4|142.75 images/s|
|8|215.07 images/s|
|16|250.17 images/s|
|32|239.24 images/s|
|64|191.98 images/s|
从这里我们可以看到，在随着batchSize的增加，吞吐量会提高。但是过多的增大batchSize反而会影响吞吐量。
主要是因为thread越多就代表同步以及互斥的访问临界区所造成的overhead可能会更大，这一点是在做multi-thread programming时我们需要注意的
