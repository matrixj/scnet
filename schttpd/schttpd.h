#ifndef _SC_HTTPD_H_
#define _SC_HTTPD_H_

   #include<stdio.h> 
   #include<string.h> 
   #include<stdlib.h> 
   #include<stdbool.h> 
   #include<sys/socket.h> 
   #include<sys/un.h> 
   #include<arpa/inet.h> 
   #include<fcntl.h> 
   #include<unistd.h> 
  #include<assert.h> 
  #include<errno.h> 
  #include<stddef.h> 
  #include<ev.h> 

  #include "../sc_mem.h"

  #define GET 1
  #define POST 2
  #define UNKNOWN 3
   
  #define ISspace(x) isspace((int)(x)) 
   
  #define SERVER_DAEMAN "Server: schttpd/0.1.0\r\n" 
   
  #define REQUEST_MAX_SIZE 10240 

  #define MAX_READ 1024 



  struct sc_http_request_s{
      int fd;
      unsigned method;
      char url[MAX_READ];
      char  protocal[MAX_READ];
      struct sc_buffer *sc_buf;
  };
typedef struct sc_http_request_s *sc_http_request_t;
#define http_new_request(r) \
        sc_new((r))
sc_http_request_t
sc_http_new_request(sc_http_request_t r, unsigned method, char *url, char *protocal)
{
    r = http_new_request(r);
    return r;
}


#endif
