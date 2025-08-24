#pragma once

#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;

    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }     
    static const int kMicroSecondsPerSecond = 1000 * 1000;
    static Timestamp invalid()
    {
        return Timestamp();
    }
    bool valid() const { return microSecondsSinceEpoch_ > 0; }
private:
    int64_t microSecondsSinceEpoch_;
};

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}
