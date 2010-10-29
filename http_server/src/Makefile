util: 
	gcc -c util.c
connection_handler:
	gcc -c connection_handler.c

all: util connection_handler
	gcc -o http_server util.o connection_handler.o http_server.c

clean:
	rm *.o http_server

