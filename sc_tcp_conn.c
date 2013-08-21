#include<assert.h>
#include<errno.h>
#include<fcntl.h>
#include<stddef.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdbool.h>

#include "sc_com.h"
#include "sc_tcp_conn.h"
#include "sc_socket.h"
#include "sc_fd.h"

static void io_callback(struct ev_loop *loop,struct ev_io *w ,int revents)
{
    struct sc_tcp_conn *self=container_of(w,struct sc_tcp_conn,io);
    const struct sc_tcp_conn_settings *settings=self->settings;

    if(revents & EV_ERROR){
        settings->on_error(self,loop,SC_TCP_CONN_WATCHER_ERROR);
        return ;
    }
    switch (self->state){
        case SC_TCP_CONN_INPROGRESS:
            assert(revents &EV_WRITE);
            if(revents &EV_WRITE){
                int error;
                socklen_t len=sizeof(error);

                if(getsockopt(sc_tcp_conn_fd(self), SOL_SOCKET, SO_ERROR, &error, &len) < 0)
                ; /*连接失败但errno已经设置好*/
                else if (error == 0)
                    goto connect_done;/*连接成功*/
                else 
                    errno = error;

                self->state = SC_TCP_CONN_FAILED;
                settings->on_error(self,loop,SC_TCP_CONN_CONNECT_ERROR);
                return ;
            }

connect_done:
    ;
    case SC_TCP_CONN_CONNECTED:
        assert(revents & EV_WRITE);
        {
            int fd=w->fd;
            ev_io_stop(loop,w);
            ev_io_init(w,io_callback,fd,EV_READ);
            ev_io_start(loop,w);
        }

        settings->on_connect(self,loop);
        self->state = SC_TCP_CONN_RUNNING;
        sc_tcp_conn_touch(self,loop);
        break;
    case SC_TCP_CONN_RUNNING:
        if (revents & EV_READ){
            settings->on_read(self,loop);
            sc_tcp_conn_touch(self,loop);
        }
        break;
    case SC_TCP_CONN_FAILED:
        assert(0);
    
    }

}
static inline int init_ipv4(struct sockaddr_in *sin,const char *addr,unsigned port)
{
    sin->sin_port=htons(port);
    return inet_pton(sin->sin_family,addr,&sin->sin_addr);
}

static inline int init_ipv6(struct sockaddr_in6 *sin6,const char *addr,unsigned port)
{
    sin6->sin6_port=htons(port);
    return inet_pton(sin6->sin6_family,addr,&sin6->sin6_addr);
}
static inline int init_local(struct sockaddr_un *sun,const char *path)
{
    size_t s=0;
    if(path==NULL){
        path="";
    }else{
        s=strlen(path);
        if(s>sizeof(sun->sun_path)-1)
            return 0;
    }
    memcpy(sun->sun_path,path,s+1);
    return 1;
}

static inline int init_tcp(struct sc_tcp_conn *self,
                          const struct sc_tcp_conn_settings *settings,
                          struct sockaddr *sa,
                          socklen_t sa_len,
                          int f)
{
    int fd=sc_socket(sa->sa_family,SOCK_STREAM,0, f);
    if(fd<0)
        return -1;

    assert(self);
    assert(settings);
    ev_io_init(&self->io,io_callback,fd,EV_READ|EV_WRITE);
    self->settings=settings;
    if(connect(fd,sa,sa_len)<0){
        if(errno==EINPROGRESS){
            self->state=SC_TCP_CONN_INPROGRESS;
        }else{
            int e=errno;
            sc_close(&self->io.fd);
            errno=e;
            return -1;
        }
    }else{
        self->state=SC_TCP_CONN_CONNECTED;
    }
    return 1;

}

void sc_tcp_start(struct sc_tcp_conn *self,struct ev_loop *loop)
{
    assert(!ev_is_active(&self->io));
    ev_io_start(loop,&self->io);
    if(self->last_activity == 0.0)
        sc_tcp_conn_touch(self,loop);

}

void sc_tcp_stop(struct sc_tcp_conn *self,struct ev_loop *loop)
{
    assert(ev_is_active(&self->io));
    ev_io_stop(loop,&self->io);
}

void sc_tcp_close(struct sc_tcp_conn *self)
{
    assert(self->io.fd>=0);
    assert(!ev_is_active(&self->io));

    sc_close(&self->io.fd);
}

int sc_tcp_ipv4_connect(struct sc_tcp_conn *self,
                       const struct sc_tcp_conn_settings *settings,
                       const char *addr,unsigned port,
                       bool cloexec)
{
    struct sockaddr_in sin={ .sin_family =AF_INET };
    int e;
    if((e=init_ipv4(&sin,addr,port))!=1)
        return e;
    return init_tcp(self,settings,(struct sockaddr *)&sin,sizeof(sin), cloexec);
}


int sc_tcp_ipv6_connect(struct sc_tcp_conn *self,
                       const struct sc_tcp_conn_settings *settings,
                       const char *addr,unsigned port,
                       bool cloexec)
{
    struct sockaddr_in6 sin6={ .sin6_family =AF_INET6 };
    int e;
    if((e=init_ipv6(&sin6,addr,port))!=1)
        return e;
    return init_tcp(self,settings,(struct sockaddr *)&sin6,sizeof(sin6), cloexec);
}

int sc_tcp_local_connect(struct sc_tcp_conn *self,
                        const struct sc_tcp_conn_settings *settings,
                        const char *path,bool cloexec)
{
    struct sockaddr_un sun={ .sun_family = AF_LOCAL };
    int e;
    if((e=init_local(&sun,path))!=1)
        return e;
    return init_tcp(self,settings,(struct sockaddr *)&sun,sizeof(sun), cloexec);

}

