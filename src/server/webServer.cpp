#include "webServer.h"
#include <iostream>
using namespace std;

WebServer::WebServer(int port, int trigMode, int threadNum): port_(port), 
            threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller()){
    if(!InitSocket()) { 
        cout<< "Init Socket Failed" <<endl; 
    }
    HttpConn::userCount = 0;
    InitEventMode(trigMode);
}

WebServer::~WebServer(){
    close(listenFd_);
    isClose = true;
    cout<< "Close Succeed!"<< endl;
}

bool WebServer::InitSocket(){
    if(port_ > 65535 || port_ < 1024) { 
        cout<< "Wrong port num"<<endl;
        return false; 
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) { 
        cout<< "socket build failed" <<endl;
        return false; 
    }

    int optval = 1;
    // port reuse
    int ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret < 0){
        cout<< "set socket setsockopt error!" <<endl;
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (sockaddr*)&addr, sizeof(addr));
    if(ret < 0){
        cout<< "Bind port: " << port_ << " error!" <<endl;
        return false;
    }

    ret = listen (listenFd_, 8);
    if(ret < 0){
        cout<< "Listen port: " << port_ << " error!" <<endl;
        return false;
    }

    ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
    if(ret < 0){
        cout<< "Add listen error!" <<endl;
    }
    SetFdNonblock(listenFd_);
    cout<< "Server port: "<< port_ <<endl;
    return true;
}

void WebServer::InitEventMode(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
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

void WebServer::start(){
    if(!isClose) { cout<< "============ Server start ============" <<endl;}
    while(!isClose){
        int eventCnt = epoller_->wait(1000);
        for(int i = 0; i < eventCnt; i++) {
            int fd = epoller_->GetEventFd(i);
            size_t events = epoller_->GetEvents(i);
            if(fd == listenFd_){
                DealListen();
            }else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                assert(users_.count(fd) > 0);
                CloseConn(&users_[fd]);
            }else if(events & EPOLLIN){
                assert(users_.count(fd) > 0);
                DealRead(&users_[fd]);
            }
        }
    }
}

void WebServer::CloseConn(HttpConn* client){
    cout<< "Client: " << client->getFd() << " quit!" <<endl;
    cout<< "User Count: " << HttpConn::userCount <<endl;
    epoller_->DelFd(client->getFd());
    client->Close();
}

void WebServer::AddClient(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].Init(fd,addr);
    epoller_->AddFd(fd, connEvent_ | EPOLLIN);
    SetFdNonblock(fd);
    cout<< "User Count: " << HttpConn::userCount <<endl;
}

void WebServer::DealListen() {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (sockaddr*)&addr, &len);
        if(fd <= 0) { return;}
        cout<< "accept: " << fd << endl;
        AddClient(fd, addr);
    } while(listenEvent_ & EPOLLET);
}

void WebServer::DealRead(HttpConn* client) {
    threadpool_->AddTask(std::bind(&WebServer::OnRead,this,client));
}

void WebServer::OnRead(HttpConn* client) {
    int readErrno = 0;
    int ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN){
        CloseConn(client);
        return;
    }
    OnProcess(client);
    client->write(&readErrno);
}

void WebServer::OnProcess(HttpConn* client) {
    if(client->process()) {
        epoller_->ModFd(client->getFd(), connEvent_ | EPOLLOUT);
    }else {
        epoller_->ModFd(client->getFd(), connEvent_ | EPOLLIN);
    }
}

int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}