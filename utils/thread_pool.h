#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <list>
#include <locker.h>

template <typename T>
class ThreadPool {
private:
    class Job {
    public:
        void* (*m_func)(void*);
        void *m_args;
    };
public:
    ThreadPool();
    ~ThreadPool();
    void addJob();

private:
    static void* worker(void*);
    void* run(void*);

private:
    std::list<T*> m_work_list;
    locker m_mutex;
    sem_t m_sem_t;
    int m_thread_num;
    int m_max_requests;
    pthread_t *m_threads;

};
#endif