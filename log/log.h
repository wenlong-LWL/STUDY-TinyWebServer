#ifndef LOG_H
#define LOG_H

#include <string>
#include <../utils/locker.h>

class Log {
    public:
        static Log* get_instance();
        void flush();
        void write_log(int level, const char* format, ...);
        void init(std::string log_path, bool is_close_log, bool is_async, int buf_size = 8192);

        static void* flush_log_thread(void*);

    private:
        Log();
        ~Log();

    private:
        bool m_is_close_log;   //开关日志
        bool m_is_async;       //同/异步标志位
        std::string m_dir_name;  //日志路径名
        std::string m_log_name;  //日志文件名
        int m_buf_size;          //缓冲区大小
        int m_today;  //当日日期
        FILE *m_fp;   //日志文件指针
        char *m_buf;  //缓冲区
        locker m_mutex;

};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->fflush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->fflush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->fflush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->fflush();}

#endif