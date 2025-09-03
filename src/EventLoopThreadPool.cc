#include <memory>

#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "Logger.h"
EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0)
    , hash_(3)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // Don't delete loop, it's stack variable
}

// 启动线程池，创建 numThreads 个 EventLoopThread，每个线程绑定一个 EventLoop
void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i); // 生成线程名称
        std::string threadName(buf);
        EventLoopThread *t = new EventLoopThread(cb, threadName);   // 创建事件循环线程
        threads_.push_back(std::unique_ptr<EventLoopThread>(t)); // 保存线程对象
        EventLoop* loop = t->startLoop(); // 启动线程并获取 EventLoop 指针
        loops_.push_back(loop);
        hash_.addNode(threadName);               // 将线程名称加入一致性哈希
        threadNameToLoop_[threadName] = loop;    // 记录线程名到EventLoop*的映射
    }

    // 如果线程数为0，仅有主事件循环，且有初始化回调则执行
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

// 根据 key 用一致性哈希分配一个 EventLoop（如分配连接到某个线程）
EventLoop *EventLoopThreadPool::getNextLoop(const std::string &key)
{
    std::string threadName = hash_.getNode(key); // 获取分配的线程名
    auto it = threadNameToLoop_.find(threadName);
    if (it != threadNameToLoop_.end()) {
        return it->second;
    } else {
        LOG_ERROR("EventLoopThreadPool::getNextLoop ERROR");
        return baseLoop_;
    }
}


// 获取所有 EventLoop 指针（如果没有子线程则只返回主事件循环）
std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop *>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}