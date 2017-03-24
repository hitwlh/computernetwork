tutorial_proxy.c是从网上找的代码，my_proxy.c是自己模仿着实现的, my_proxy_linux.c是把windows的改到了linux

有几个要点：
1.要用多线程，如果一个连接之后客户端申请了更多的连接，自然需要有多线程来处理
2.代理服务器的accept负责建立和client的连接，所以要用while(1)一直等待accept（accept让主程序进入阻塞状态），accept成功的话就要新开线程去recv客户端的socket
3.要点：只支持get类，不支持连接类的http(头部是CONNECT，如baidu.com)，所以这个模型是代理服务器1次recv client，根据这个recv解析出真实服务器信息，然后send到真实服务器，真实服务器会分若干次返回消息，代理服务器要用while循环将这些消息一一发送回client。注意到：client只需要send一次！（我一开始写的一直不能通过，就是因为我写成了client每次recv之后都要send，大错特错！！！）
4.关于真实服务器的ip和端口，借鉴了tutorial_proxy.c的代码。但是需要注意的是：只需要从那里头获取ip和端口就行了。tutorial_proxy.c是直接自己构建了那个socket(它设置了超时时间，还对(192.168.0.1)这类ip地址作了解析，我的方法里头都没管这些)。
5.有点晚，困了。不想弄了。还有最后一点就是那个缓冲区的大小没弄好(我把servername的缓冲区那里写成几w是不对的。同时我也发现缓冲区设置成两万多会溢出，这也是我的代码里为了打印ecv和send的信息而在缓冲区数组的最后添加'\0'时越界的原因，这时已经超过缓冲区了（访问youku.com的时候会这样），tutorial_proxy.c如果想打印recv和send的信息，也需要在最后添加'\0'，结果也会导致越界中断)。先不管了，代码是能运行的，这个是细节。


从windows改到移植linux：
1. 要修改include的头文件
2. 普通的移植需要修改的(内存set、多线程、数据类型BYTE\DWORD)
3. 编译的时候要用命令gcc my_proxy_linux.c -o main -pthread
4. 
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1


typedef int SOCKET;
这里学到的：SOCKET是一个文件描述符，其实就是个int
摘抄：
int socket(int protofamily, int type, int protocol);
socket函数对应于普通文件的打开操作。普通文件的打开操作返回一个文件描述字，而socket()用于创建一个socket描述符（socket descriptor），它唯一标识一个socket。这个socket描述字跟文件描述字一样，后续的操作都有用到它，把它作为参数，通过它来进行一些读写操作。

closesocket 改成 close

不需要WSAStartup()、WSACleanup()

WSAGetLastError()改成errno
shutdown的第二个参数linux下是SHUT_RDWR、SHUT_RD、SHUT_WR


摘抄：
close-----关闭本进程的socket id，但链接还是开着的，用这个socket id的其它进程还能用这个链接，能读或写这个socket id
shutdown--则破坏了socket连接，读的时候可能侦探到EOF结束符，写的时候可能会收到一个SIGPIPE信号，这个信号可能直到socket buffer被填充了才收到，shutdown还有一个关闭方式的参数，0 不能再读，1不能再写，2 读写都不能。


通过国外的服务器运行my_proxy_linux.c，本机访问http://ip.chinaz.com/得到的ip是国外ip，good！