// Copyright 2017, Steven Zhang.  All rights reserved.
//
// Author: Steven Zhang (stevenzscn@gmail.com)
//
// This is a public header file, it must only include public header files.

#ifndef TIMER_TASK_H_
#define TIMER_TASK_H_

#include <thread>
#include <atomic>
#include <chrono>
#include <functional>

class TimerTask {
public:
    TimerTask() : execute_(false)
    {}

    ~TimerTask() {
        if (execute_.load(std::memory_order_acquire))
            stop();
    }

    void stop() {
        execute_.store(false, std::memory_order_release);
        if (thd_.joinable())
            thd_.join();
    }

    void start(int interval_ms, std::function<void(void)> task) {
        if (execute_.load(std::memory_order_acquire)) {
            stop();
        }
        execute_.store(true, std::memory_order_release);
        thd_ = std::thread([this, interval_ms, task]() {
                    while (execute_.load(std::memory_order_acquire)) {
                        task();
                        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
                    }
                });
    }

    bool is_running() const noexcept {
        return (execute_.load(std::memory_order_acquire) &&
                thd_.joinable());
    }

private:
    std::atomic<bool> execute_;
    std::thread thd_;
};

#endif
