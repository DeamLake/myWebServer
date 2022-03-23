#include <sys/socket.h> // socket
#include <arpa/inet.h> // htons
#include <unistd.h> // close
#include <memory> // uniptr

#include "Epoller.h"

class WebServer {
public:
    WebServer(int port);
    ~WebServer();

    void start();
    bool InitSocket();

private:
    int port_;
    int listenFd_;
    std::unique_ptr<Epoller> epoller_;
};