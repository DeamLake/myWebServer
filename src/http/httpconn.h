#include <arpa/inet.h> //sockaddr_in
#include <assert.h>

#include "../buffer/buffer.h"

class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void Init(int fd, const sockaddr_in& addr);
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    bool process();

    void Close();
    int getFd() const;
    int ToWriteBytes() { return iov_[0].iov_len + iov_[1].iov_len;}

    static std::atomic<int> userCount;
    static bool isET;
private:
    int fd_;
    bool isClose;
    sockaddr_in addr_;

    int iovCnt_;
    iovec iov_[2];
    Buffer readBuff_;
    Buffer writeBuff_;
};