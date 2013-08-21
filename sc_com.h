#ifndef _SC_COM_H
#define _SC_COM_H

#include<ev.h>

#define CLOEXEC 0x00001
#define NONBLOCK 0x00010
#define NODELAY 0x000100
#define NOPUSH 0x001000


#undef ev_io_init
static inline void ev_io_init(struct ev_io *w, 
                             void (*cb) (struct ev_loop *, struct ev_io *, int),
                             int fd, int events)
{
    ev_init(w,cb);
    ev_io_set(w, fd, events);
}

#undef ev_is_active
#define ev_is_active(w) (w)->active

#ifndef container_of
#define container_of(P, T, M) (T *)((char *)(P) - offsetof(T, M))
#endif




#endif
