#pragma once

#include <functional> // function
#include <chrono>
// #include <algorithm> // swap
#include <vector>
#include <unordered_map>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimeNode {
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator< (const TimeNode& t) {
        return expires < t.expires;
    }
};

class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64);}
    ~HeapTimer() { clear();}
    void clear();
    void adjust(int id, int newExpires);
    void add(int id, int timeOut, const TimeoutCallBack& cb);
    void doWork(int id);
    void tick();
    void pop();
    int GetNextTick();
private:
    void del(size_t index);
    void siftup(size_t index);
    bool siftdown(size_t index,size_t n);
    void SwapNode(size_t lhs, size_t rhs);
    std::vector<TimeNode> heap_;
    std::unordered_map<int, size_t> ref_;
};