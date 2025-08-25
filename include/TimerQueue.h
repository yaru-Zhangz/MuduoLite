#pragma once

#include <set>
#include <vector>
#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

class EventLoop;
class Timer;
class TimerId;

// TimerQueue负责管理所有定时器，基于定时器文件描述符实现高效定时任务调度
class TimerQueue : noncopyable
{
public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  TimerId addTimer(TimerCallback cb, Timestamp when, double interval);  // 添加定时器
  void cancel(TimerId timerId);     // 取消定时器

private:
  // 用unique_ptr<Timer>管理定时器对象，提升异常安全
  using Entry = std::pair<Timestamp, Timer*>;
  using TimerList = std::set<Entry>;
  using ActiveTimer = std::pair<Timer*, int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  void addTimerInLoop(Timer* timer);
  void cancelInLoop(TimerId timerId);
  void handleRead();
  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);
  bool insert(Timer* timer);

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  TimerList timers_;

  // for cancel()
  ActiveTimerSet activeTimers_;
  bool callingExpiredTimers_;
  ActiveTimerSet cancelingTimers_;
};

