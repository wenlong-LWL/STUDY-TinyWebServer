#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <list>
#include <stdexcept>
#include <semaphore.h>
#include <cstring>
#include <locker.h>

class ThreadPool {
private:
    class Job {
    public:
        void* (*m_func)(void*);
        void* m_args;
    public:
        ~Job() { if (m_args) delete[] m_args; }
    };
public:
    ThreadPool(int thread_num, int max_requests) {
        if (thread_num <= 0) {
            throw std::runtime_error("thread_num wrong");
        }
        if (max_requests <= 0) {
            throw std::runtime_error("max_requests wrong");
        }
        m_thread_num = thread_num;
        m_max_requests = max_requests;
        m_stop = false;
        pthread_cond_init(&m_cond_t, NULL);

        m_threads = new pthread_t[thread_num];
        if (m_threads == NULL) {
            throw std::runtime_error("new pthread_t[] wrong");
        }

        for (int i = 0; i < thread_num; ++i) {
            if (pthread_create(&m_threads[i], NULL, worker, this) != 0) {
                delete[] m_threads;
                throw std::runtime_error("pthread pool create thread wrong");
            }
            //if (pthread_detach(m_threads[i]) != 0) {
            //    delete[] m_threads;
            //    throw std::runtime_error("pthread pool pthread detach wrong");
            //}
        }
    }
    ~ThreadPool() {
        m_stop = true;
        pthread_cond_broadcast(&m_cond_t);
        for (size_t i = 0; i < m_thread_num; ++i) {
            pthread_join(m_threads[i], NULL);
        }
        pthread_cond_destroy(&m_cond_t);
        if (m_threads)
            delete[] m_threads;
    }
    bool addJob(void* (*func)(void*), void* args, int args_len) {
        Job* work_job = new Job;
        if (work_job == NULL) {
            throw std::exception();
            return false;
        }
        work_job->m_func = func;
        work_job->m_args = new char[args_len];
        if (work_job->m_args == NULL) {
            throw std::exception();
            return false;
        }
        memcpy(work_job->m_args, args, args_len);
        return pushJob(work_job);
    }

private:
    static void* worker(void* args) {
        ThreadPool* pool = (ThreadPool*)args;
        pool->run();
        return pool;
    }

    void* run() {
        while (true) {
            m_mutex.lock();
            while (m_work_list.size() == 0) {
                if (m_stop) {
                    m_mutex.unlock();
                    return NULL;
                }
                if (pthread_cond_wait(&m_cond_t, m_mutex.get()) != 0) {
                    m_mutex.unlock();
                    continue;;
                }
            }
            Job* work_job = m_work_list.front();
            m_work_list.pop_front();
            m_mutex.unlock();
            work_job->m_func(work_job->m_args);
            delete work_job;
        }
        return NULL;
    }
    bool pushJob(Job* task) {
        m_mutex.lock();
        if (m_work_list.size() >= m_max_requests) {
            m_mutex.unlock();
            return false;
        }
        m_work_list.push_back(task);
        m_mutex.unlock();
        pthread_cond_signal(&m_cond_t);
        return true;
    }

private:
    std::list<Job*> m_work_list;
    locker m_mutex;
    pthread_cond_t m_cond_t;
    pthread_t* m_threads;
    int m_thread_num;
    int m_max_requests;
    bool m_stop;
};

#endif