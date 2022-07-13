#include "Epoller.h"

Epoller::Epoller(int maxEvent): epollFd_(epoll_create(maxEvent)), events_(maxEvent)
{
    assert(epollFd_ > 0 && events_.size() > 0);
}

int Epoller::GetEventFd(size_t i) 
{
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

uint32_t Epoller::GetEvents(size_t i) 
{
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}

int Epoller::wait(int timeMS) 
{
    return epoll_wait(epollFd_, &events_[0], events_.size(), timeMS);
}

int Epoller::AddFd(int fd, uint32_t events) 
{
    if(fd < 0) { return false; }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
}

int Epoller::ModFd(int fd, uint32_t events) 
{
    if(fd < 0) { return false; }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
}

int Epoller::DelFd(int fd) 
{
    if(fd < 0) { return false; }
    epoll_event ev = {0};
    return epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
}

