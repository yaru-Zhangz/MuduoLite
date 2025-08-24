#pragma once

#include <functional>

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class InetAddress;

/*
acceptor专门负责监听服务器段的监听socket，当有新的客户端连接到来时，负责接收连接并将新连接的文件描述符
和对端地址传递给上层
*/
class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb) { NewConnectionCallback_ = cb; }
    bool listenning() const { return listenning_; } // 查询是否正在监听
    void listen(); // 启动监听


private:
    void handleRead(); // 处理新连接到来的事件

    EventLoop *loop_;           // 事件循环对象指针
    Socket acceptSocket_;       // 用于监听新连接的 socket
    Channel acceptChannel_;     // 用于监听 socket 上的事件
    NewConnectionCallback NewConnectionCallback_; // 新连接到来时的回调函数
    bool listenning_;           // 标记是否正在监听
};