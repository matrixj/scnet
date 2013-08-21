#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sc_buffer.h"
#include "sc_fd.h"

void sc_buffer_bind(struct sc_buffer *self, char *buf, size_t size)
{
    assert((buf && size > 0) || (!buf && size == 0));
    assert(self);
    

    *self = (struct sc_buffer) {
        .buf = buf,
        .size = size,
    };
}

ssize_t sc_buffer_read(struct sc_buffer *self, int fd)
{
    ssize_t len = sc_read(fd, sc_buffer_data(self) + sc_buffer_len(self),
                         sc_buffer_available(self));

    if (len > 0)
        self->len += len;

    return len;
}

void sc_buffer_rebase(struct sc_buffer *self)
{
    if (self->base >0) {
        if (self->len > 0) {
            memmove(self->buf,
                   self->buf+self->base,
                   self->len);
        }/*memmove可以处理好数据覆盖情况,momcpy不能*/
        self->base = 0;
    }
}

size_t sc_buffer_skip(struct sc_buffer *self, size_t step)
{
    if (step >= self->len) {
        sc_buffer_reset(self);
        return self->size;
    } else if (step > 0) {
        self->base += step;
        self->len -= step;
    }

    if (sc_buffer_available(self) < (self->size / 10))
        sc_buffer_rebase(self);

    return sc_buffer_available(self);
}
