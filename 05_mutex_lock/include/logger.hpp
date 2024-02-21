#ifndef LOGGER_HPP
#define lOGGER_HPP

#include <string>

// 这里进行宏定义，目的是为了能够在预编译的时候展开，从而加速
#define LOGD(...)                 logger::__make_log(__FILE__,__LINE__, logger::LogLevel::Debug, __VA_ARGS__)
#define LOGV(...)                 logger::__make_log(__FILE__,__LINE__, logger::LogLevel::Verbose, __VA_ARGS__)
#define LOG(...)                  logger::__make_log(__FILE__,__LINE__, logger::LogLevel::Info, __VA_ARGS__)
#define LOGW(...)                 logger::__make_log(__FILE__,__LINE__, logger::LogLevel::Warning, __VA_ARGS__)
#define LOGE(...)                 logger::__make_log(__FILE__,__LINE__, logger::LogLevel::Error, __VA_ARGS__)
#define LOGF(...)                 logger::__make_log(__FILE__,__LINE__, logger::LogLevel::Fatal, __VA_ARGS__)

#define DGREEN    "\033[1;36m"
#define BLUE      "\033[1;34m"
#define PURPLE    "\033[1;35m"
#define GREEN     "\033[1;32m"
#define YELLOW    "\033[1;33m"
#define RED       "\033[1;31m"
#define CLEAR     "\033[0m"

namespace logger {
    enum class LogLevel : int {
        Debug    = 5,
        Verbose  = 4,
        Info     = 3,
        Warning  = 2,
        Error    = 1,
        Fatal    = 0,
    };
    void set_log_level(LogLevel level);
    void __make_log(const char* file, int line, LogLevel level, const char* format, ...);
}; // namespace logger

#endif
