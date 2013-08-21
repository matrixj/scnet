object=sc_tcp_conn.o sc_tcp_server.o sc_stream.o sc_buffer.o
CC=gcc
CFLAGS= -Wall 

scnet:$(object)
	ar rv  libsc.a $(object) 

sc_tcp_conn.o:sc_tcp_conn.c
	gcc -g -c sc_tcp_conn.c -o sc_tcp_conn.o -lev
sc_tcp_server.o:sc_tcp_server.c
	gcc -g -c sc_tcp_server.c -o sc_tcp_server.o -lev
sc_buffer.o:sc_buffer.c
	gcc -g -c sc_buffer.c -o sc_buffer.o -lev
sc_stream.o:sc_stream.c
	gcc -g -c sc_stream.c -o sc_stream.o -lev


clean:
	rm  libsc.a $(object)
    
