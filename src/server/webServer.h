#pragma once
#include <sys/socket.h> // socket
#include <arpa/inet.h> // htons linger
#include <unistd.h> // close
#include <memory> // uniptr
#include <fcntl.h>
#include <unordered_map>

#include "Epoller.h"
#include "../log/log.h"
#include "../http/httpconn.h"
#include "../pool/threadpool.h"
#include "../timer/heaptimer.h"


class WebServer {
public:
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, 
        int threadNum, bool openLog, int logLevel, int logQueueSize);
    ~WebServer();
    void start();

private:
    bool InitSocket();
    void InitEventMode(int trigMode);
    void AddClient(int fd, sockaddr_in addr);
    void CloseConn(HttpConn* client);

    void DealListen();
    void DealRead(HttpConn* client);

    void ExtentTime(HttpConn* client);
    void OnRead(HttpConn* client);
    void OnProcess(HttpConn* client);

    static int SetFdNonblock(int fd);
private:
    int port_;
    int listenFd_;
    bool openLinger_;
    int timeoutMS_;
    bool isClose_;

    uint32_t listenEvent_;
    uint32_t connEvent_;
    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int,HttpConn> users_;
};