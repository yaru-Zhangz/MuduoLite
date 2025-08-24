#pragma once

#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <string>
#include <atomic>

#include "noncopyable.h"

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() { return started_; }
    pid_t tid() const { return tid_; }
    const std::string &name() const { return name_; }

    static int numCreated() { return numCreated_; }

private:
    void setDefaultName();                  // 设置默认线程名称

    bool started_;                          // 标记线程是否已启动
    bool joined_;                           // 标记线程是否已 join
    std::shared_ptr<std::thread> thread_;   // 智能指针管理 std::thread 对象
    pid_t tid_;                             // 线程 id，在线程创建后绑定
    ThreadFunc func_;                       // 线程执行的回调函数
    std::string name_;                      // 线程名称
    static std::atomic_int numCreated_;     // 已创建线程总数，原子操作保证线程安全
};