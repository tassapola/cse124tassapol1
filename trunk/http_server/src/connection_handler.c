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

void processCommands(char *commands) {
	int i;
	int size = strlen(commands);
	int found = -1;
	for (i=0; i < size-3; i++) {
		if (commands[i] == 13 && commands[i+1] == 10 && commands[i+2] == 13 && commands[i+3] == 10) {
			printf("found CRLF at %d\n", i);
			found = i;
			break;
		}
	}
	if (found > -1) {
		char *curCommands = malloc((found + 1) * sizeof(char));
		strncpy(curCommands, commands, (found) * sizeof(char));
		curCommands[found] = '\0';

		int newStart = found+4;
		int numChars = size - newStart;

		strncpy(commands, &commands[newStart], (numChars + 1) * sizeof(char));
		commands[numChars] = '\0';

		printf("DEBUG\n");
		printf("%s-%s\n", curCommands, commands);
		printf("size of commands = %d\n", strlen(commands));
	}
}

void handleConnection(int hSocket) {
	printf("hsocket %d\n", hSocket);
	char *commands = malloc(BUFFER_SIZE * sizeof(char));

			char pBuffer[1000];
			//strcpy(pBuffer, "hahah1234");
			//printf("sending %s to client\n", pBuffer);
			int size = read(hSocket, pBuffer, BUFFER_SIZE);
			int i;
			for (i =0; i < size; i++)
				printf("%d,", pBuffer[i]);
			pBuffer[size] = '\0';
			printf("\n");
			printf("%d %s--end\n", size, pBuffer);

			char *newCommands = malloc(BUFFER_SIZE * sizeof(char));
			newCommands = strncpy(newCommands, pBuffer, size);
			newCommands[size] = '\0';
			commands = strcat(commands, newCommands);
			printf("%s\n", commands);
			processCommands(commands);

			/*
	if (close(hSocket) == SOCKET_ERROR) {
		printf("could not close socket\n");
	} else {
		printf("socket closed\n");
	}*/
}
