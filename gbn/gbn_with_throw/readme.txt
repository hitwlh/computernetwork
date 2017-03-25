这里的throw全都是在server端完成的
%100收到数据包但不处理，只打印丢失信息，来模拟数据包丢失
%50不发ack，来模拟ack丢失
server端
#define continue_throw_ack_time 1<<M-1
定义连续丢失ack的量(每次都连丢1<<M-1个ack，来验证滑动窗口的有效性)