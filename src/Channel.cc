#include <sys/epoll.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

// 定义 Channel 支持的事件类型常量
const int Channel::kNoneEvent = 0; // 空事件
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; // 读事件
const int Channel::kWriteEvent = EPOLLOUT; // 写事件

// 初始化 Channel 对象，绑定所属事件循环和文件描述符，并初始化成员变量
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1)
    , tied_(false) {
}

Channel::~Channel() {
}


// 绑定一个智能指针对象，用于保护 Channel 生命周期，防止回调过程中被提前销毁
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;    // 保存弱引用指针
    tied_ = true;  // 标记已绑定保护对象
}

// 通知 Poller 更新当前 Channel 关注的事件类型
void Channel::update()
{
    loop_->updateChannel(this);     // 通过所属事件循环对象，调用 Poller 的更新方法
}

void Channel::remove()
{
    loop_->removeChannel(this);
}

// 处理事件分发入口，根据是否绑定了生命周期保护对象决定是否执行回调
void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_) // 如果已绑定保护对象
    {
        std::shared_ptr<void> guard = tie_.lock(); // 尝试提升为强引用，确保对象未被销毁
        if (guard)
        {
            handleEventWithGuard(receiveTime); // 对象有效时，执行事件分发
        }
        // 对象已被销毁时，不做任何处理，防止访问非法内存
    }
    else
    {
        handleEventWithGuard(receiveTime); // 未绑定保护对象时，直接执行事件分发
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d\n", revents_);
    // 关闭
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) // 当TcpConnection对应Channel 通过shutdown 关闭写端 epoll触发EPOLLHUP
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }
    // 错误
    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
        {
            errorCallback_();
        }
    }
    // 读
    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }
    // 写
    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}