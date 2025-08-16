#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <stdexcept>
#include <locker.h>

template <typename T>
class block_queue {
public:
    block_queue(int max_size = 1000) {
        if (max_size <= 0) {
            throw std::runtime_error("block queue max size is illegal");
        }
        this->m_max_size = max_size;
        this->m_size = 0;
        this->m_front_index = -1;
        this->m_front_index = -1;
        this->m_array = new T[this->m_max_size];
        pthread_cond_init(&m_cond, NULL);
    }

    ~block_queue() {
        this->m_mutex.lock();
        if (this->m_array)
            delete[] this->m_array;
        this->m_mutex.unlock();
        pthread_cond_destroy(&m_cond);
    }

    bool front(T& elem) {
        m_mutex.lock();
        if (m_size == 0) {
            m_mutex.unlock();
            return false;
        }
        elem = m_array[m_front_index];
        m_mutex.unlock();
        return true;
    }
    bool back(T& elem) {
        m_mutex.lock();
        if (m_size == 0) {
            m_mutex.unlock();
            return false;
        }
        elem = m_array[m_back_index];
        m_mutex.unlock();
        return true;
    }
    bool push(const T& elem) {
        m_mutex.lock();
        if (m_size >= m_max_size) {

            pthread_cond_broadcast(&m_cond);
            m_mutex.unlock();
            return false;
        }
        ++m_size;
        m_back_index = (m_back_index + 1) % m_max_size;
        m_array[m_back_index] = elem;

        pthread_cond_broadcast(&m_cond);
        m_mutex.unlock();
        return true;
    }
    bool pop(T& elem) {
        m_mutex.lock();
        while (m_size <= 0) {
            if (pthread_cond_wait(&m_cond, m_mutex.get()) != 0) {
                m_mutex.unlock();
                return false;
            }
        }
        m_front_index = (m_front_index + 1) % m_max_size;
        elem = m_array[m_front_index];
        --m_size;
        m_mutex.unlock();
        return true;
    }

    bool empty() {
        m_mutex.lock();
        if (this->m_size == 0) {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }
    bool full() {
        m_mutex.lock();
        if (m_size >= m_max_size) {
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }
    void clear() {
        this->m_mutex.lock();
        this->size = 0;
        this->m_front_index = -1;
        this->m_back_index = -1;
        this->m_mutex.unlock();
    }
    size_t size() {
        size_t temp = 0;
        m_mutex.lock();
        temp = this->m_size;
        m_mutex.unlock();
        return temp;
    }

private:
    int m_size;
    int m_max_size;
    int m_front_index;
    int m_back_index;
    T* m_array;
    locker m_mutex;
    pthread_cond_t m_cond;
};

#endif