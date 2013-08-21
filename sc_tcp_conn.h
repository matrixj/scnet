#ifndef _SCNET_H
#define _SCNET_H

struct sc_tcp_conn;

enum sc_tcp_conn_state{
    SC_TCP_CONN_INPROGRESS,
    SC_TCP_CONN_CONNECTED,
    SC_TCP_CONN_RUNNING,
    SC_TCP_CONN_FAILED,
};

enum sc_tcp_conn_error{
    SC_TCP_CONN_WATCHER_ERROR,
    SC_TCP_CONN_CONNECT_ERROR,
};

struct sc_tcp_conn_settings{
    void (*on_read)(struct sc_tcp_conn * self,
                   struct ev_loop *l);
    void (*on_connect)(struct sc_tcp_conn* self,
                      struct ev_loop *l);
    void (*on_error)(struct sc_tcp_conn *self,
                    struct ev_loop *l,
                    enum sc_tcp_conn_error);
};

struct sc_tcp_conn{
    ev_io io;
    enum sc_tcp_conn_state state;
    ev_tstamp last_activity;
    const struct sc_tcp_conn_settings *settings;
};

#define sc_tcp_conn_fd(P) ((P)->io.fd)
#define sc_tcp_conn_touch(C,L) do{ (C)->last_activity=ev_now(L); }while(0)
#define sc_tcp_conn_elapsed(C,L) (ev_now(L)-(C)->last_activity)

void sc_tcp_conn_start(struct sc_tcp_conn *self,
                      struct ev_loop *loop);

void sc_tcp_conn_stop(struct sc_tcp_conn *self,
                     struct ev_loop *loop);

void sc_tcp_conn_close(struct sc_tcp_conn *self);

int sc_tcp_ipv4_connect(struct sc_tcp_conn *self,
                       const struct sc_tcp_conn_settings *settings,
                       const char *addr,unsigned port,
                       bool cloexec);

int sc_tcp_ipv6_connect(struct sc_tcp_conn *self,
                       const struct sc_tcp_conn_settings *settings,
                       const char *addr,unsigned port,
                       bool cloexec);

int sc_tcp_local_connect(struct sc_tcp_conn *self,
                        const struct sc_tcp_conn_settings *settings,
                        const char *path,
                        bool cloexec);

#endif
