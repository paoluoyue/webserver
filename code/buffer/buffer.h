#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    // 缓存区中可以读取的字节数
    size_t WritableBytes() const;
    // 缓存区中可以写入的字节数       
    size_t ReadableBytes() const ;
    // 缓存区中已经读取的字节数
    size_t PrependableBytes() const;

    // 获取当前读指针
    const char* Peek() const;

    // 保证将数据写入缓冲区
    void EnsureWriteable(size_t len);
    // 更新写指针
    void HasWritten(size_t len);

    // 更新读指针
    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);
    // 将缓冲区的数据转化为字符串
    void RetrieveAll() ;
    std::string RetrieveAllToStr();

    // 获取当前写指针
    const char* BeginWriteConst() const;
    char* BeginWrite();

    // 将数据写入到缓冲区
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    // IO操作的读写接口
    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:

    // 返回指向缓冲区初始位置的指针
    char* BeginPtr_();
    const char* BeginPtr_() const;

    // 用于缓冲区空间不够时的扩容
    void MakeSpace_(size_t len);

    // buffer的实体
    std::vector<char> buffer_;
    // 用于指示读指针
    std::atomic<std::size_t> readPos_;
    // 用于指示写指针
    std::atomic<std::size_t> writePos_;
};

#endif //BUFFER_H