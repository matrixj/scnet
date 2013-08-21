#ifndef _SC_BUFFER_H
#define _SC_BUFFER_H

struct sc_buffer{
    char *buf;

    uint_fast16_t base, len, size;
};

void sc_buffer_bind(struct sc_buffer *self, char *buf, size_t size);

#define sc_buffer_available(B) ((B)->size - (B)->base - (B)->len)
#define sc_buffer_len(B) ((B)->len)
#define sc_buffer_data(B) ((B)->buf + (B)->base)

ssize_t sc_buffer_read(struct sc_buffer *, int fd);
/*将数据重新复制回到buf的起点处*/
void sc_buffer_rebase(struct sc_buffer *);

size_t sc_buffer_skip(struct sc_buffer *, size_t);

#define sc_buffer_reset(B) do { (B)->base = (B)->len =0; } while(0)



#endif
