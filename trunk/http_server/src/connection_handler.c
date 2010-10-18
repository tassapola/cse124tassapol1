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

char **curCmd;
int numLinesInCurCmd;

void splitToArray(char *curCommands) {
	printf("starting split to array\n");
	curCmd = malloc(sizeof(char *) * 100);
	numLinesInCurCmd = 0;
	int i;
	char *tmp = malloc(sizeof(char) * BUFFER_SIZE);
	int tmpSize = 0;
	for (i = 0; i < strlen(curCommands); i++) {
		if (i < strlen(curCommands) - 1 && curCommands[i] == 13 && curCommands[i+1] == 10) {
			tmp[tmpSize] = '\0';
			curCmd[numLinesInCurCmd] = tmp;
			numLinesInCurCmd++;
			tmp = malloc(sizeof(char) * BUFFER_SIZE);
			tmpSize = 0;
		} else if (i > 0 && curCommands[i] == 10 && curCommands[i-1]  == 13) {
			//do nothing
		} else {
			tmp[tmpSize++] = curCommands[i];
		}
	}
	if (tmpSize > 0) {
		tmp[tmpSize] = '\0';
		curCmd[numLinesInCurCmd] = tmp;
		numLinesInCurCmd++;

	}
	printf("DEBUG\n");
	for (i = 0; i < numLinesInCurCmd; i++) {
		printf("%s\n", curCmd[i]);
	}
}

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

		//printf("DEBUG\n");
		//printf("%s-%s\n", curCommands, commands);
		//printf("size of commands = %d\n", strlen(commands));

		splitToArray(curCommands);
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
