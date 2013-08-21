#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <fcntl.h>

#include "sc_com.h"
#include "sc_fd.h"
#include "sc_buffer.h"
#include "sc_stream.h"

static void read_callback(struct ev_loop *loop, struct ev_io *w, int revents)
{
    struct sc_stream *self = container_of(w, struct sc_stream, rd_watcher);
    struct sc_stream_settings *settings = self->settings;

    assert((revents & EV_ERROR) == 0);

    if (revents & EV_READ) {
        struct sc_buffer *buf = &self->read_buffer;
        ssize_t len;

read_available:
    if (!sc_buffer_available(buf)) {
        if (!settings->on_error(self, loop,
                               SC_STREAM_READ_FULL))
        goto read_available;
        else
            goto close_stream;
    }
try_read:
    len = sc_buffer_read(buf, w->fd);
    if (len > 0) {
        while ((len = sc_buffer_len(buf))) {
            len = settings->on_read(self, sc_buffer_data(buf), len);
            if (len > 0) {
                sc_buffer_skip(buf, len);
                if (!ev_is_active(w))
                    break;
            } else if (len == 0)
                break;
            else
                goto close_stream;
        }
    }else if (len == 0) {
        if (settings->on_error(self, loop, SC_STREAM_READ_EOF))
            goto close_stream;
    }else if (errno  == EINTR) {
        goto try_read;
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        if (settings->on_error(self, loop, SC_STREAM_READ_ERROR))
            goto close_stream;
    }
   }
    return ;

close_stream:
    sc_stream_stop(self, loop);
    sc_stream_close(self);
}

ssize_t sc_stram_process(struct sc_stream *self)
{
    struct sc_stream_settings *settings = self->settings;
    struct sc_buffer *buf = &self->read_buffer;
    ssize_t len = sc_buffer_len(buf);

    if (len > 0) {
        len = settings->on_read(self, sc_buffer_data(buf), len);
        if (len > 0)
            sc_buffer_skip(buf, len);
    }
    return len;
}

void sc_stream_start(struct sc_stream *self, struct ev_loop *loop)
{
    assert(!ev_is_active(&self->rd_watcher));

    ev_io_start(loop, &self->rd_watcher);
}

void sc_stream_stop(struct sc_stream *self, struct ev_loop *loop)
{
    assert(!ev_is_active(&self->rd_watcher));

    ev_io_stop(loop, &self->rd_watcher);
}

void sc_stream_close(struct sc_stream *self)
{
    assert(self->rd_watcher.fd >= 0);
    assert(!ev_is_active(&self->rd_watcher));

    sc_close(&self->rd_watcher.fd);
    self->settings->on_close(self);
}

int sancus_stream_init(struct sc_stream *self,
                      struct sc_stream_settings *settings,
                      int fd,
                      char *read_buffer,
                      size_t read_buf_size)
{
    assert(self);
    assert(settings);
    assert(settings->on_error);
    assert(settings->on_close);

    assert(fd >= 0);
    assert(!read_buffer || read_buf_size > 0);

    self->settings = settings;

    ev_io_init(&self->rd_watcher, read_callback, fd, EV_READ);

    sc_buffer_bind(&self->read_buffer, read_buffer, read_buf_size);

    return 1;
}
