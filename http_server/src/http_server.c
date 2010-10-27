/*
 ============================================================================
 Name        : http_server.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "connection_handler.h"

#define SOCKET_ERROR	-1
#define QUEUE_SIZE		100
#define BUFFER_SIZE		100000

int main(void) {
	int hSocket, hServerSocket;
	struct sockaddr_in address;
	char *webRoot = "/home/tor/cse124tassapol1/webroot";

	int port;
	for (port = 30000; port < 30010; port ++) {
		hServerSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (hServerSocket == SOCKET_ERROR) {
			printf("Could not make a socket\n");
			return EXIT_SUCCESS;
		}
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);
		address.sin_family = AF_INET;


		if (bind(hServerSocket, (struct sockaddr *) &address, sizeof(address)) == SOCKET_ERROR) {

		} else {
			printf("successfully binding to port %d\n", port);
			break;
		}
	}
	if (port == 30011) {
		printf("could not bind socket to any port\n");
		return EXIT_SUCCESS;
	}
	int nAddressSize = sizeof(struct sockaddr_in);
	getsockname(hServerSocket, (struct sockaddr *) &address, (socklen_t *) &nAddressSize);
	if (listen(hServerSocket, QUEUE_SIZE) == SOCKET_ERROR) {
		printf("could not listen\n");
		return EXIT_SUCCESS;
	}

	while (1) {
		printf("waiting for a connection\n");
		hSocket = accept(hServerSocket,(struct sockaddr *) &address, (socklen_t *) &nAddressSize);
		printf("  got a connection\n");
		pid_t pId = fork();
		if (pId != 0) {
			handleNewConnection(hSocket, webRoot);
		}
	}
	return EXIT_SUCCESS;
}
