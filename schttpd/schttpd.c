#include "schttpd.h"

#include "../sc_com.h"
#include "../sc_socket.h"
#include "../sc_tcp_server.h"
#include "../sc_buffer.h"
#include "../sc_pool.h"


sc_http_request_t req[1000];



void response(int fd);

void send_headers(int so_fd, struct sc_buffer *sc_buf);

void send_body(int so_fd, const char *filename, 
               struct sc_buffer *sc_buf);
/*
void run_cgi(int fd, const char *path, 
             const char *method, const char *query_string, 
             struct sc_buffer *sc_buf);
*/


void
on_accept_read(struct ev_loop *loop, struct sc_tcp_server * self, int fd)
{
    printf("on_accept_read  run!\n");
    int ret;
    bool cgi = false;
    char buf[10000];
    unsigned  method;
    char protocal[MAX_READ];
    char url[255];

    struct stat st;

    sc_http_request_t r;
    r = sc_pool_alloc(self->settings->pool, sizeof (*r), __FILE__, __LINE__);

    req[fd] = r;
    
    struct sc_buffer sc_buf;

    sc_buffer_bind(&sc_buf, buf, sizeof(buf));

    if((ret = read_header(fd, &sc_buf, r)) < 0)
        fprintf(stderr, "%d %s", ret, "ERROR in read_header\n" );
    /*
    int i=0, j=0;
    while(!ISspace(sc_buf.buf[j]) && (i < sizeof(method) - 1)){
        method[i] = sc_buf.buf[j];
        i++;j++;
    }
    method[i] = '\0';
    if (strncasecmp(sc_buf.buf[]))
   
    if (strcasecmp(method, "GET") == 0){
        r.method = 1;
    }
*/
    if(r->method == POST) {
        cgi = true;
    }

    if (r->method == UNKNOWN){
        printf("RECEIVE unknown headers\n");
        close(fd);
        return ;
    }
/*
    i = 0;
    while(ISspace(sc_buf.buf[j]) && (j < sizeof(sc_buf.buf)))
    j++;

    while(!ISspace(sc_buf.buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf))){
        url[i] = sc_buf.buf[j];
        i++;j++;
    }
    url[i] = '\0';

    */
    char *query_string = r->url;
    if (r->method == GET){
        while ((*query_string != '?') && (*query_string != '\0')) query_string ++;
        if (*query_string == '?'){
            cgi = true;
            *query_string = '\0';
            query_string++;
        }
    }

    char path[100];
    sprintf(path, "%s", r->url);
    if (path[strlen(path) - 1] == '/' ) 
        strcat(path, "index.html");
    if (stat(path, &st) == -1){
        printf("request is %s\n", path);
        printf("request file not found!\n");
        /*TODO: send 404 page*/
        /*sprintf(path, "%s", 404_page);*/
    }
    else{
        if ((st.st_mode & S_IFMT) == S_IFDIR)
            strcat(path, "/index.html");
        if ((st.st_mode & S_IXUSR) ||
            (st.st_mode & S_IXGRP) ||
            (st.st_mode & S_IXOTH)  )
            cgi = true;
        //if (!cgi){
            //response(loop, fd, path, &sc_buf);

        //}
        
        /*
        else
            run_cgi(fd, path, method, query_string, &sc_buf);
        */
    }
    sprintf(r->url, "%s", path);
}

void 
test_on_accept_write_callback(int fd)
{

    printf("on_accept_write_callback  run\n");
    response(fd);
    return ;
}


int 
main()
{
    struct ev_loop *loop = ev_default_loop(0);
    
    sc_pool_t pool = sc_pool_new();
    struct sc_tcp_server_settings settings = {
        .pre_bind = NULL,
        .on_accept_read = on_accept_read,
        .on_accept_write = test_on_accept_write_callback,
        .pool = pool,
    };

    struct sc_tcp_server self ;
    char *addr = NULL;
    unsigned port = 8000;
    int f = 0;
    f |= NONBLOCK;
    f |= NOPUSH;
    f |= CLOEXEC;


    signal(SIGPIPE, SIG_IGN);

    if(sc_tcp_ipv4_listen(&self,&settings, addr, port, f, 128) < 0)
        printf("error listen\n");
    sc_tcp_server_start(&self, loop);

    //ev_run(loop, 0);

    return 0;
}


int 
read_header(int fd, struct sc_buffer *sc_buf, sc_http_request_t r)
{
    int i = 0;
    char c = '\0';
    int n;
    ssize_t ret;
    char buf[MAX_READ];
    char method[5];
    ret = sc_read(fd, sc_buf->buf, sc_buf->size, __FILE__, __LINE__);
    printf("sc_read_header read %d\n", ret);

    sc_buf->buf[sc_buf->size-1] = '\0';
    sscanf(sc_buf->buf, "%s %s %s", method, r->url, r->protocal);
    r->fd = fd;
/*
    while (strstr(sc_buf->buf, "\r\n\r\n") == NULL || ret == EAGAIN  ){
         ret = sc_read(fd, sc_buf->buf, sc_buf->size, __FILE__, __LINE__);
    }
*/
    if (strcasecmp(method, "GET") == 0) 
        r->method = GET; 
    else if (strcasecmp(method, "POST") == 0 ) r->method = POST;
    else r->method = UNKNOWN;
    
    /*
    while((i < size - 1) && (c != '\n')){
        n = recv(fd, &c, 1, 0);
        if (n > 0){
            if(c == '\r'){
                n = recv(fd, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n'))
                    recv(fd, &c, 1, 0);
                else
                    c = '\n';
            }
            sc_buf->buf[i++] = c;
        }
        else 
            c = '\n';
    }
    sc_buf->buf[i] = '\0';
    */

    return ret;

}

void 
response(int so_fd)
{
    int ret = 1;
/*
    sc_buf->buf[0] = 'A';
    sc_buf->buf[1] = '\0';
    while ((ret > 0) && ('\n' != sc_buf->buf[0]))
        ret = read_line(so_fd, sc_buf, sc_buf->size);
*/
    struct sc_buffer sc_buf;
    char buf[MAX_READ];
    sc_buffer_bind(&sc_buf, buf, sizeof(buf));

    send_headers(so_fd, &sc_buf);
    send_body(so_fd, req[so_fd]->url, &sc_buf);
}

void 
send_headers(int so_fd, struct sc_buffer *sc_buf)
{
    sprintf(sc_buf->buf, "%s%s%s%s", "HTTP/1.0 200 OK\r\n", SERVER_DAEMAN, 
           "Content-Type: text/html; charset=UTF-8\r\n",
           "\r\n");
   send(so_fd, sc_buf->buf,strlen(sc_buf->buf), 0);
}

void 
send_body(int so_fd, const char *filename, struct sc_buffer *sc_buf)
{
    struct stat st;

    int fd = open(filename, O_RDONLY);
    if ((stat(filename, &st)) == -1)
    {
        perror("stat");
        close(fd);
        close(so_fd);
        return;
    }

    printf("request filename is %s:%s %d\n", filename, __FILE__, __LINE__);
    off_t offset = 0;
    int ret ;
    if((ret = sendfile(so_fd, fd, &offset, (ssize_t)st.st_size)) < 0){
        if (errno == EAGAIN)
            printf("sendfile errno is EAGAIN:%s %d\n", __FILE__, __LINE__);
        close(so_fd);
        close(fd);
        return ;
    }
    printf("sendfile send data %d to fd %d:%s %d\n", ret, so_fd, __FILE__, __LINE__);
    close(fd);
    close(so_fd);
    return ;
}
/*
void 
run_cgi(int fd, const char *path, const char *method, const char *query_string, struct sc_buffer *sc_buf)
{
    char buf[1024];
    int output[2];
    int input[2];
    pid_t pid;
    int status;
    int buf_size;
    int ret;
    int i;
    char c;
    int content_length;

    
    buf[0] = 'a';
    buf[1] = '\0';
    buf_size = sizeof(buf);

    if (strcasecmp(method, "GET") == 0)
        while ((ret >0 ) && strcmp("\n", buf))
            //ret = read_line(fd, sc_buf, sc_buf->size);得到Request　 Header最后一行
    else{
        ret = read_line(fd, sc_buf, sc_buf->size);
        while ((ret > 0) && strcmp("\n", buf)){
            buf[15] = '\0';
            if (strcasecmp(buf, "Content-length:") == 0)
                content_length = atoi (&buf[16]);
            ret = read_line(fd, sc_buf, sc_buf->size );
        }
        if(content_length == -1){
         //bad request
            printf("bad request !!!\n");
            return ;
        }

    }

    sprintf(buf, "HTTP/1.0 200 ok\r\n");发送HTTP response
    send(fd, buf, strlen(buf), 0);
    
    if (pipe(output) < 0) {
        printf("cann't not execute cgi output\n");
        return ;
    }
    if(pipe(input) < 0){
        printf("cann't not execut cgi input\n");
        return ;
    }

    if((pid = fork()) < 0) {
        perror("fork");
        printf("cann't not fork");
    }

    if(pid == 0){
        char env[255];
        char que_env[255];
        char length_env[255];

        //dup2(output[1], 1);stdout
        //dup2(input[0], 0);stdin

        sc_close(&output[0]);
        sc_close(&input[1]);

        sprintf(env, "REQUEST_METHOD=%s", method);
        putenv(env);
        if (strcasecmp(method, "GET") == 0) {
            sprintf(que_env, "QUERY_STRING=%s", query_string);
            putenv(que_env);
        }
       // else{POST请求写在body中
            sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }
        execl(path, path, NULL);
        exit(0);
    }
    else{
        sc_close(&output[1]);
        sc_close(&input[0]);
        if (strcasecmp(method, "POST") == 0)
            for(i = 0; i < content_length; i++){
                recv(fd, &c, 1, 0);
                write(input[1], &c, 1);
            }
        while (read(output[0], &c, 1) > 0)
            send(fd, &c, 1, 0);

        sc_close(&input[1]);
        sc_close(&output[0]);

        waitpid(pid, &status, 0);
    }
}
*/
