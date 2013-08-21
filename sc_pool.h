#ifndef _SC_POOL_H_
#define _SC_POOL_H_

#define T sc_pool_t 

typedef struct T *T;

struct T{
    T prev;/*begin of the pool block*/
    char *avail;/*the begin free location*/
    char *limit;/*the pointer to the byte that is after the end of pool*/
};

union align {
    int i;
    long l;
    long *lp;
    void *p;
    void (*fp)(void);
    float t;
    double d;
    long double ld;
};

union header {
    struct T b;
    union align a;
};

static T freechunks;
static int nfree;

T 
sc_pool_new   (void);     /*make a new sc_pool*/

void 
sc_pool_dispose(T *sp);/*force to free the  sc_pool block*/

void 
*sc_pool_alloc(T p, long nbytes, const char *filename, 
                    int line);/*alloc a new block in a sc_pool*/
void 
*sc_pool_calloc(T p, long count, 
                     long nbytes, const char *filename, 
                     int line);/*xxxx*/
void 
sc_pool_free(T p);/*free the sc_pool block by put it to the freechunks link*/



#undef T
#endif
