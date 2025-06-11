#pragma once

//AI 写的线程池

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <type_traits>

namespace Touch {

class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;
    
    size_t get_thread_count() const { return workers.size(); }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

template <typename... Args>
void set_promise_value(std::promise<void>& promise, Args&&...) {
    promise.set_value_at_thread_exit();
}

template <typename T, typename... Args>
void set_promise_value(std::promise<T>& promise, T&& value, Args&&...) {
    promise.set_value_at_thread_exit(std::forward<T>(value));
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::invoke_result<F, Args...>::type> {
    
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> res = task->get_future();
    
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        if (stop) {
            std::promise<return_type> p;
            
            if constexpr (std::is_void_v<return_type>) {
                p.set_value_at_thread_exit();
            } else {
                p.set_value_at_thread_exit(return_type());
            }
            
            return p.get_future();
        }

        tasks.emplace([task]() { (*task)(); });
    }
    
    condition.notify_one();
    return res;
}

}