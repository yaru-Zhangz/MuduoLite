#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>

#include "noncopyable.h"
#include "Thread.h"

class EventLoop;

/*
one loop per thread: 将一个EventLoop和一个Thread封装在一起
*/

class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                    const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startLoop();     // 启动线程并返回线程中的 EventLoop 指针

private:
    void threadFunc(); // 线程主函数，负责创建和运行 EventLoop

    EventLoop *loop_;                  // 指向线程中 EventLoop 实例的指针
    bool exiting_;                     // 标记线程是否正在退出
    Thread thread_;                    // 封装的线程对象
    std::mutex mutex_;                 // 互斥锁，保护 loop_ 指针和条件变量
    std::condition_variable cond_;     // 条件变量，用于主线程等待 EventLoop 创建完成
    ThreadInitCallback callback_;      // 线程初始化回调函数
};