#pragma once

#include <vector>
#include <sys/epoll.h>

#include "Poller.h"
#include "Timestamp.h"

class Channel;

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    // 重写基类Poller的抽象方法
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;   // 初始化 epoll 事件列表的容量

    // 将 epoll_wait 检测到的活跃事件填充到 activeChannels 列表
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    // 封装 epoll_ctl 操作，更新 Channel 的事件类型
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>; // 定义 epoll_event 的集合类型

    int epollfd_;      // epoll 实例的文件描述符，由 epoll_create 创建
    EventList events_; // 存储 epoll_wait 检测到的所有事件
};