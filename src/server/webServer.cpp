#include "webServer.h"
using namespace std;

WebServer::WebServer(
    int port, int trigMode, int timeoutMS, bool OptLinger, 
    int threadNum,bool openLog, int logLevel, int logQueSize): 
    port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
    timer_(new HeapTimer()),threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
{
    srcDir_ = getcwd(nullptr, 256);
    strncat(srcDir_, "/resources/", 16);
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;
    SqlConnPool::Instance()->Init("localhost", 3306, "leilei", "666", "web", 12);

    InitEventMode(trigMode);
    if(!InitSocket()) { 
        LOG_INFO("Init Socket Failed");
    }
    if(openLog) {
        Log::Instance()->init(logLevel, "./logs", ".logs", logQueSize);
        if(isClose_) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent_ & EPOLLET ? "ET": "LT"),
                            (connEvent_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", 12, threadNum);
        }
    }
}

WebServer::~WebServer()
{
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::Instance()->ClosePool();
    LOG_INFO("Close Succeed!");
}

bool WebServer::InitSocket()
{
    if(port_ > 65535 || port_ < 1024) { 
        LOG_INFO("Wrong port num");
        return false; 
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) { 
        LOG_INFO("socket build failed!");
        return false; 
    }

    linger optLinger = { 0 };
    if(openLinger_) {
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    int ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0){
        LOG_INFO("Init linger error!");
        close(listenFd_);
        return false;
    }

    /* SO_REUSEADDR 允许端口被重复使用 */
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret < 0){
        LOG_INFO("set socket setsockopt error!!");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (sockaddr*)&addr, sizeof(addr));
    if(ret < 0){
        LOG_INFO("Bind port: %d error", port_);
        return false;
    }

    ret = listen (listenFd_, 8);
    if(ret < 0){
        LOG_INFO("Listen port: %d error!", port_);
        return false;
    }

    ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
    if(ret < 0){
        LOG_INFO("Add listen error!");
    }
    SetFdNonblock(listenFd_);
    LOG_INFO("Server port: %d", port_);
    return true;
}

void WebServer::InitEventMode(int trigMode) 
{
    // EPOLLRDHUP 事件代表对端断开连接
    listenEvent_ = EPOLLRDHUP;
    // 针对connfd，开启EPOLLONESHOT，因为我们希望每个socket在任意时刻都只被一个线程处理
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        connEvent_ |= EPOLLET;
        listenEvent_ |= EPOLLET;
        break;
    default:
        connEvent_ |= EPOLLET;
        listenEvent_ |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);
}

void WebServer::start()
{
    int timeMS = -1;
    if(!isClose_) { LOG_INFO("============ Server start ============");}
    while(!isClose_)
    {
        if(timeoutMS_ > 0) {
            timeMS = timer_->GetNextTick();
        }
        int eventCnt = epoller_->wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            int fd = epoller_->GetEventFd(i);
            size_t events = epoller_->GetEvents(i);
            if(fd == listenFd_){
                DealListen();
            }else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConn(&users_[fd]);
            }else if(events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead(&users_[fd]);
            }else if(events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite(&users_[fd]);
            }else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::CloseConn(HttpConn* client)
{
    epoller_->DelFd(client->GetFd());
    client->Close();
    LOG_INFO("Client: %d quit!",client->GetFd());
}

void WebServer::AddClient(int fd, sockaddr_in addr) 
{
    assert(fd > 0);
    users_[fd].Init(fd,addr);
    if(timeoutMS_ > 0) {
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn, this, &users_[fd]));
    }
    epoller_->AddFd(fd, connEvent_ | EPOLLIN);
    SetFdNonblock(fd);
}

void WebServer::DealListen() 
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    // ET mode
    do {
        int fd = accept(listenFd_, (sockaddr*)&addr, &len);
        if(fd <= 0) { return;}
        else if(HttpConn::userCount >= MAX_FD) {
            SendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient(fd, addr);
    } while(listenEvent_ & EPOLLET);
}

void WebServer::DealRead(HttpConn* client) 
{
    assert(client);
    ExtentTime(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead,this,client));
}

void WebServer::DealWrite(HttpConn* client) 
{
    assert(client);
    ExtentTime(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite,this,client));
}

void WebServer::SendError(int fd, const char* info) 
{
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::ExtentTime(HttpConn* client) 
{
    assert(client);
    if(timeoutMS_ > 0) { timer_->adjust(client->GetFd(), timeoutMS_);}
}

void WebServer::OnRead(HttpConn* client) 
{
    int readErrno = 0;
    int ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN){
        CloseConn(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite(HttpConn* client) 
{
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        // finish translate
        if(client->isKeepAlive()) {
            OnProcess(client);
            return;
        }
    }else if(ret < 0) {
        if(writeErrno == EAGAIN){
            // continue translate
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn(client);
}

void WebServer::OnProcess(HttpConn* client) 
{
    if(client->process()) {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    }else {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}

int WebServer::SetFdNonblock(int fd) 
{
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}