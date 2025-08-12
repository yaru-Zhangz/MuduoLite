#pragma once

#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

/*
Poller 负责监听所有注册的文件描述，并在有事件发生时收集活跃的 Channel，通知 EventLoop 进行分发和处理。
Poller 维护 fd 到 Channel 的映射关系，负责 Channel 的注册、更新和移除，确保时间能否被正确分发到对应的回调
*/
class Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    bool hasChannel(Channel *channel) const;

    // 工厂方法，创建默认的 IO 复用器实现（如 EpollPoller）
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; 
};