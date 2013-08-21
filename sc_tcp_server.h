#ifndef _SC_SERVER_H
#define _SC_SERVER_H

#include "sc_pool.h"

struct sc_tcp_server ; 
//struct sc_tcp_server_settings ;

enum sc_tcp_server_error {
    SC_TCP_SERVER_ACCEPT_ERROR,
};

struct sc_tcp_server_settings {
    void (*pre_bind) (struct sc_tcp_server *);

    bool (*on_connect) (struct sc_tcp_server *, struct ev_loop *, int, struct sockaddr *, socklen_t);
    void (*on_error) (struct sc_tcp_server *,
                     struct ev_loop *,
                     enum sc_tcp_server_error);
    void (*on_accept_read)(struct ev_loop *, struct sc_tcp_server *, int );
    void (*on_accept_write)(int );
    sc_pool_t pool;
};


struct sc_tcp_server {
    ev_io connect;

    struct sc_tcp_server_settings *settings;
};

struct sc_tcp_server_client{
    ev_io accept_fd;

    struct sc_tcp_server_settings *settings;
};



struct sc_tcp_server_settings *sc_tcp_server_get_settings(); 

void sc_tcp_server_start(struct sc_tcp_server *self, struct ev_loop *loop);

void sc_tcp_server_stop(struct sc_tcp_server *self, struct ev_loop *loop);

void sc_tcp_sever_close(struct sc_tcp_server *self);

int sc_tcp_ipv4_listen(struct sc_tcp_server *self, 
                        const struct sc_tcp_server_settings *settings,
                        const char *addr, unsigned port,
                        int f, unsigned backlog);

/**TODO
*ipv6的sc_tcp_ipv6_listen()
*local域的sc_tcp_local_listen()
*/
#endif
