#include <sys/epoll.h>
#include <vector>
#include <assert.h> 
#include <unistd.h> //close

class Epoller{
public:
    Epoller(int maxEvent = 1024);
    ~Epoller();

    int AddFd(int fd, uint32_t events);
    int ModFd(int fd, uint32_t events);
    int DelFd(int fd);
    int wait(int timeMS);
    int GetEventFd(size_t i);
    uint32_t GetEvents(size_t i);
private:
    int epollFd_;
    std::vector<epoll_event> events_;
};