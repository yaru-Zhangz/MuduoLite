#pragma once

#include <memory>
#include <string>
#include <atomic>

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

class Channel;
class EventLoop;
class Socket;

/**
 * TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
 * => TcpConnection设置回调 => 设置到Channel => Poller => Channel回调
 **/

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,
                  const std::string &nameArg,
                  int sockfd,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; } // 获取所属事件循环
    const std::string &name() const { return name_; } // 获取连接名称
    const InetAddress &localAddress() const { return localAddr_; } // 获取本地地址
    const InetAddress &peerAddress() const { return peerAddr_; }   // 获取对端地址

    bool connected() const { return state_ == kConnected; }

    // 发送数据
    void send(const std::string &buf);
    void sendFile(int fileDescriptor, off_t offset, size_t count); 
    
    // 关闭半连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    void connectEstablished();
    void connectDestroyed();

private:
    enum StateE
    {
        kDisconnected, // 已经断开连接
        kConnecting,   // 正在连接
        kConnected,    // 已连接
        kDisconnecting // 正在断开连接
    };
    void setState(StateE state) { state_ = state; }

    // 事件处理函数
    void handleRead(Timestamp receiveTime); // 处理读事件
    void handleWrite();                     // 处理写事件
    void handleClose();                     // 处理关闭事件
    void handleError();                     // 处理错误事件

    // 内部发送和关闭操作（在事件循环线程中执行）
    void sendInLoop(const void *data, size_t len);
    void shutdownInLoop();
    void sendFileInLoop(int fileDescriptor, off_t offset, size_t count);

    EventLoop *loop_;           // 所属事件循环对象指针
    const std::string name_;    // 连接名称
    std::atomic_int state_;     // 连接状态，原子变量保证线程安全
    bool reading_;              // 是否监听读事件

    std::unique_ptr<Socket> socket_;   // 封装的 socket 对象
    std::unique_ptr<Channel> channel_; // 封装的事件通道对象

    const InetAddress localAddr_; // 本地地址
    const InetAddress peerAddr_;  // 对端地址

    // 各种事件回调函数
    ConnectionCallback connectionCallback_;       // 新连接建立时回调
    MessageCallback messageCallback_;             // 有消息到达时回调
    WriteCompleteCallback writeCompleteCallback_; // 写完成时回调
    HighWaterMarkCallback highWaterMarkCallback_; // 高水位回调
    CloseCallback closeCallback_;                 // 连接关闭时回调
    size_t highWaterMark_;                        // 高水位阈值

    Buffer inputBuffer_;    // 接收数据缓冲区
    Buffer outputBuffer_;   // 发送数据缓冲区
};
