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
        }

        ~block_queue() {
            if (this->m_array)
                delete[] this->m_array;
        }

        T* front() {}
        T* back() {}
        T* pop() {}
        T* push() {}
        bool empty() { return this->size() == 0; }
        void clear() {
            this->m_mutex.lock();
            this->size = 0;
            this->m_front_index = -1;
            this->m_back_index = -1;
            this->m_mutex.unlock();
        }
        size_t size() { return this->m_size; }

    private:
        int m_size;
        int m_max_size;
        int m_front_index;
        int m_back_index;
        T* m_array;
        locker m_mutex;
};

#endif