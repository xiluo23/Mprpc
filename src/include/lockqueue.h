#pragma once 
#include<queue>
#include<mutex>
#include<condition_variable>
#include<thread>

template<typename T>
class LockQueue{
public:
    void Push(T&data){
        {
            std::lock_guard<std::mutex>lock(m_mutex);
            m_queue.push(data);
            m_cv.notify_one();
        }        
    }
    T Pop(){
        std::unique_lock<std::mutex>lock(m_mutex);
        while(m_queue.empty()){
            m_cv.wait(lock);
        }
        T data=m_queue.front();
        m_queue.pop();
        return data;
    }
private:
    std::queue<T>m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};