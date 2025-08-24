#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Channel;
class Poller;

// 事件循环类 主要包含了两个大模块 Channel Poller(epoll的抽象)
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    using ChannelList = std::vector<Channel*>;

    EventLoop();
    ~EventLoop();

    void loop();        // 开启事件循环
    void quit();        // 退出事件循环

    Timestamp pollReturnTime() const { return pollRetureTime_; }    // 获取最近一次事件检测的时间戳


    void runInLoop(Functor cb);     // 在当前时间循环线程中立即执行回调
    void queueInLoop(Functor cb);   // 将回调函数加入队列，稍后在事件循环线程中执行

    void wakeup();  // 通过eventfd唤醒事件循环线程，防止长时间阻塞

    void updateChannel(Channel *channel); // 更新 Channel 的关注事件
    void removeChannel(Channel *channel); // 移除 Channel，不再监听
    bool hasChannel(Channel *channel);    // 检查 Channel 是否已被管理

    // 判断当前代码是否运行在事件循环所属线程，保证线程安全
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
        abortNotInLoopThread();
        }
    }
private:
    void abortNotInLoopThread();
    void handleRead();        // 处理 eventfd 读事件，响应唤醒操作
    void doPendingFunctors(); // 执行队列中的所有待处理回调

    std::atomic_bool looping_; // 标记事件循环是否正在运行
    std::atomic_bool quit_;    // 标记是否请求退出事件循环

    const pid_t threadId_; // 记录事件循环所属线程的 id

    Timestamp pollRetureTime_; // 记录最近一次 Poller 检测事件的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_; // 作用：当mainLoop获取一个新用户的Channel 需通过轮询算法选择一个subLoop 通过该成员唤醒subLoop处理Channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_; // 返回Poller检测到当前有事件发生的所有Channel列表

    std::atomic_bool callingPendingFunctors_; // 标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;    // 存储loop需要执行的所有回调操作
    std::mutex mutex_;                        // 互斥锁 用来保护上面vector容器的线程安全操作
};