#pragma once
#include <cstdint>

class Timer;

class TimerId
{
 public:
 TimerId() = default;
  TimerId()
    : timer_(nullptr),
      sequence_(0)
  {
  }

  TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
      sequence_(seq)
  {
  }
  ~TimerId() = default;

  friend class TimerQueue;

 private:
  Timer* timer_;
  int64_t sequence_;
};



