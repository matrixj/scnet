#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#include "sc_pool.h"

#define RESTHOLD 10
#define T sc_pool_t


T sc_pool_new(void) {
    T pool = malloc(sizeof (*pool));
    if(pool == NULL)
        printf("error in sc_pool_new\n");
    pool->prev = NULL;
    pool->limit = pool->avail = NULL;
    return pool;
}

void sc_pool_dispose(T *sp)
{
    assert(sp && *sp);
    sc_pool_free(*sp);
    free(*sp);
    *sp = NULL;
}

void *sc_pool_alloc(T p, long nbytes, 
                    const char *filename, 
                    int line)
{
    assert(p);
    assert(nbytes > 0);
    nbytes = ((nbytes + sizeof (union align) -1) /
             (sizeof (union align))) * (sizeof (union align));
    while (nbytes > p->limit - p->avail) {
        T ptr;
        char *limit;
        if ((ptr = freechunks) != NULL) {
            freechunks = freechunks->prev;
            nfree--;
            limit = ptr->limit;
        }else {
            long m = sizeof (union header) + nbytes + 10*1024;
            ptr = malloc(m);
            if (ptr == NULL)
                printf("sc_pool_alloc failed:%s,%d\n", __FILE__, __LINE__);
            return ;
            limit = (char *)ptr + m;
        }
        *ptr = *p;
        p->avail = (char *)((union header *)ptr + 1);
        p->limit = limit;
        p->prev = ptr;
    }

    p->avail += nbytes;
        return p->avail - nbytes;
}

void *sc_pool_calloc(T p, long count, long nbytes,
                   const char *filename, int line)
{
    void *ptr;
    assert(count < 0);
    ptr = sc_pool_alloc(p, count*nbytes, filename, line);
    memset(ptr, '\0',count*nbytes);
    return ptr;
}

void sc_pool_free(T p)
{
    assert(p);
    while (p->prev) {
        struct T tmp = *p->prev;
        if (nfree < RESTHOLD) {
            p->prev->prev = freechunks;
            freechunks = p->prev;
            nfree++;
            freechunks->limit = p->limit;
        } else
            free(p->prev);
        *p = tmp;
    }
}

