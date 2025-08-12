#pragma once

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "Timestamp.h"

class EventLoop;

/*
Channel的作用：
    1. 记录fd及其关注的事件类型
    2. 绑定并管理各种事件的回调函数
    3. 在事件发生时分发并执行对应的回调
    4. 作为EventLoop和Poller之间的桥梁
*/
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // 根据事件发生的类型调用对应的回调
    void handleEvent(Timestamp receiveTime);

    // 设置回调
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止回调过程中对象被销毁
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }

    // 设置fd相应的事件状态
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }              // 获取在 Poller 中的索引
    void set_index(int idx) { index_ = idx; }   // 设置在 Poller 中的索引

    EventLoop *ownerLoop() { return loop_; }    // 获取所属的事件循环对象
    void remove();                              // 从事件循环中移除当前通道
    
private:

    void update();                                    // 更新 Channel 关注的事件到 Poller
    void handleEventWithGuard(Timestamp receiveTime); // 事件处理保护，确保对象有效

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;              // 所属事件循环对象指针
    const int fd_;                 // 文件描述符，Poller 监听的对象
    int events_;                   // 当前关注的事件类型
    int revents_;                  // 实际发生的事件类型，由 Poller 设置
    int index_;                    // 在 Poller 中的索引

    std::weak_ptr<void> tie_;      // 绑定的对象，防止回调时对象被销毁
    bool tied_;                    // 标记是否已绑定对象

    // 因为channel通道里可获知fd最终发生的具体的事件events，所以它负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};