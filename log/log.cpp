#include <string.h>
#include <ctime>
#include <stdexcept>
#include <stdarg.h>
#include <iostream>
#include "log.h"
#define pf(str) std::cout << str << std::endl;

Log::Log() {
    this->m_is_async = false;
}

Log::~Log() {
    if (this->m_buf)
        delete[] this->m_buf;
    if (this->m_fp)
        fclose(this->m_fp);
}

Log* Log::get_instance() {
    static Log instance;
    return &instance;
}

void Log::init(std::string log_path, bool is_close_log, bool is_async, int block_queue_size, int buf_size) {
    this->m_is_close_log = is_close_log;
    if (is_close_log)
        return;

    if (block_queue_size >= 1) {
        this->m_log_queue = new block_queue<std::string>(block_queue_size);
        is_async = true;
        pthread_t pid;
        pthread_create(&pid, NULL, flush_log_thread, NULL);
    }

    this->m_is_async = is_async;
    this->m_buf_size = buf_size;
    this->m_buf = new char[m_buf_size];
    memset(this->m_buf, 0, m_buf_size);

    std::tm mtm;
    std::time_t now = std::time(NULL);
    localtime_r(&now, &mtm);
    //std::tm *mtm_p = std::localtime(&now);
    //std::tm mtm = *mtm_p;
    this->m_today = mtm.tm_mday;

    size_t pos = log_path.rfind("/");
    char log_full_name[256] = { 0 };
    if (pos == std::string::npos) {
        this->m_log_name = log_path;
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", mtm.tm_year + 1900, mtm.tm_mon + 1, mtm.tm_mday, this->m_log_name.c_str());
    }
    else {
        this->m_log_name = log_path.substr(pos + 1);
        this->m_dir_name = log_path.substr(0, pos + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", this->m_dir_name.c_str(), mtm.tm_year + 1900, mtm.tm_mon + 1, mtm.tm_mday, this->m_log_name.c_str());
    }
    this->m_fp = fopen(log_full_name, "a");
    if (this->m_fp == NULL) {
        throw std::runtime_error("Failed to open log file: " + std::string(log_full_name));
    }
    if (this->m_is_async) {
        char buffer[64];
        sprintf(buffer, "%d-%02d-%02d %02d:%02d:%02d-----------------------\n", mtm.tm_year + 1900, mtm.tm_mon + 1, mtm.tm_mday, mtm.tm_hour, mtm.tm_min, mtm.tm_sec);
        this->m_log_queue->push(buffer);
    }
    else {
        fprintf(this->m_fp, "%d-%02d-%02d %02d:%02d:%02d-----------------------\n", mtm.tm_year + 1900, mtm.tm_mon + 1, mtm.tm_mday, mtm.tm_hour, mtm.tm_min, mtm.tm_sec);
    }
}

void Log::write_log(int level, const char* format, ...) {
    timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    std::time_t now_sec = now.tv_sec;
    std::tm mtm;
    localtime_r(&now_sec, &mtm);

    char level_buf[10] = { 0 };
    switch (level) {
    case 0:
        strcpy(level_buf, "[debug]:");
        break;
    case 1:
        strcpy(level_buf, "[info]:");
        break;
    case 2:
        strcpy(level_buf, "[warn]:");
        break;
    case 3:
        strcpy(level_buf, "[error]:");
        break;
    default:
        strcpy(level_buf, "[info]:");
        break;
    }

    this->m_mutex.lock();
    char log_full_name[256] = { 0 };
    if (mtm.tm_mday != this->m_today) {//new day
        fflush(this->m_fp);
        fclose(this->m_fp);
        this->m_today = mtm.tm_mday;
        snprintf(log_full_name, sizeof(log_full_name), "%s%d_%02d_%02d_%s", this->m_dir_name.c_str(), mtm.tm_year + 1900, mtm.tm_mon + 1, mtm.tm_mday, this->m_log_name.c_str());
        this->m_fp = fopen(log_full_name, "a");
        if (this->m_fp == NULL) {
            throw std::runtime_error("Failed to open log file: " + std::string(log_full_name));
        }
    }
    this->m_mutex.unlock();

    va_list valst;
    va_start(valst, format);
    std::string log_str;

    this->m_mutex.lock();
    int n = sprintf(this->m_buf, "%d-%02d-%02d %02d:%02d:%02d.%d [%s]:", mtm.tm_year + 1900, mtm.tm_mon + 1, mtm.tm_mday, mtm.tm_hour, mtm.tm_min, mtm.tm_sec, now.tv_nsec, level_buf);
    if (n < 0) {
        throw std::runtime_error("format info error!");
    }
    int m = vsnprintf(this->m_buf + n, this->m_buf_size - n - 1, format, valst);
    if (m < 0) {
        throw std::runtime_error("format info error!");
    }
    this->m_buf[n + m] = '\n';
    this->m_buf[n + m + 1] = '\0';
    log_str = this->m_buf;
    this->m_mutex.unlock();
    va_end(valst);

    if (this->m_is_async && !this->m_log_queue->full()) {
        this->m_log_queue->push(log_str);
    }
    else {
        this->m_mutex.lock();
        fputs(log_str.c_str(), this->m_fp);
        this->m_mutex.unlock();
    }
}

void Log::flush() {
    this->m_mutex.lock();
    fflush(this->m_fp);
    this->m_mutex.unlock();
}

void Log::async_write_log() {
    std::string log;
    while (this->m_log_queue->pop(log)) {
        this->m_mutex.lock();
        fputs(log.c_str(), this->m_fp);
        this->m_mutex.unlock();
    }
}

void* Log::flush_log_thread(void*) {
    Log::get_instance()->async_write_log();
}