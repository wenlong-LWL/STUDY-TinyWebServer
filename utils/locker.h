#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <stdexcept>

class locker {
    public:
        locker() {
            if (pthread_mutex_init(&this->m_mutex, NULL) != 0)
                throw std::runtime_error("pthread_mutex_init error");
        }
        ~locker() {pthread_mutex_destroy(&this->m_mutex);}
        bool lock() {return pthread_mutex_lock(&this->m_mutex) == 0;}
        bool unlock() {return pthread_mutex_unlock(&this->m_mutex) == 0;}
        pthread_mutex_t* get() {return &this->m_mutex;}

    private:
        pthread_mutex_t m_mutex;
};
#endif