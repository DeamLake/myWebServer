#pragma once
#include <arpa/inet.h> //sockaddr_in
#include <assert.h>
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn 
{
public:
    HttpConn();
    ~HttpConn() { Close(); }

    void Init(int fd, const sockaddr_in& addr);
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    bool process();

    void Close();
    int GetFd() const { return fd_; }
    int GetPort() const { return addr_.sin_port; }
    const char* GetIP() const { return inet_ntoa(addr_.sin_addr); }
    sockaddr_in GetAddr() const { return addr_; }

    int ToWriteBytes() { return iov_[0].iov_len + iov_[1].iov_len;}
    bool isKeepAlive() { return request_.IsKeepAlive();}

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
    
private:
    int fd_;
    bool isClose;
    sockaddr_in addr_;

    int iovCnt_;
    iovec iov_[2];
    Buffer readBuff_;
    Buffer writeBuff_;

    HttpRequest request_;
    HttpResponse response_;
};