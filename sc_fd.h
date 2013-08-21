#ifndef _SC_FD_H
#define _SC_FD_H
#include<sys/fcntl.h>
#include<stdio.h>
#ifdef _FCNTL_H

/*文件描述符读/写/关闭*/

static inline int sc_open(const char *pathname, int flags, int cloexec, mode_t mode)
{
    int fd;
#ifdef O_CLOEXEC
    if (cloexec)
        flags |= O_CLOEXEC;
    else
        flags &= ~O_CLOEXEC;
#endif

open_retry:
    if ((fd = open(pathname, flags, mode) < 0)) {
        if (errno == EINTR)
            goto open_retry;
        else
            goto open_done;
    }

    if (cloexec) {
        int fl = fcntl(fd, F_GETFL);
        if (fl <0)
            goto open_failed;
#ifdef O_CLOEXEC
        if ((fl & FD_CLOEXEC) == 0)
#endif
            if(fcntl(fd, F_SETFL, fl|FD_CLOEXEC) < 0) {
                goto open_failed;
            }
    }
    goto open_done;
open_failed:
    close(fd);
    fd = -1;
open_done:
    return fd;
}

#endif

static inline ssize_t sc_read(int fd, char *buf, size_t count,const char *filename, int line)
{
    ssize_t rd_count = 1 ,total_read = 0;
    while (count != total_read){
        rd_count = read(fd, buf, count - total_read);
        if (rd_count < 0){
#ifdef EAGAIN
            if (errno == EAGAIN)
                return EAGAIN;
#endif
#ifdef EWOULDBLOCK
            if (errno == EWOULDBLOCK)
                return EWOULDBLOCK;
#endif
            return -1;
        }
        printf("%d read:%s,%d\n", rd_count, filename, line);
        if (rd_count == 0) return total_read;
        total_read += rd_count;
        buf += rd_count;
    }
    return total_read;
}

static inline ssize_t sc_write(int fd, char *buf, size_t count)
{
    ssize_t wr_count, total_write = 0;
    while(total_write != count){
        wr_count = write(fd, buf, count - total_write);
        if (wr_count == 0) return total_write;
        if(wr_count == -1) return -1;
        total_write += wr_count;
        buf += wr_count;
    }
    return total_write;
}

static inline int sc_close(int *fd)
{
    int ret = 0;
    if (fd != NULL && *fd >= 0){
        while(1){
            ret = close(*fd);
            if (ret == 0){
                *fd = -0xdead;
                break;
            }
            else if (errno == EINTR)
                continue;
        }
    }
    return ret;
}

static inline void  setnonblock(int so_fd)
{
    int flag ;
    int fd= so_fd;
    if(flag = fcntl(fd, F_GETFL) < 0)
        perror("fcntl_get");
    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) < 0)
        perror("fcntl_set");
    printf("set fd:%d to nonblock:%s,%d\n", fd, __FILE__, __LINE__);
}

#ifdef _SYS_UIO_H

/*聚集读，写*/

#endif/*_SYS_UIO_H*/

#endif/*_SC_FD_H*/
