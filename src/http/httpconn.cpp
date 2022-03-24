#include "../log/log.h"
#include "httpconn.h"

std::atomic<int> HttpConn::userCount;
bool HttpConn::isET;

HttpConn::HttpConn() {
    fd_ = -1;
    addr_ = {0};
    isClose = true;
}

HttpConn::~HttpConn() {
    Close();
}

int HttpConn::getFd() const{
    return fd_;
}

void HttpConn::Close(){
    if(isClose == false) {
        isClose = true;
        userCount--;
        close(fd_);
    }
}

void HttpConn::Init(int sockFd, const sockaddr_in& addr) {
    assert(sockFd > 0);
    userCount++;
    fd_ = sockFd;
    addr_ = addr;
    readBuff_.RetrieveAll();
    writeBuff_.RetrieveAll();
    isClose = false;
    LOG_INFO("Client: %d in!",fd_);
}

ssize_t HttpConn::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if(len <= 0) { break;}
    }while(isET);
    return len;
}

ssize_t HttpConn::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(ToWriteBytes() == 0) { break;}
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len){
                readBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }else{
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            readBuff_.Retrieve(len);
        }
    }while(isET || ToWriteBytes() > 10240);
    return len;
}

bool HttpConn::process() {
    if(readBuff_.ReadableBytes() <= 0)
        return false;

    iov_[0].iov_base = const_cast<char*>(readBuff_.Peek());
    iov_[0].iov_len = readBuff_.ReadableBytes();
    iovCnt_ = 1;
    return false;
}