参考
http://www.cnblogs.com/churi/archive/2013/02/27/2935427.html

第一个注意的：
创建socket时要 SOCK_DGRAM, IPPROTO_UDP
SOCK_STREAM, IPPROTO_UDP 执行会退出


1.htons将整型变量从主机字节顺序转变成网络字节顺序， 就是整数在地址空间存储方式变为：高位字节存放在内存的低地址处。
2.SOCK_DGRAM套接口是在不建立连接的情况下被发送到远程进程的，可能无序地到达接收端。
3.inet_addr()的功能是将一个点分十进制的IP转换成一个长整数型数
4.INADDR_ANY就是指定地址为0.0.0.0的地址
5.AF_INET(又称PF_INET)是IPv4网络协议的套接字类型,AF_INET6则是IPv6的


参考http://blog.sina.com.cn/s/blog_412ef3420100xi4z.html
sockaddr_in和sockaddr是并列的结构，指向sockaddr_in的结构体的指针也可以指向
sockadd的结构体，并代替它。
struct sockaddr {
    unsigned short sa_family; 
    char sa_data[14]; 
};


struct sockaddr_in {
    short int sin_family; 
    unsigned short int sin_port; 
    struct in_addr sin_addr;
    unsigned char sin_zero[8]; 
};
sin_family指代协议族，在socket编程中只能是AF_INET
sin_port存储端口号（使用网络字节顺序）
sin_addr存储IP地址，使用in_addr这个数据结构
sin_zero是为了让sockaddr与sockaddr_in两个数据结构保持大小相同而保留的空字节。


struct in_addr {
    unsigned long s_addr;
};
s_addr按照网络字节顺序存储IP地址

服务器：
1.创建套接字，注意要用SOCK_DGRAM, IPPROTO_UDP
2.将套接字和本机的监听端口绑定
3.定义一个sockaddr_in结构体client，当recvfrom时远程客户机的ip、端口等存储在这里面
4.执行recvfrom
5.根据client用sendto发送消息

客户机：
1.创建套接字，注意要用SOCK_DGRAM, IPPROTO_UDP
2.定义sockaddr_in类型的结构体sin
3.根据套接字和sin执行sendto
4.同上执行recvfrom