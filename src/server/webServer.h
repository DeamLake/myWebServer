#include <sys/socket.h> // socket
#include <arpa/inet.h> // htons linger
#include <unistd.h> // close
#include <memory> // uniptr
#include <fcntl.h>
#include <unordered_map>

#include "Epoller.h"
#include "../http/httpconn.h"
#include "../pool/threadpool.h"

class WebServer {
public:
    WebServer(int port, int trigMode, int threadNum);
    ~WebServer();
    void start();

private:
    bool InitSocket();
    void InitEventMode(int trigMode);
    void AddClient(int fd, sockaddr_in addr);
    void CloseConn(HttpConn* client);

    void DealListen();
    void DealRead(HttpConn* client);

    void OnRead(HttpConn* client);
    void OnProcess(HttpConn* client);

    static int SetFdNonblock(int fd);
private:
    int port_;
    int listenFd_;
    bool isClose = false;

    uint32_t listenEvent_;
    uint32_t connEvent_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int,HttpConn> users_;
};