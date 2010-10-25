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

struct FirstCmd {
	char *httpOp, *path, *httpVersion;
};

char **cmdList;
int cmdListSize;

void splitToArray(char *curRequest) {
	printf("starting split to array\n");
	cmdList = malloc(sizeof(char *) * 100);
	cmdListSize = 0;
	int i;
	char *line = malloc(sizeof(char) * BUFFER_SIZE);
	int lineLen = 0;
	for (i = 0; i < strlen(curRequest); i++) {
		if (i < strlen(curRequest) - 1 && curRequest[i] == 13 && curRequest[i+1] == 10) {
			line[lineLen] = '\0';
			cmdList[cmdListSize] = line;
			cmdListSize++;
			line = malloc(sizeof(char) * BUFFER_SIZE);
			lineLen = 0;
		} else if (i > 0 && curRequest[i] == 10 && curRequest[i-1]  == 13) {
			//do nothing
		} else {
			line[lineLen++] = curRequest[i];
		}
	}
	if (lineLen > 0) {
		line[lineLen] = '\0';
		cmdList[cmdListSize] = line;
		cmdListSize++;

	}
	printf("DEBUG\n");
	for (i = 0; i < cmdListSize; i++) {
		printf("%s\n", cmdList[i]);
	}
}

char *extractFirstWord(char **ptr) {
	char *s = *ptr;
	char *blankPos = strchrnul(s, ' '); //return pointer to space or to end of string
	//printf("blankPos = %s, s = %s\n",blankPos, s);
	char *word = malloc(sizeof(char) * (strlen(s) + 1));
	strncpy(word, s, blankPos - s);
	word[blankPos - s + 1] = '\0';
	*ptr = blankPos + 1;
	return word;
}

struct FirstCmd processFirstCmd() {
	printf("starting processing of first command\n");
	char *firstCmdCopy = malloc(sizeof(char) * (strlen(cmdList[0]) + 1));
	strcpy(firstCmdCopy, cmdList[0]);
	struct FirstCmd result;

	result.httpOp = extractFirstWord(&firstCmdCopy);
	result.path = extractFirstWord(&firstCmdCopy);
	result.httpVersion = extractFirstWord(&firstCmdCopy);
	return result;
	/*
	char *blankPos = strchr(firstCmdCopy, ' ');
	printf("blankPos = %s, firstCmdCopy = %s\n",blankPos, firstCmdCopy);
	char *httpOp = malloc(sizeof(char) * (strlen(curCmd[0]) + 1));
	strncpy(httpOp, firstCmdCopy, blankPos - firstCmdCopy);
	httpOp[blankPos - firstCmdCopy + 1] = '\0';
	*/
	/*
	char *httpOp = strtok(firstCmdCopy, " ");
	printf("httpOp = %s\n", httpOp);
	httpOp[0] = 'z';
	printf("httpOp = %s\n", httpOp);
	printf("firstCmdCopy = %s\n", firstCmdCopy);
	char *path= strtok(firstCmdCopy, " ");
		printf("path = %s\n", path);
	*/
}

void addResponse(char *buffer, char *text) {
	strcat(buffer, text);
	addResponseEnding(buffer);
}

void addResponseEnding(char *buffer) {
	char *ending = malloc(sizeof(char) * 3);
	ending[0] = 13;
	ending[1] = 10;
	ending[2] = '\0';
	strcat(buffer, ending);
}

void doGet(char *path, int hSocket, char *webRoot) {
	char *pBuffer = malloc(sizeof(char) * 10000);
	pBuffer[0] = '\0';
	char *absolutePath = malloc(sizeof(char) * 10000);
	strcat(absolutePath, webRoot);
	strcat(absolutePath, path);
	printf("absolutePath = %s\n", absolutePath);
	FILE *f = fopen(absolutePath, "r");
	if (f == NULL) {
		addResponse(pBuffer, "HTTP/1.1 404 Not Found");
		addResponse(pBuffer, "Connection: close");
		addResponse(pBuffer, "Content-Type: text/html");
		addResponse(pBuffer, "Content-Length: 19");
		addResponseEnding(pBuffer);
		addResponse(pBuffer, "Error 404 Not Found");
	} else {
		char c;
		char *bodyBuffer = malloc(sizeof(char) * 10000);
		int bodyLen = 0;
		while (!feof(f)) {
			fscanf(f, "%c",&c);
			//printf("%c",c);
			bodyBuffer[bodyLen++] = c;
		};
		bodyBuffer[bodyLen] ='\0';
		addResponse(pBuffer, "HTTP/1.1 200 OK");
			addResponse(pBuffer, "Date: Mon, 25 Oct 2010 07:54:17 GMT");
			addResponse(pBuffer, "Connection: keep-alive");
			addResponse(pBuffer, "Content-Type: text/plain");
			char *contentLength = malloc(sizeof(char) * 100);
			sprintf(contentLength, "Content-Length: %d", bodyLen);
			//printf("contentLength = %s\n", contentLength);
			addResponse(pBuffer, contentLength);
			addResponseEnding(pBuffer);
			addResponse(pBuffer, bodyBuffer);
	}

	//addResponseEnding(pBuffer);
	//addResponseEnding(pBuffer);
	write(hSocket, pBuffer, strlen(pBuffer) + 1);

	if (f != NULL)
		fclose(f);

/*
	if (close(hSocket) == SOCKET_ERROR) {
		printf("could not close socket\n");
		return EXIT_SUCCESS;
	} else {
		printf("closing socket is successful\n");
	}
	*/
	printf("ending doGet\n");
}

void processHttpRequest(char *request, int hSocket, char *webRoot) {
	int i;
	int size = strlen(request);
	int found = -1;
	for (i=0; i < size-3; i++) {
		if (request[i] == 13 && request[i+1] == 10 && request[i+2] == 13 && request[i+3] == 10) {
			printf("found CRLF at %d\n", i);
			found = i;
			break;
		}
	}
	if (found > -1) {
		char *curRequest = malloc((found + 1) * sizeof(char));
		strncpy(curRequest, request, (found) * sizeof(char));
		curRequest[found] = '\0';

		int newStart = found+4;
		int numChars = size - newStart;

		strncpy(request, &request[newStart], (numChars + 1) * sizeof(char));
		request[numChars] = '\0';

		//printf("DEBUG\n");
		//printf("%s-%s\n", curCommands, commands);
		//printf("size of commands = %d\n", strlen(commands));

		splitToArray(curRequest);
		struct FirstCmd firstCmd = processFirstCmd();
		//printf("httpOp = %s\n", firstCmd.httpOp);
		//printf("path = %s\n", firstCmd.path);
		//printf("httpVersion = %s\n", firstCmd.httpVersion);

		if (strcmp(firstCmd.httpOp, "GET") == 0) {
			doGet(firstCmd.path, hSocket, webRoot);
		}
	}
}

void handleNewConnection(int hSocket, char *webRoot) {
	printf("hsocket %d\n", hSocket);
	char *httpRequest = malloc(BUFFER_SIZE * sizeof(char));

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

	char *newHttpRequest = malloc(BUFFER_SIZE * sizeof(char));
	newHttpRequest = strncpy(newHttpRequest, pBuffer, size);
	newHttpRequest[size] = '\0';
	httpRequest = strcat(httpRequest, newHttpRequest);
	printf("%s\n", httpRequest);
	processHttpRequest(httpRequest, hSocket, webRoot);

	/*
	if (close(hSocket) == SOCKET_ERROR) {
		printf("could not close socket\n");
	} else {
		printf("socket closed\n");
	}*/
}
