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

	int port = 30000;
	hServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hServerSocket == SOCKET_ERROR) {
		printf("Could not make a socket\n");
		return EXIT_SUCCESS;
	}
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	address.sin_family = AF_INET;

	printf("binding to port %d\n", port);
	if (bind(hServerSocket, (struct sockaddr *) &address, sizeof(address)) == SOCKET_ERROR) {
		printf("could not bind socket to host\n");
		return EXIT_SUCCESS;
	}
	int nAddressSize = sizeof(struct sockaddr_in);
	getsockname(hServerSocket, (struct sockaddr *) &address, (socklen_t *) &nAddressSize);
	printf("opened socket as fd (%d) on port (%d) for stream i/o\n", hServerSocket,
	 	   ntohs(address.sin_port)
		  );
	printf("making a listen queue of %d elements", QUEUE_SIZE);
	if (listen(hServerSocket, QUEUE_SIZE) == SOCKET_ERROR) {
		printf("could not listen\n");
		return EXIT_SUCCESS;
	}

	while (1) {
		printf("waiting for a connection\n");
		hSocket = accept(hServerSocket,(struct sockaddr *) &address, (socklen_t *) &nAddressSize);
		printf("got a connection\n");
		pid_t pId = fork();
		if (pId != 0) {
			handleConnection(hSocket);
		}
		/*
		char pBuffer[1000];
		//strcpy(pBuffer, "hahah1234");
		printf("sending %s to client\n", pBuffer);
		int size = read(hSocket, pBuffer, BUFFER_SIZE);
		int i;
		for (i =0; i < size; i++)
			printf("%d,", pBuffer[i]);
		printf("\n");
		printf("%d %s--end\n", size, pBuffer);

		char *newCommands = malloc(BUFFER_SIZE * sizeof(char));
		newCommands = strncpy(newCommands, pBuffer, size);
		newCommands[size] = '\0';
		commands = strcat(commands, newCommands);
		printf("%s\n", commands);
		//write(hSocket, pBuffer, strlen(pBuffer) + 1);
		if (close(hSocket) == SOCKET_ERROR) {
			printf("could not close socket");
			return EXIT_SUCCESS;
		}
		*/
	}
	return EXIT_SUCCESS;
}
