#pragma once

#include "Timestamp.h"
#include "Callbacks.h"
#include "noncopyable.h"
#include <atomic>

class Timer : noncopyable
{
public:
  Timer(TimerCallback cb, Timestamp when, double interval)
    : callback_(std::move(cb))
    , expiration_(when)
    , interval_(interval)
    , repeat_(interval > 0.0)
    , sequence_(++s_numCreated_)
    { }

    void run() const
    {
        callback_();    // 定时器到期时执行的回调函数
    }

    Timestamp expiration() const  { return expiration_; }   // 到期时间
    bool repeat() const { return repeat_; }                 // 是否是周期性定时器
    int64_t sequence() const { return sequence_; }          // 序列号，用于区分不同定时器

    void restart(Timestamp now);                            // 重启定时器，周期性定时器到期后重置下次到期时间

    static int64_t numCreated() { return s_numCreated_.load(); }    // 获取已创建定时器的总数

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic<int64_t> s_numCreated_;
};