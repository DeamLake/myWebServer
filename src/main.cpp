#include "server/webServer.h"

int main(){
    WebServer server(1321, 1, 6);
    server.start();
}