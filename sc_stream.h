#ifndef _SC_STREAM_H
#define _SC_STREAM_H

#define sc_stream_reset_readbuffer(S) sc_buffer_reset(&(S)->read_buffer)

struct sc_stream;

enum sc_stream_error{
    SC_STREAM_READ_ERROR,
    SC_STREAM_READ_EOF,
    SC_STREAM_READ_FULL,
};

struct sc_stream_settings{
    bool (*on_error) (struct sc_stream *,
                     struct ev_loop *,
                     enum sc_stream_error);
    void (*on_close) (struct sc_stream *);

    ssize_t (*on_read) (struct sc_stream *,
                       char *,size_t);
};

struct sc_stream{
    struct ev_io rd_watcher;
    struct sc_buffer read_buffer;

    struct sc_stream_settings *settings;
};

void sc_stream_start(struct sc_stream *self, struct ev_loop *loop);

void sc_stream_stop(struct sc_stream *self, struct ev_loop *loop);

void sc_stream_close(struct sc_stream *self);

int sc_stream_init(struct sc_stream *self,
                  struct sc_stream_settings *settings,
                  int fd,
                  char *read_buffer,
                  size_t read_buf_size);
static inline int sc_stream_fd(struct sc_stream *self)
{
    return self->rd_watcher.fd;
}

ssize_t sc_stream_process(struct sc_stream *self);

#endif
