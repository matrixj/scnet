#ifndef _SC_SOCKET_H
#define _SC_SOCKET_H

#include <stdio.h>
#include <netinet/tcp.h>
#include "sc_fd.h"

static inline int sc_setsockopt_nodelay(int fd)
{
    int yes = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, 
                   &yes, sizeof(yes)) == -1){
        fprintf(stderr, "setsockopt error\n");
        return -1;
    }
    return fd;
}

static inline int sc_setsockopt_nopush(int fd)
{
    int on = 1;
    if(setsockopt(fd, SOL_TCP, TCP_CORK, 
                  &on, sizeof(on)) == -1){
        fprintf(stderr, "setsockopt_tcp_CORK\n");
        return -1;
                  }
    return fd;
}

static inline int sc_socket(int family,int type,
                            int protocol,int f)
                            /*bool cloexec,
                            bool  nonblock,bool  nodelay,
                           bool nopush
                           */
{
    assert(!((f & NODELAY) &&  (f & NOPUSH)));
    int fd;
    if(f & CLOEXEC)
        type |=SOCK_CLOEXEC;
    else 
        type &=~SOCK_CLOEXEC;

    if(f & NONBLOCK)
        type |=SOCK_NONBLOCK;
    else 
        type &=~SOCK_NONBLOCK;
    if((fd = socket(family,type,protocol)) > 0)
        goto socket_done;/*成功*/

    if(f & NODELAY)
        if(sc_setsockopt_nodelay(fd) > 0)
            goto socket_done;
    if(f & NOPUSH)
        if(sc_setsockopt_nopush(fd) >0)
            goto socket_done;
/*
    if(cloexec || nonblock){
        int f_tmp,f=fcntl(fd,F_GETFL);
        if(f<0)
            goto socket_failed;
        f_tmp=f;
        if(cloexec)
            f_tmp |=FD_CLOEXEC;
        if(nonblock)
            f_tmp |=O_NONBLOCK;

        if(f != f_tmp && fcntl(fd,F_SETFL,f_tmp) < 0)
            goto socket_failed;
        goto socket_done;
    }
    */
socket_failed:
    close(fd);
    fd = -1;
socket_done:
    return fd;
}

/**
*自动失败重试的accept(3) 封装
*/
static inline int sc_accept(int fd,struct sockaddr *sa,socklen_t *sa_len)
{
    int accept_fd;
    while(1){
        if((accept_fd = accept(fd, sa, sa_len)) < 0 && errno == EINTR) continue;
        else if(accept_fd > 0)break;
    }
    printf("accept_fd is %d:%s,%d\n", accept_fd, __FILE__, __LINE__);
    setnonblock(accept_fd);

    return accept_fd;
}

#endif
