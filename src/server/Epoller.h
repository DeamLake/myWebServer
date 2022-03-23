#include <sys/epoll.h>
#include <vector>
#include <assert.h> 
#include <unistd.h> //close

class Epoller{
public:
    Epoller(int maxEvent = 1024);
    ~Epoller();
private:
    int epollFd_;
    std::vector<epoll_event> events_;
};