#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

class Timestamp
{
public:
    using clock = std::chrono::system_clock;
    using time_point = std::chrono::time_point<clock, std::chrono::microseconds>;

    Timestamp() : tp_(){}       // 默认构造函数，初始化为无效时间戳
    explicit Timestamp(int64_t microSecondsSinceEpoch)          // 有参构造函数，传入自纪元以来的微秒数
    : tp_(std::chrono::time_point<clock, std::chrono::microseconds>(std::chrono::microseconds(microSecondsSinceEpoch))){}        

    std::string toString() const    {                           // 将时间戳转化为字符串
    
        auto s = std::chrono::duration_cast<std::chrono::seconds>(tp_.time_since_epoch());
        std::time_t t = s.count();
        std::tm tm_time = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm_time, "%Y/%m/%d %H:%M:%S");
        return oss.str();
    }


    bool valid() const { return tp_.time_since_epoch().count() > 0; }  // 判断时间戳是否有效

    int64_t microSecondsSinceEpoch() const { return tp_.time_since_epoch().count(); }    // 获取微秒数

    static Timestamp now() {     // 获取当前时间戳
        auto now = std::chrono::time_point_cast<std::chrono::microseconds>(clock::now());
        return Timestamp(now.time_since_epoch().count());       
    }
    static Timestamp invalid()  {return Timestamp();}   // 获取无效时间戳
    
    static const int kMicroSecondsPerSecond = 1000 * 1000;  // 1秒 = 1000 * 1000微秒
private:
    time_point tp_;    // 自纪元以来的微秒数
};


inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline Timestamp addTime(Timestamp timestamp, double seconds)   // 给一个时间戳加上指定秒数，返回新的时间戳
{
    auto delta = std::chrono::microseconds(static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond));
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta.count());
}
