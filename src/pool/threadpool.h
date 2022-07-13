#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <assert.h>
#include <functional>
#include <condition_variable>

class ThreadPool 
{
public:
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>())
    {
        assert(threadCount > 0);
        for(int i = 0; i < static_cast<int>(threadCount); i++){
            std::thread([pool = pool_] {
                std::unique_lock<std::mutex> locker(pool->mtx);
                while(true) {
                    if(!pool->tasks.empty()) {
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }else if(pool->isClose) break;
                    else pool->cond.wait(locker);
                }
            }).detach();
        }
    }  

    ThreadPool() = delete;
    ThreadPool(ThreadPool&&) = delete;

    ~ThreadPool() 
    {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClose = true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) 
    {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }
private:
    struct Pool 
    {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClose;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
};