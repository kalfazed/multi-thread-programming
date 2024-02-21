#include <string>
#include <iostream>
#include <stdarg.h>
#include "logger.hpp"
#include "time.h"

using namespace std;
namespace logger
{
    LogLevel g_level = LogLevel::Info;

    string time_now(){
        time_t timep;
        char time_string[20];
        // 获取现在的时间
        time(&timep);
        tm &t = *localtime(&timep);
        snprintf(time_string, sizeof(time_string), 
            "%04d-%02d-%2d %02d:%02d:%02d", 
            t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
        return time_string;
    }

    // 注意，c++中的string是一个类，所以如果想要打印的话，需要使用c_str()
    string level_string(LogLevel level){
        switch (level){
        case LogLevel::Debug: return "debug";
        case LogLevel::Verbose: return "verbose";
        case LogLevel::Info: return "info";
        case LogLevel::Warning: return "warning";
        case LogLevel::Error: return "error";
        case LogLevel::Fatal: return "fatal";
        default: return "unknown";
        }
    }
    void set_log_level(LogLevel level){
        g_level = level;
    }
    void __make_log(const char* file, int line, LogLevel level, const char* format, ... ){
        // 如果是verbose的话就取消？
        // if (level > g_level) return;
        // 定义一个char*类型的vl，用来指向可变参数
        va_list vl;
        // 通过va_start开始存放从format开始之后的第一个参数
        va_start(vl, format);

        char buff[2000];
        int n = 0;
        auto now = time_now();
        // print time
        n += snprintf(buff + n, sizeof(buff) - n, YELLOW"[%s]"CLEAR, now.c_str());

        // print log level
        if (level == LogLevel::Debug){
            n += snprintf(buff + n, sizeof(buff) - n, DGREEN"[%s]"CLEAR, level_string(level).c_str());
        }else if (level == LogLevel::Verbose){
            n += snprintf(buff + n, sizeof(buff) - n, PURPLE"[%s]"CLEAR, level_string(level).c_str());
        }else if (level == LogLevel::Info){
            n += snprintf(buff + n, sizeof(buff) - n, GREEN"[%s]"CLEAR, level_string(level).c_str());
        }else if (level == LogLevel::Warning){
            n += snprintf(buff + n, sizeof(buff) - n, BLUE"[%s]"CLEAR, level_string(level).c_str());
        }else if (level == LogLevel::Error || level == LogLevel::Fatal){
            n += snprintf(buff + n, sizeof(buff) - n, RED"[%s]"CLEAR, level_string(level).c_str());
        }

        // print file and line
        n += snprintf(buff + n, sizeof(buff) - n, "[%s:%d]", file, line);

        // print va_args
        n += vsnprintf(buff + n, sizeof(buff) - n, format, vl);
        fprintf(stdout, "%s\n", buff);
        // free va_list
        va_end(vl);
        
        if (level == LogLevel::Error || level == LogLevel::Fatal){
            fflush(stdout);
            abort();
        }
    }
} // namespace logger
