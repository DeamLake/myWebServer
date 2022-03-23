#include <vector>
#include <atomic>
#include <sys/uio.h> // iovec 
#include <errno.h>
#include <unistd.h> // write
#include <assert.h>
#include<string.h> // bzero
 
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
    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

    void EnsureWritable(size_t len);
    void MakeSpace(size_t len);
    void Append(const char* str, size_t len);
private:
    char* BeginPtr();
    const char* BeginPtr() const;
    std::vector<char> buffer_;
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};