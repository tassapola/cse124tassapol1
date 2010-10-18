/*
 * connection_handler.c
 *
 *  Created on: Oct 17, 2010
 *      Author: torvnc
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define SOCKET_ERROR	-1
#define BUFFER_SIZE		100000

void handleConnection(int hSocket) {
	printf("hsocket %d\n", hSocket);
	char *commands = malloc(BUFFER_SIZE * sizeof(char));

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

	if (close(hSocket) == SOCKET_ERROR) {
		printf("could not close socket\n");
	} else {
		printf("socket closed\n");
	}
}
