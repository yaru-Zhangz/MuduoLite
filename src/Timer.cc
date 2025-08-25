#include "Timer.h"



std::atomic<int64_t> Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{
    if (repeat_)
    {
        expiration_ = addTime(now, interval_);  // 如果是周期性定时器，重置定时器为当前时间 + 触发的间隔
    }
    else
    {
        expiration_ = Timestamp::invalid();    // 一次性定时器重置为无效时间
    }
}
