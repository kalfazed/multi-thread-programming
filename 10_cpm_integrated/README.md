## 问题所在
我们把生产者-消费者模型考虑放在实际的应用场景中

e.g.
- 生产者: 从图片集中读取n张图片, 之后做infer
    - 一般只有一个生产者
    - 读完n张图片之后，做batched DNN forward
    - 之后将
- 消费者: 处理视频流的线程, 做infer处理, 可以infer一张图片，也可以infer多张图片
    - 一般可以有多个消费者
    - 比如说，消费者0负责detection0, 消费者1负责detection1, ...
    - 再比如说，消费者0负责detection, 消费者1负责segmentation, ...
这样可以实现multi-batch inference



