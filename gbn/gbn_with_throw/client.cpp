#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <algorithm>
#define DEFAULT_PORT "27015"
#define DEFAULT_DATASIZE 256
#define DEFAULT_BUFLEN DEFAULT_DATASIZE+sizeof(int)
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define M 2
typedef int SOCKET;
typedef unsigned char BYTE;
typedef unsigned long DWORD;
using namespace std;

int total_read = 0;
class MY_INT{
public:
	MY_INT(int value_, int size_)
	{
		value = value_;
		size = size_;
	}
	MY_INT& operator++(){
		value = (value+1) % size;
		return *this;
	}
	MY_INT operator+(MY_INT &a){
		return MY_INT((value + a.value) % size, size);
	}
	MY_INT operator+(int a){
		return MY_INT((value + a) % size, size);
	}
	bool operator<(MY_INT &a){
		return value < a.value;
	}
	bool operator==(MY_INT &a){
		return value == a.value;
	}
	bool operator==(int a){
		return value == a;
	}
	bool operator!=(MY_INT &a){
		return value != a.value;
	}
	bool operator>(MY_INT &a){
		return value > a.value;
	}
	int value;
	int size;
};
class BUF{
	//区间[start_index, end_index]就是client的发送window
	//读文件的时候讲文件读到buf里，处理的时候读过的文件不往回读，所以要先判断是否真的要读文件，以及读到的文件是否确认发送掉了
public:
	BUF(int window_size_){
		window_size = window_size_;
		read_buf = new char*[window_size_];
		for(int i = 0; i < window_size_; i++)
			read_buf[i] = new char[DEFAULT_BUFLEN];
		//send_buf = new char*[window_size_];
		start_index = 0;
		now_index = 0;
		error_flag = 0;
	}
	int writebuf(ifstream &fin)
	//start表示消息的起点，writebuf之后可以认为buf里的就是正确的文件
	{
		if(error_flag)
		{
			cout << "error flag\n";
			exit(-1);
		}
		fin.read(read_buf[now_index], sizeof(char) * DEFAULT_DATASIZE);
		total_read ++ ;
		printf("write buf[%d]\n", now_index);
		now_index = (now_index+1) % window_size;
		if(now_index == start_index)
			error_flag = 1;

		if(fin.eof()){
			printf("here returned %d\n", fin.gcount());
			return fin.gcount();
		}
		else
			return DEFAULT_DATASIZE;
		
	}
	void setSf(int sf)
	{
		start_index = sf;
		error_flag = 0;
	}
	char *getbuf(int n)
	{
		if(n>=0 && n<window_size)
			return read_buf[n];
		else
			cout << "n error!" << n <<' ' << window_size << endl;
		return NULL;
	}
	int window_size;
	char **read_buf;
	//read_buf负责读取文件，但是由于丢包等情况，read_buf的起点可能不是0，整个read_buf是从起点到了尾部再从头循环的
	//send_buf就是正确的顺序字符流
	int now_index;
	int error_flag;
	int get_start_index(){
		return start_index;
	}
private:
	int start_index;
};
int distance(int a, int b, int mm)
//M就是a和b的加法时需要模的M
{
	if(b < a)
		return mm - a + b;
	else
		return b - a;
}
int main(int argc, char **argv)
{
	//const char *sourcefile = "/SPclasses/tmp/yul.mp3";
	const char *sourcefile = "/SPclasses/tmp/a.pdf";
	cout << "sizeof(int): " << sizeof(int) << endl;
	cout << "DEFAULT_BUFLEN: " << DEFAULT_BUFLEN << endl;
	std::ifstream fin(sourcefile, std::ios::binary);
	char recvbuf[5] = "0000";
	int iResult;
	char server[20];
	if (argc != 2)	strcpy(server, "127.0.0.1");
	else	strcpy(server, argv[1]);
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(27015);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	socklen_t len = sizeof(sin);
	SOCKET ReceiveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int size = 1 << M;
	BUF my_buf(size - 1);
	struct timeval ti;
	ti.tv_sec = 0;
	ti.tv_usec = 1;
	setsockopt(ReceiveSocket,SOL_SOCKET,SO_RCVTIMEO,&ti,sizeof(ti));
	int Readend = 0;
	MY_INT Sf(0, size);
	MY_INT Sn(0, size);
	my_buf.setSf(0);
	int write_bytes, ret_value, Sendend = 0, read_flag = 1;
	char *sendbuf;
	//printf("%d %d\n", Sf.value, Sn.value);
	while(!Readend){
		//这个地方非常容易出错！如果sf = 1， window_size = 3，那Sf+window_size == 0，就是说加法用下面这种判断是错误的
			//Sn < (Sf+window_size)
			//解决方案是分3种情况，
			//1.Sf == 0，那么发送到Sn == 3时停止(不包含3)
			//2.Sf == 3，那么发送到Sn == 2时停止(不包含2)
			//3.发送到 Sn < Sf为止
		//这3种情况合并到一起就是distance函数
			//注意还要设置超时重传的flag，如果是超时重传，那就不用读文件了，否则按照默认的每次都读文件，就不对了
				//每次Sn更新，就往缓冲区里写数据
				//最后文件读取结束的时候，write_bytes里存储的就是最后一个块的字节数目
				//需要注意一下判断文件读完的条件
		if(Readend && Sf.value == Sn.value)
			break;
		while((distance(Sf.value, Sn.value, size) != size - 1) && !Readend){
			//printf("(distance(%d, %d, %d) = %d\n", Sf.value, Sn.value, size, distance(Sf.value, Sn.value, size));
			write_bytes = my_buf.writebuf(fin);
			++Sn;
			if(write_bytes != DEFAULT_DATASIZE)
				Readend = 1;
		}
		cout << "Total read:" << total_read <<endl;
		int real_window_size = min(size - 1, distance(Sf.value, Sn.value, size));
		//如果文件读完了，那窗口真实的size就不是size - 1了
		//上面这层while执行完之后缓冲区就写好了(整个窗口所有窗格的数据都被读入)，接下来就一起发送整个窗口
		for(int i = 0; i < real_window_size ; i++){
			int truely_write_bytes;
			if(i == real_window_size - 1)
				truely_write_bytes = write_bytes;
			else
				truely_write_bytes = DEFAULT_DATASIZE;
			sendbuf = my_buf.getbuf((my_buf.get_start_index() + i) % (size - 1));
			printf("read buf[%d]\n", (my_buf.get_start_index() + i) % (size - 1));
			int value = (Sf.value + i) % (size);
			memcpy(sendbuf + truely_write_bytes, &value, sizeof(int));
			cout << "SYN" << value << endl;
			sendto(ReceiveSocket, sendbuf, truely_write_bytes + sizeof(int), 0, (struct sockaddr *)&sin, len);
		}
		
		int ret = 1;
		int old_distance = 0;
		int old_f = Sf.value;
		while(ret != -1){
			//cout << "ret is: "<< ret << endl;
			ret = recvfrom(ReceiveSocket, recvbuf, sizeof(int), 0, (struct sockaddr *)&sin, &len);
			ret_value = *((int *)recvbuf);
			if(ret != -1 && distance(old_f, ret_value, size) > old_distance){//取最“大”的ack，防止ack乱序到达
				old_distance = distance(old_f, ret_value, size);
				Sf.value = ret_value;
			}
		}
		//printf("old_f is: %d, Sf.value is: %d\n", old_f, Sf.value);
		if(old_f != Sf.value){
			//printf("old_f: %d, Sf.value: %d, distance(old_f, Sf.value, size): %d\n", old_f, Sf.value, distance(old_f, Sf.value, size));
			//printf("my_buf.start_index: %d, distance(old_f, Sf.value, size): %d\n", my_buf.get_start_index(), distance(old_f, Sf.value, size));
			my_buf.setSf((my_buf.get_start_index() + distance(old_f, Sf.value, size) ) % ((1 << M) - 1));
		}
	}
	cout << "fin.gcount()+sizeof(int) is:" << sizeof(int) << endl;
	sendto(ReceiveSocket, sendbuf, sizeof(int), 0, (struct sockaddr *)&sin, len);
	fin.close();
	close(ReceiveSocket);
	return 0;
}