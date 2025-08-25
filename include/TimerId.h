#pragma once
#include <cstdint>

class Timer;
// 用于唯一标识一个定时器实例
class TimerId
{
public:
    TimerId()
        : timer_(nullptr)
        , sequence_(0)
    {
    }

    TimerId(Timer* timer, int64_t seq)
        : timer_(timer)
        , sequence_(seq)
    {
    }
    ~TimerId() = default;

    friend class TimerQueue;

private:
    Timer* timer_;          // 指向实际定时器对象的指针
    int64_t sequence_;      // 定时器的唯一序号
};



