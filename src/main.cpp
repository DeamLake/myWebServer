#include "server/webServer.h"

int main(){
    WebServer server(
        1321, 3, true, 6,    // port trigmode optlinger threadnum
        true, 1, 1024);      // openlog loglevel logqueuesize
    server.start();
}