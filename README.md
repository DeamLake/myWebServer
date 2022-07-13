# Tiny WebServer

Writen by C++

The most of details are based on 

[qinguoyi's TinyWebServer](https://github.com/qinguoyi/TinyWebServer) and 高性能服务器编程 by 游双

# Dependence

* libmysqlclient-dev
* ctags
# Build and Run(Only for Ubuntu)

```powershell
cd /path/to/webserver
make
cd build
./server
open localhost:1321 by browser
```

# Webbench
```powershell
cd webbench
make
./webbench -c [clients] -t [time] http://[ip]:[port]/ 
```