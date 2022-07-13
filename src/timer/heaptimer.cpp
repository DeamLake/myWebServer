#include "heaptimer.h"

HeapTimer::HeapTimer() 
{
    heap_.reserve(64);
    // 0th node just take place
    heap_.push_back(TimeNode()); 
}

HeapTimer::~HeapTimer() 
{
    ref_.clear();
    heap_.clear();
}

void HeapTimer::pop()
{
    assert(!heap_.empty());
    del(1); 
}

void HeapTimer::SwapNode(size_t lhs, size_t rhs) 
{
    assert(lhs > 0 && lhs < heap_.size());
    assert(lhs > 0 && lhs < heap_.size());
    std::swap(heap_[lhs],heap_[rhs]);
    ref_[heap_[lhs].id] = lhs;
    ref_[heap_[rhs].id] = rhs;
}

void HeapTimer::siftup(size_t i) 
{
    assert(i > 0 && i < heap_.size());
    size_t parent = i / 2;
    while(parent > 0)
    {
        if(heap_[parent] < heap_[i]) { break;}
        SwapNode(i, parent);
        i = parent;
        parent = i / 2;
    }
}

bool HeapTimer::siftdown(size_t index, size_t n) 
{
    assert(index > 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t son = i * 2;
    while(son < n) 
    {
        if(son + 1 < n && heap_[son+1] < heap_[son]) son++;
        if(heap_[i] < heap_[son]) { break;}
        SwapNode(i, son);
        i = son;
        son = i * 2;
    }
    return i > index;
}

void HeapTimer::del(size_t index) {
    assert(index > 0 && index < heap_.size());
    size_t i = index;
    size_t n = heap_.size()-1;
    if(i < n)
    {
        SwapNode(i, n);
        if(!siftdown(i, n)){
            siftup(i);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) 
{
    assert(id > 0);
    if(ref_.count(id) == 0) {
        size_t i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        siftup(i);
    }else {
        size_t i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if(!siftdown(i, heap_.size())) {
            siftup(i);
        }
    }
}

void HeapTimer::doWork(int id) 
{
    if(heap_.size() == 1 || ref_.count(id) == 0){ return;}
    size_t i = ref_[id];
    TimeNode node = heap_[i];
    node.cb();
    del(i);
}

void HeapTimer::adjust(int id, int timeout) 
{
    assert(heap_.size() > 1 && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);
    siftdown(ref_[id], heap_.size());
}

void HeapTimer::tick() 
{
    if(heap_.size() == 1) { return;}
    while(heap_.size() > 1) 
    {
        TimeNode node = heap_[1];
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break;
        }
        node.cb();
        pop();
    }
}

int HeapTimer::GetNextTick() 
{
    tick();
    size_t res = -1;
    if(heap_.size() > 1) {
        res = std::chrono::duration_cast<MS>(heap_[1].expires - Clock::now()).count();
        if(res < 0) { res = 0;}
    }
    return res;
}