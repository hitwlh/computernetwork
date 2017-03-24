这两个.c代码是为了验证shutdown和close之间的区别
详细区别http://blog.csdn.net/mafuli007/article/details/7544373
注意:
    1>. 如果有多个进程共享一个套接字，close每被调用一次，计数减1，直到计数为0时，也就是所用进程都调用了close，套接字将被释放。
    2>. 在多进程中如果一个进程中shutdown(sfd, SHUT_RDWR)后其它的进程将无法进行通信. 如果一个进程close(sfd)将不会影响到其它进程. 得自己理解引用计数的用法了


首先建立一个连接，然后client端用同一个连接创建多个线程，去发送相同的数据
在线程里头，发送完有四种断开可能，具体查看client.c的58~69行

结果：

iResult = shutdown(ConnectSocket, SHUT_RD);也不影响后面的线程
SHUT_RD代表不能发送但是可以接收，但是后面的线程仍然可以正常读写(有一次recev失败了，不知道为什么？)，主进程的i'm here to fuck world也可以执行。



用close(ConnectSocket);后面的线程一一执行，但在尝试读写时都会失败



iResult = shutdown(ConnectSocket, SHUT_WR);
iResult = shutdown(ConnectSocket, SHUT_RDWR);
会影响后面的线程，后面的线程无法执行
直接退出，无法再发，连主进程的printf("i'm here to fuck world\n");都不能执行。