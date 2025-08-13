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

/*

typedef union epoll_data {
    void        *ptr;    // 用户自定义指针，常用于存放对象指针（如 Channel*）
    int         fd;      // 文件描述符
    uint32_t    u32;     // 32位无符号整数
    uint64_t    u64;     // 64位无符号整数
} epoll_data_t;

struct epoll_event {
    uint32_t     events; // 事件类型掩码，如 EPOLLIN、EPOLLOUT、EPOLLERR 等
    epoll_data_t data;   // 用户数据，可以存放 fd、指针等
};

注册事件时，将events设置为关注的事件类型，将data.ptr设置为Channel指针
事件发生时，epoll_wait返回的epoll_event数组中，每个元素的events表示发生的事件类型
*/