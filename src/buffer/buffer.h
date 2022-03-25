#pragma once
#include <vector>
#include <atomic>
#include <sys/uio.h> // iovec 
#include <errno.h>
#include <unistd.h> // write
#include <assert.h>
#include <string>
#include <string.h> // bzero
 
class Buffer {
public:
    Buffer(int BufferSIze = 1024);
    ~Buffer() = default;

    size_t ReadableBytes() const;
    size_t WritableBytes() const;
    size_t UselessBytes() const;

    char* BeginWrite();
    const char* Peek() const;
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveAll();
    void RetrieveUntil(const char* end);

    std::string RetrieveAllToStr();
    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

    void EnsureWritable(size_t len);
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

private:
    char* BeginPtr();
    const char* BeginPtr() const;
    void MakeSpace(size_t len);
    std::vector<char> buffer_;
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};