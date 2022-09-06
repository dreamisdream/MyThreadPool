// ThreadPool.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <queue>
#include <condition_variable>
#include <stdexcept>
#include <functional>
#include <mutex>
#include <memory>

class MyThreadPool {

public:
    using Task = std::function<void()>;

    // 加入任务
    template <typename F, typename ...Args>
    auto enqueue(F&& f, Args &&...args)
        ->std::future<typename std::result_of<F(Args ...)>::type> {

        using returnType = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared< std::packaged_task<returnType()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

        // 任务的执行结果
        std::future<returnType> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_mtxQueue);

            if (m_stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            m_taskQueue.emplace([task] {(*task)(); });
        }
        m_cv.notify_one();
        return res;

    }
    MyThreadPool(size_t threads = 1) : m_stop(false) {
        std::cout << "线程池启动的线程数为: " << threads << std::endl;
        for (size_t i = 0; i < threads; ++i) {
            m_works.emplace_back([this] {
                for (;;) {
                    Task tempTask;
                    {
                        std::unique_lock<decltype(m_mtxQueue)> lk(m_mtxQueue);
                        m_cv.wait(lk, [this] {return m_stop || !m_taskQueue.empty(); });
                        if (m_stop && m_taskQueue.empty())
                            return;
                        tempTask = std::move(m_taskQueue.front());
                        m_taskQueue.pop();

                    }
                    tempTask();

                }
                }

            );

        }
    }
    virtual ~MyThreadPool() {
        {
            std::unique_lock<decltype(m_mtxQueue)> lk(m_mtxQueue);
            m_stop = true;
        }
        m_cv.notify_all();
        for (auto &work : m_works) {
            if (work.joinable())
                work.join();
        }
    }
private:

    std::vector<std::thread> m_works;
    std::queue<Task> m_taskQueue;
    std::mutex m_mtxQueue;
    std::condition_variable m_cv;
    bool m_stop;
};

#endif
// TODO: 在此处引用程序需要的其他标头。
