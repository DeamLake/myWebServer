#include "server/webServer.h"

int main()
{
    WebServer server(
        1321, 3, 60000, true,  // port       trigmode  timeout  optlinger
        6, true, 1, 1024);     // threadnum  openlog   loglevel logqueuesize
    server.start();
}