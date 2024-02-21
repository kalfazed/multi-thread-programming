# 使用Logger记录日志的c++实现

## 涉及到的知识点
- C++中的基本封装的注意点
- 使用智能指针进行分配内存和释放
- 使用__FILE__和__LINE__进行更方便的调试和记录
- 灵活使用可变参数
- 各种prinf的方法(printf, sprintf, snprintf, vsnprintf)
- 使用time_t获取准确时间
- 熟悉使用string的各种函数

在封装的时候注意尽量让接口简单明了。不要让caller关注没有用的东西

## 整体的流程是这样的
Goal: 能够在main函数中调用各种级别的log
log的接口可以是
- LOG(...);
- LOGV(...);
- LOGW(...);
- LOGE(...);
需要注意的几个点：
1. 可变参数声明的时候使用__VA_ARGS__, 调用的时候使用va_list, va_start, va_args, va_end~
2. LOG中的参数是基本上是(char\* format, ...)~
3. 使用各种prinf来进行保存字符串

