#pragma once


#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>


// A simple unbounded thread-safe queue
// Too simple i d like to implement a lock-free queue but have no enough time :(((
template <typename T>
class ThreadSafeQueue 
{
public:
    void push(T value) 
    {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_q.push(std::move(value));
        }
        m_cv.notify_one();
    }

    // Returns false if stopped and no item retrieved
    // This awaiting approach is necessary to avoid 100% busy loop in a queue user thread
    bool waitPop(T& out) 
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cv.wait(lk, [&] { 
                return m_stop || m_q.empty() == false; 
            });
        if(m_stop && m_q.empty() == true)
        {
            return false;
        }

        out = std::move(m_q.front());
        m_q.pop();
        
        return true;
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_stop = true;
        }
        m_cv.notify_all();
    }

    bool empty() const 
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_q.empty();
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<T> m_q;
    bool m_stop{false};
};
