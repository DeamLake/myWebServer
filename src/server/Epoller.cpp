#include "Epoller.h"

Epoller::Epoller(int maxEvent): epollFd_(epoll_create(maxEvent)), events_(maxEvent){
    assert(epollFd_ > 0 && events_.size() > 0);
}

Epoller::~Epoller(){
    close(epollFd_);
}