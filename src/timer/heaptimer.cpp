#include "heaptimer.h"

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

void HeapTimer::pop(){
    assert(!heap_.empty());
    del(0);
}

void HeapTimer::SwapNode(size_t lhs, size_t rhs) {
    assert(lhs>=0&&lhs<heap_.size());
    assert(lhs>=0&&lhs<heap_.size());
    std::swap(heap_[lhs],heap_[rhs]);
    ref_[heap_[lhs].id] = lhs;
    ref_[heap_[rhs].id] = rhs;
}

void HeapTimer::siftup(size_t i) {
    assert(i>=0 && i < heap_.size());
    size_t j = (i - 1) / 2;
    while(j >= 0){
        if(heap_[j] < heap_[i]) { break;}
        SwapNode(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

bool HeapTimer::siftdown(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while(j < n) {
        if(j + 1 < n && heap_[j] < heap_[j+1]) j++;
        if(heap_[i] < heap_[j]) { break;}
        SwapNode(i, j);
        i = j;
        j = (i * 2 + 1);
    }
    return i > index;
}

void HeapTimer::del(size_t index) {
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    size_t i = index;
    size_t n = heap_.size()-1;
    assert(i <= n);
    if(i < n){
        SwapNode(i, n);
        if(!siftdown(i, n)){
            siftup(i);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
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

void HeapTimer::doWork(int id) {
    if(heap_.empty() || ref_.count(id) == 0){ return;}
    size_t i = ref_[id];
    TimeNode node = heap_[i];
    node.cb();
    del(i);
}

void HeapTimer::adjust(int id, int timeout) {
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);
    siftdown(ref_[id], heap_.size());
}

void HeapTimer::tick() {
    if(heap_.empty()) { return;}
    while(!heap_.empty()) {
        TimeNode node = heap_.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break;
        }
        node.cb();
        pop();
    }
}

int HeapTimer::GetNextTick() {
    tick();
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0;}
    }
    return res;
}