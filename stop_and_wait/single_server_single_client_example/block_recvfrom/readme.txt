client 端又发又收
server 端只收不发

如果client的recefrom 函数不阻塞的话，发送了1次之后就会卡死在recefrom
现在的代码是设置了1s的阻塞，过了1s就不再等了

