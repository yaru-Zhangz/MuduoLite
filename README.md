# muduo-lite

## 项目介绍

本项目采用多Reactor模型，其中主Reactor负责监听和派发新连接，多个从reactor负责连接后的I/O事件处理。调用TcpServer的start函数后，内部会创建线程池，每个线程独立的运行一个事件循环，也就是一个reactor，当主reactor监听到一个连接事件，会从线程池中通过一致性哈希算法获取一个从reactor并将这个新连接派发给它，以后这个从reactor就负责这个连接的I/O事件。通过这个架构，可以高效处理大量并发的TCP连接。

项目已经实现了 事件轮询和分发模块（包括 Channel 模块、Poller 模块、EventLoop模块）、线程池模块、网络连接模块、缓冲区模块、定时器模块。

## 开发环境

* 操作系统：CentOS Linux 8
* Linux 内核版本：4.18.0-348.7.1.el8_5.x86_64
* gcc 版本：13.2.0
* cmake 版本：3.26.4

## 并发模型

使用主从 Reactor 模型有诸多优点：

1. 响应快，不必为单个同步事件所阻塞，虽然 Reactor 本身依然是同步的；
2. 可以最大程度避免复杂的多线程及同步问题，并且避免多线程/进程的切换；
3. 扩展性好，可以方便通过增加 Reactor 实例个数充分利用 CPU 资源；
4. 复用性好，Reactor 模型本身与具体事件处理逻辑无关，具有很高的复用性；

## 构建项目

安装基本工具

```shell
sudo yum makecache
sudo yum install -y wget cmake gcc-c++ make unzip git
```

## 编译指令

进入到muduo-lite文件
```shell
cd muduo-lite
```

创建build文件夹，并且进入build文件:
```shell
mkdir build && cd build
```

然后生成可执行程序文件：
```shell
cmake .. && make -j${nproc}
```

运行程序，进入example文件夹，并且执行可执行程序
```shell
cd example  &&  ./testserver
```

## 功能介绍

#### 事件轮询与分发模块

EventLoop负责事件循环和分发，Poller负责底层事件监测和Channel管理，Channel负责封装fd和感兴趣的事件，并与回调进行绑定。

- **EventLoop、Channel 和 Poller 之间的关系：**

1. `one loop per thread`: 一个线程对应一个事件循环
2. 一个事件循环持有一个 `Poller` 和多个 `Channel`，通过不断循环调用 `Poller` 的 `poll` 方法，获取活跃的Channel，并分发事件
3. 一个`Poller`对应一个`epoll`实例，里面封装了 `epoll/poll/select` 等底层 IO 复用机制，负责管理所有注册的   `Channel`，监听它们的事件，当有事件发生时，将活跃的`Channel`返回给`EventLoop`，由后者分发处理。
4. 一个`Channel`封装一个`fd`以及关注的事件类型，负责绑定各种事件的回调函数。

#### 线程池模块

线程池模块完成线程与事件循环的绑定，每个线程独立运行一个事件循环，可以支持设置线程数量，启动所有线程，通过一致性哈希算法分配新连接给一个事件循环。线程池模块主要通过 `Thread.*`、`EventLoopThread.*` 和 `EventLoopThreadPool.*` 实现，具体实现思路如下：

- Thread类对线程进行了封装，支持传入任意可调用对象作为线程入口，线程启动后会执行用户指定的回调函数
- EventLoopThread负责封装一个事件循环和一个线程，实现one loop per thread
- EventLoopThreadPool负责统一管理多个EventLoopThread

调用TcpServer的start函数后，内部就会创建线程池，通过设置的线程数创建并启动多个事件循环线程，之后主线程只负责监听新连接，

#### 网络连接模块

1. `TcpServer` 创建 `Acceptor`，监听端口。
2. `Acceptor` 监听到新连接后，accept 并回调给 `TcpServer`。
3. `TcpServer` 通过线程池选择一个 EventLoop，创建 `TcpConnection`，并将其注册到对应的EventLoop。
4. `TcpConnection` 负责后续的读写事件、数据收发、回调触发和连接管理。
5. `Socket` 类为上述各模块提供底层 socket 操作支持。

#### 缓冲区模块

`Buffer`内部通常用一个`std::vector<char>`作为底层存储，维护两个索引：`readIndex_`（读指针）和`writeIndex_`（写指针）。数据区分为三部分：

- `[0, readIndex_)`：已读数据
- `[readIndex_, writeIndex_)`：可读数据
- `[writeIndex_, capacity)`：可写空间

当有新数据到达时，Buffer会将数据直接写入`writeIndex_`之后的空间，并更新`writeIndex_`。用户读取数据时，从`readIndex_`开始读取，读取后移动`readIndex_`。支持`peek`（只读不取走）、`retrieve`（取走数据）、`retrieveAll`（清空缓冲区）等操作。如果写入数据时空间不足，`Buffer`会先尝试将未读数据前移，如果仍不足则自动扩容`std::vector`。发送数据时，`Buffer`负责将可读区的数据分多次写入`socket`，直到全部发送完毕或内核缓冲区满。如果一次没发完，剩余数据会保留在`Buffer`，等待下次可写事件再继续发送。

#### 定时器模块

主要由 Timer.\*、TimerId.\*、TimerQueue.\*和 Timestamp.\* 组成。

`Timer`表示单个定时任务，`TimerId`用于唯一标识一个定时器，`TimerQueue`管理所有定时器对象，当`timerfd`到期时，`TimerQueue`负责读取`timerfd`，批量取出所有到期定时器，依次执行回调，并对周期性定时器重启。

## 参考资料
- muduo网络库源码：https://github.com/chenshuo/muduo
- 一致性哈希算法：https://www.bilibili.com/video/BV1FJ4m1e7Sz/?spm_id_from=333.337.search-card.all.click&vd_source=5eae1b2580d836bc51a9f4cb2fb7ad10
- 