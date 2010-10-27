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
#include "util.h"

#define SOCKET_ERROR	-1
#define BUFFER_SIZE		100000



char **cmdList;
int cmdListSize;
int acceptingConn;

void splitToArray(char *curRequest) {
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
}

struct FirstCmd processFirstCmd() {
	char *firstCmdCopy = malloc(sizeof(char) * (strlen(cmdList[0]) + 1));
	strcpy(firstCmdCopy, cmdList[0]);
	struct FirstCmd result;

	result.httpOp = extractFirstWord(&firstCmdCopy);
	result.path = extractFirstWord(&firstCmdCopy);
	result.httpVersion = extractFirstWord(&firstCmdCopy);
	return result;
}

void doGet(struct FirstCmd firstCmd, int hSocket, char *webRoot) {
	char *path = firstCmd.path;
	char *pBuffer = malloc(sizeof(char) * 10000000);
	pBuffer[0] = '\0';
	int pBufferLen = 0;
	char *absolutePath = malloc(sizeof(char) * 10000);
	strcat(absolutePath, webRoot);
	strcat(absolutePath, path);
	FILE *f = fopen(absolutePath, "r");
	if (f == NULL) {
		char *line;
		line = malloc(sizeof(char) * 100);
		strcpy(line, firstCmd.httpVersion);
		strcat(line, " 404 Not Found");
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Connection: close";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Content-Type: text/html";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Content-Length: 19";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		addCRLF(pBuffer, &pBufferLen);
		line = "Error 404 Not Found";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
	} else {
		char c;
		char *bodyBuffer = malloc(sizeof(char) * 10000000);
		int bodyLen = 0;
		while (!feof(f)) {
			fscanf(f, "%c",&c);
			bodyBuffer[bodyLen++] = c;
		};
		bodyBuffer[bodyLen] ='\0';
		char *line;
		line = malloc(sizeof(char) * 100);
		strcpy(line, firstCmd.httpVersion);
		strcat(line, " 200 OK");
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Date: Mon, 25 Oct 2010 07:54:17 GMT";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Connection: keep-alive";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
			char *contentType = getContentType(path);
			addResponse(pBuffer, &pBufferLen, contentType, strlen(contentType));
			char *contentLength = malloc(sizeof(char) * 100);
			sprintf(contentLength, "Content-Length: %d", bodyLen);
			addResponse(pBuffer, &pBufferLen, contentLength, strlen(contentLength));
			addCRLF(pBuffer, &pBufferLen);
			addResponse(pBuffer, &pBufferLen, bodyBuffer, bodyLen);
	}
	printf("  responding to GET command\n");
	write(hSocket, pBuffer, pBufferLen);
	if (f != NULL)
		fclose(f);
}

void processHttpRequest(char *request, int hSocket, char *webRoot) {
	int i;
	int size = strlen(request);
	int found = -1;
	for (i=0; i < size-3; i++) {
		if (request[i] == 13 && request[i+1] == 10 && request[i+2] == 13 && request[i+3] == 10) {
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


		splitToArray(curRequest);
		struct FirstCmd firstCmd = processFirstCmd();

		if (strcmp(firstCmd.httpOp, "GET") == 0) {
			doGet(firstCmd, hSocket, webRoot);
		}
	}
}

void handleNewConnection(int hSocket, char *webRoot) {
	char *httpRequest = malloc(BUFFER_SIZE * sizeof(char));

	char pBuffer[1000];
	acceptingConn = 1;
	while (acceptingConn) {
		int size = read(hSocket, pBuffer, BUFFER_SIZE);
		if (read > 0) {
			int i;

			pBuffer[size] = '\0';

			char *newHttpRequest = malloc(BUFFER_SIZE * sizeof(char));
			newHttpRequest = strncpy(newHttpRequest, pBuffer, size);
			newHttpRequest[size] = '\0';
			httpRequest = strcat(httpRequest, newHttpRequest);

			processHttpRequest(httpRequest, hSocket, webRoot);
		}
	}
}
