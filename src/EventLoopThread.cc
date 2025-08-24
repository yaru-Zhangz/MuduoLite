#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const std::string &name)
    : loop_(nullptr)
    , exiting_(false)
    , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    , mutex_()
    , cond_()
    , callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

// 启动线程，等待 EventLoop 创建完成并返回其指针
EventLoop *EventLoopThread::startLoop()
{
    thread_.start(); // 启动底层线程

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this](){return loop_ != nullptr;});
        loop = loop_;
    }
    return loop;
}

// 线程主函数，在新线程中创建和运行 EventLoop
void EventLoopThread::threadFunc()
{
    EventLoop loop; // 创建独立的 EventLoop 实例

    if (callback_)
    {
        callback_(&loop); // 如果有初始化回调，先执行
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;           // 设置 loop_ 指针，通知主线程 EventLoop 已创建
        cond_.notify_one();
    }
    loop.loop();    // 启动事件循环，进入 Poller 的 poll 流程

    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr; // 事件循环结束后，清空 loop_ 指针
}