!!!因bitbucket被墙,所以移到这里,已停止开发,当初写的代码现在自己都不想再动,完成重写了.请看SCNET2

SENET
===========
关于框架
------------
**SCNET** IS A simple C network server framework.实现了以下功能：

1.封装了 TCP server 中接受连接的功能；

2.使用非阻塞型I/O和事件驱动模型，基于libev:[http://libev.schmorp.de/](http://libev.schmorp.de/)；

3.基于内存池的分配器(pool allocators)

#TODO:
1.多线程，并实现线程池,实现leader/worker队列式的业务处理;

2.嵌入web服务器;

4.封装更多的常用数据结构，包括hash表等;

3.支持ssl;
