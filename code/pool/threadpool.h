/*
 * @Author: paoluo
 * @Date: 2022-03-09 22:38:23
 * @LastEditors: paoluo
 * @LastEditTime: 2022-08-17 17:45:30
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
        assert(threadCount > 0);

        // 创建 threadCount个子线程
        for(size_t i = 0; i < threadCount; i++) {
            std::thread([pool = pool_] {
                std::unique_lock<std::mutex> locker(pool->mtx);
                while(true) {
                    if(!pool->tasks.empty()) {
                        // 从队列中取出一个任务
                        auto task = std::move(pool->tasks.front());
                        // 移除掉该任务
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    } 
                    else if(pool->isClosed) break;
                    else pool->cond.wait(locker); //阻塞，等待locker解锁
                }
            }).detach(); // 线程分离
        }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }

private:
    // 结构体
    struct Pool {
        std::mutex mtx;                 // 互斥锁
        std::condition_variable cond;   // 条件变量
        bool isClosed;                  // 是否关闭
        std::queue<std::function<void()>> tasks;    // 队列（保存的任务）
    };
    std::shared_ptr<Pool> pool_;        // 池子
};


#endif //THREADPOOL_H