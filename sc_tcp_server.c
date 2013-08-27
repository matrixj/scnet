#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "sc_com.h"
#include "sc_fd.h"
#include "sc_socket.h"
#include "sc_tcp_server.h"
//#include "sc_mem.h"


static struct sc_tcp_server_settings *s;

static void accept_write_callback(struct ev_loop *loop, struct ev_io *w, int revents)
{
    struct sc_tcp_server *accept_self = container_of(w, struct sc_tcp_server, connect);
    if ((revents & EV_WRITE ) && accept_self->settings->on_accept_write)
        accept_self->settings->on_accept_write(w->fd);
    
    ev_io_stop(loop, w);
    return  ;

}

static void accept_callback(struct ev_loop *loop, struct ev_io *w, int revents)
{
    //printf("accept_callback invoked :%s,%d\n", __FILE__, __LINE__);
    struct sc_tcp_server *accept_self = container_of(w, struct sc_tcp_server, connect);
    //assert(!(revents & EV_ERROR));
    //s->on_error(self, loop, SC_TCP_SERVER_ACCEPT_ERROR);
    int fd = w->fd;
    if (revents & EV_READ ){
        accept_self->settings->on_accept_read(loop, accept_self, fd);
        ev_io_stop(loop, w);

        ev_io_init(&(accept_self->connect), accept_write_callback, fd, EV_WRITE);
        ev_io_start(loop, &(accept_self->connect));
    }
}

static void init_accept_fd(struct sc_tcp_server *self, struct ev_loop *loop, int fd)
{
    struct sc_tcp_server *accept_self = sc_pool_alloc(self->settings->pool, sizeof(*self), __FILE__, __LINE__);
    accept_self->settings = self->settings;

    ev_io_init(&(accept_self->connect), accept_callback, fd, EV_READ);
    ev_io_start(loop, &(accept_self->connect));
    printf("fd = %d !!!  add to accept_callbak:%s,%d\n", fd, 
           __FILE__, __LINE__);
}


static void connect_callback(struct ev_loop *loop,struct ev_io *w ,int revents)
{
    struct sc_tcp_server *self = container_of(w,struct sc_tcp_server, connect);

    printf("in connect_callback:%s,%d\n", __FILE__, __LINE__);
    const struct sc_tcp_server_settings *settings = self->settings;
    assert(!(revents & EV_ERROR));

    if (revents & EV_READ) {
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof(addr);
        int fd;
        fd= sc_accept(w->fd, (struct sockaddr *)&addr, &addrlen);
        if (fd < 0)
            return;
              
        init_accept_fd(self, loop, fd);
/*
        if (fd < 0) {
            settings->on_error(self,loop,SC_TCP_SERVER_ACCEPT_ERROR);
        } else if (!settings->on_connect( self, loop, fd, 
                                        (struct sockaddr*)&addr, addrlen))
        sc_close(&fd);
*/
    }
}


static inline int init_ipv4(struct sockaddr_in *sin, const char *addr, unsigned port)
{
    sin->sin_port = htons(port);

    if (addr == NULL || addr[0] == '\0' ||
       ((addr[0] == '0' || addr[0] == '*')&& addr[1] == '\0')) {
           sin->sin_addr.s_addr = htonl(INADDR_ANY);
           return 1;
       }

    return inet_pton(sin->sin_family, addr, &sin->sin_addr);
}

static inline int init_ipv6(struct sockaddr_in6 *sin6, const char *addr, unsigned port)
{
    sin6->sin6_port = htons(port);

    if (addr == NULL || addr[0] == '\0' ||
       (addr[0] == '*' && addr[1] == '\0') ||
        (addr[0] == ':' && addr[1] == ':' && addr[2] == '\0')) {
            sin6->sin6_addr = in6addr_any;
            return 1;
        }
    return inet_pton(sin6->sin6_family,addr,&sin6->sin6_addr);
}

static inline int init_tcp(struct sc_tcp_server *self,
                          const struct sc_tcp_server_settings *settings,
                          struct sockaddr *sa, socklen_t sa_len,
                          int f,  unsigned backlog)
{
    int fd = sc_socket(sa->sa_family, SOCK_STREAM, 0, f);
    if (fd < 0)
        return -1;

    assert(self);
    assert(settings);

    if (sa->sa_family != AF_LOCAL) {
        int flags = 1;
        struct linger ling = { 0,0 };
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&flags, sizeof(flags));
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&flags, sizeof(flags));
        setsockopt(fd, SOL_SOCKET, SO_LINGER, (void*)&ling, sizeof(ling));

    }
    ev_io_init(&self->connect, connect_callback, fd, EV_READ);

    self->settings = settings;

    if (settings->pre_bind)
        settings->pre_bind(self);

    if (bind(fd, sa, sa_len) <0 ||
       listen(fd, backlog)<0) {
           int e = errno;
           sc_close(&self->connect.fd);
           errno=e;
           return -1;
       }
    return 1;

}

/*用户接口函数*/

struct sc_tcp_server_settings *sc_tcp_server_get_settings()
{
    struct sc_tcp_server_settings settings;
    struct sc_tcp_server_settings *pt_settings;
    pt_settings = &settings;
    return pt_settings;
}

void sc_tcp_server_start(struct sc_tcp_server *self,struct ev_loop *loop)
{
    assert(!ev_is_active(&self->connect));

    ev_io_start(loop, &self->connect);
    ev_run(loop, 0);
}

void sc_tcp_server_stop(struct sc_tcp_server *self, struct ev_loop *loop)
{
    assert(!ev_is_active(&self->connect));

    ev_io_stop(loop, &self->connect);
}

void sc_tcp_server_close(struct sc_tcp_server *self)
{
    assert(self->connect.fd >= 0);
    assert(!ev_is_active(&self->connect));

    sc_close(&self->connect.fd);
}


int sc_tcp_ipv4_listen(struct sc_tcp_server *self,
                      const struct sc_tcp_server_settings *settings,
                      const char *addr, unsigned port,
                      int f, unsigned backlog)
{
    struct sockaddr_in sin = { .sin_family = AF_INET };
    int e;

    //s = settings;
    if((e = init_ipv4(&sin, addr, port)) != 1)
        return e;
    return init_tcp(self, settings, (struct sockaddr *)&sin, sizeof(sin), f, backlog);
}

/*TODO*/
/*写ipv6的sc_tcp_ipv6_listen与本地域sc_tcp_local_listen*/
