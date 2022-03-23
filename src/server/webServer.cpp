#include "webServer.h"
#include <iostream>
using namespace std;

WebServer::WebServer(int port): port_(port), epoller_(new Epoller()){
    if(!InitSocket()) { 
        cout<< "Init Socket Failed" <<endl; 
    }
}

WebServer::~WebServer(){
    close(listenFd_);
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

    int ret = bind(listenFd_, (sockaddr*)&addr, sizeof(addr));
    if(ret < 0){
        cout<< "Bind port: " << port_ << " error!";
        return false;
    }

    ret = listen (listenFd_, 8);
    if(ret < 0){
        cout<< "Listen port: " << port_ << " error!";
        return false;
    }

    cout<< "Server port: "<< port_<<endl;
    return true;
}

void WebServer::start(){
    while(true){

    }
}