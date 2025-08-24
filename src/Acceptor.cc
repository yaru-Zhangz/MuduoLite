#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

// 创建一个非阻塞、自动关闭的 TCP socket
static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

// 初始化监听 socket、事件通道，并设置回调
Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonblocking())
    , acceptChannel_(loop, acceptSocket_.fd())
    , listenning_(false)
{
    acceptSocket_.setReuseAddr(true);      // 启用地址复用
    acceptSocket_.setReusePort(true);      // 启用端口复用
    acceptSocket_.bindAddress(listenAddr); // 绑定监听地址
    // 设置监听 socket 的读事件回调为 handleRead
    acceptChannel_.setReadCallback(
        std::bind(&Acceptor::handleRead, this));
}

// 移除事件监听并释放资源
Acceptor::~Acceptor()
{
    acceptChannel_.disableAll(); // 移除所有事件监听
    acceptChannel_.remove();     // 从事件循环中移除通道
}

// 启动监听，注册读事件到 Poller
void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();         // 启动 socket 监听
    acceptChannel_.enableReading(); // 注册读事件到 Poller
}

// 处理监听 socket 上的读事件（即有新连接到来）
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr); // 接收新连接
    if (connfd >= 0)
    {
        if (NewConnectionCallback_)
        {
            // 调用用户设置的新连接回调，传递新连接 fd 和对端地址
            NewConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            // 未设置回调时可做日志或关闭连接等处理
            ::close(connfd);
        }
    }
    else
    {
        // 接收连接失败时记录错误
        LOG_ERROR("Acceptor::handleRead accept error:%d\n", errno);
    }
}