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
#include <sys/stat.h>

#define SOCKET_ERROR	-1
#define BUFFER_SIZE		100000
#define DEBUG			0


char **cmdList;
int cmdListSize;
int acceptingConn;
struct FirstCmd firstCmd;

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

	if (DEBUG) {
		int i;
		printf("printing out curRequest\n");
		printf("%s\n", curRequest);
		printf("printing out cmd list\n");
		for (i=0; i < cmdListSize; i++) {
			printf("%s\n", cmdList[i]);
		}
	}
}

struct FirstCmd processFirstCmd() {
	char *firstCmdCopy = malloc(sizeof(char) * (strlen(cmdList[0]) + 1));
	strcpy(firstCmdCopy, cmdList[0]);
	struct FirstCmd result;

	result.httpOp = extractFirstWord(&firstCmdCopy);
	//printf("firstCmdCopy = %s\n", firstCmdCopy);
	result.path = extractFirstWord(&firstCmdCopy);
	//printf("path = %s\n", result.path);
	result.httpVersion = extractFirstWord(&firstCmdCopy);
	return result;
}

void doDelete(struct FirstCmd firstCmd, int hSocket, char *webRoot) {
	char *pBuffer = malloc(sizeof(char) * 10000000);
	pBuffer[0] = '\0';
	int pBufferLen = 0;
	char *absolutePath = malloc(sizeof(char) * 10000);
	absolutePath[0] = '\0';
	strcat(absolutePath, webRoot);
	strcat(absolutePath, firstCmd.path);
	int result = remove(absolutePath);
	if (DEBUG) {
		printf("absolutePath = %s\n", absolutePath);
		printf("result = %d\n", result);
	}
	if (result == 0) {
		//successful
		char *line;
		line = malloc(sizeof(char) * 100);
		strcpy(line, firstCmd.httpVersion);
		strcat(line, " 200 OK");
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Connection: close";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Content-Length: 0";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		addCRLF(pBuffer, &pBufferLen);
	} else {
		char *line;
		line = malloc(sizeof(char) * 100);
		strcpy(line, firstCmd.httpVersion);
		strcat(line, " 202 Accepted");
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Connection: close";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Content-Length: 0";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		addCRLF(pBuffer, &pBufferLen);
	}
	//if (DEBUG)
	//	printf("pBuffer = %s\n", pBuffer);
	write(hSocket, pBuffer, pBufferLen);
}

void mkDirsIfNeeded(char *absolutePath) {
	char *dir = malloc(sizeof(char) * 10000);
	mode_t mode = S_IRWXU;
	int i;
	for (i=0; i < strlen(absolutePath); i++) {
		dir[i] = absolutePath[i];
		dir[i+1] = '\0';
		if (dir[i] == '/') {
			mkdir(dir, mode);
		}
	}
}

void doPut(struct FirstCmd firstCmd, int hSocket, char *webRoot) {
	if (DEBUG)
		printf("starting doPut\n");
	char *pBuffer = malloc(sizeof(char) * 10000000);
	pBuffer[0] = '\0';
	int pBufferLen = 0;
	char *absolutePath = malloc(sizeof(char) * 10000);
	absolutePath[0] = '\0';
	strcat(absolutePath, webRoot);
	strcat(absolutePath, firstCmd.path);
	int fileExist = 0;
	FILE *f = fopen(absolutePath, "r");
	printf("f == NULL = %d\n", f == NULL);
	if (f != NULL) {
		fileExist = 1;
		if (DEBUG) printf("trying to close a file, f = %d\n", f);
		fclose(f);
	}
	if (DEBUG) printf("after closing a file\n");
	mkDirsIfNeeded(absolutePath);
	f = fopen(absolutePath, "w");
	if (DEBUG) {
		printf("absolutePath = %s\n", absolutePath);
		printf("f = %d\n", f);
	}
	char *content = cmdList[cmdListSize-1];
	int i;
	for (i=0; i < strlen(content); i++) {
		fprintf(f, "%c", content[i]);
	}
	fclose(f);

	char *line;
	line = malloc(sizeof(char) * 100);
	strcpy(line, firstCmd.httpVersion);
	if (!fileExist)
		strcat(line, " 201 Created");
	else
		strcat(line, " 200 OK");
	addResponse(pBuffer, &pBufferLen, line, strlen(line));
	line = getNowGMTDate();
	addResponse(pBuffer, &pBufferLen, line, strlen(line));
	line = "Connection: close";
	addResponse(pBuffer, &pBufferLen, line, strlen(line));
	line = "Content-Length: 0";
	addResponse(pBuffer, &pBufferLen, line, strlen(line));
	addCRLF(pBuffer, &pBufferLen);
	//if (DEBUG)
	//	printf("pBuffer = %s\n", pBuffer);
	write(hSocket, pBuffer, pBufferLen);
}

void doTrace(struct FirstCmd firstCmd, int hSocket) {
	char *pBuffer = malloc(sizeof(char) * 10000000);
	pBuffer[0] = '\0';
	int pBufferLen = 0;

	char *line;
	line = malloc(sizeof(char) * 100);
	strcpy(line, firstCmd.httpVersion);
	strcat(line, " 200 OK");
	addResponse(pBuffer, &pBufferLen, line, strlen(line));
	line = getNowGMTDate();
	if (DEBUG)
		printf("line = %s\n", line);
	addResponse(pBuffer, &pBufferLen, line, strlen(line));
	line = "Connection: close";
	addResponse(pBuffer, &pBufferLen, line, strlen(line));
	int i;
	int len = 0;
	for (i = 0; i < cmdListSize; i++)
		len += strlen(cmdList[i]) + 2;
	char *contentLength = malloc(sizeof(char) * 100);
	sprintf(contentLength, "Content-Length: %d", len);
	addResponse(pBuffer, &pBufferLen, contentLength, strlen(contentLength));

	addCRLF(pBuffer, &pBufferLen);
	for (i = 0; i < cmdListSize; i++)
		addResponse(pBuffer, &pBufferLen, cmdList[i], strlen(cmdList[i]));
	if (DEBUG)
		printf("pBuffer = %s\n", pBuffer);
	write(hSocket, pBuffer, pBufferLen);
}


void doGetOrHead(struct FirstCmd firstCmd, int hSocket, char *webRoot) {
	char *path = firstCmd.path;
	char *pBuffer = malloc(sizeof(char) * 10000000);
	pBuffer[0] = '\0';
	int pBufferLen = 0;
	char *absolutePath = malloc(sizeof(char) * 10000);
	absolutePath[0] = '\0';
	strcat(absolutePath, webRoot);
	strcat(absolutePath, path);
	int isGet;
	if (strcmp(firstCmd.httpOp, "GET") == 0)
		isGet = 1;
	else isGet = 0;
	FILE *f = fopen(absolutePath, "r");
	//printf("absolutePath = %s\n", absolutePath);
	//printf("f = %d\n", f);
	if (f == NULL) {
		char *line;
		line = malloc(sizeof(char) * 100);
		strcpy(line, firstCmd.httpVersion);
		strcat(line, " 404 Not Found");
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = getNowGMTDate();
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Connection: close";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Content-Type: text/html";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		line = "Content-Length: 19";
		addResponse(pBuffer, &pBufferLen, line, strlen(line));
		addCRLF(pBuffer, &pBufferLen);
		if (isGet) {
			line = "Error 404 Not Found";
			addResponse(pBuffer, &pBufferLen, line, strlen(line));
		}
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
		int ifModifiedSinceIndex = -1;
		int i;
		for (i = 0; i < cmdList[i]; i++) {
			if (strcasestr(cmdList[i], "If-Modified-Since:")) {
				ifModifiedSinceIndex = i;
				break;
			}
		}
		if (ifModifiedSinceIndex == -1) {
			strcat(line, " 200 OK");
			addResponse(pBuffer, &pBufferLen, line, strlen(line));
			line = getNowGMTDate();
			addResponse(pBuffer, &pBufferLen, line, strlen(line));
			line = "Connection: keep-alive";
			addResponse(pBuffer, &pBufferLen, line, strlen(line));
			char *contentType = getContentType(path);
			addResponse(pBuffer, &pBufferLen, contentType, strlen(contentType));
			char *contentLength = malloc(sizeof(char) * 100);
			sprintf(contentLength, "Content-Length: %d", bodyLen);
			addResponse(pBuffer, &pBufferLen, contentLength, strlen(contentLength));
			addCRLF(pBuffer, &pBufferLen);
			if (isGet)
				addResponse(pBuffer, &pBufferLen, bodyBuffer, bodyLen);
		} else {
			//found if modified since
			struct tm* t1 = getModifiedSinceTime(cmdList[ifModifiedSinceIndex]);
			if (isFileModifiedAfterSinceTime(absolutePath, t1)) {
				strcat(line, " 200 OK");
				addResponse(pBuffer, &pBufferLen, line, strlen(line));
				line = getNowGMTDate();
				addResponse(pBuffer, &pBufferLen, line, strlen(line));
				line = "Connection: keep-alive";
				addResponse(pBuffer, &pBufferLen, line, strlen(line));
				char *contentType = getContentType(path);
				addResponse(pBuffer, &pBufferLen, contentType, strlen(contentType));
				char *contentLength = malloc(sizeof(char) * 100);
				sprintf(contentLength, "Content-Length: %d", bodyLen);
				addResponse(pBuffer, &pBufferLen, contentLength, strlen(contentLength));
				addCRLF(pBuffer, &pBufferLen);
				if (isGet)
					addResponse(pBuffer, &pBufferLen, bodyBuffer, bodyLen);
			} else {
				strcat(line, " 304 Not Modified");
				addResponse(pBuffer, &pBufferLen, line, strlen(line));
				line = getNowGMTDate();
				addResponse(pBuffer, &pBufferLen, line, strlen(line));
				line = "Connection: keep-alive";
				addResponse(pBuffer, &pBufferLen, line, strlen(line));
				line = "Content-Length: 0";
				addResponse(pBuffer, &pBufferLen, line, strlen(line));
				addCRLF(pBuffer, &pBufferLen);
			}
		}
	}
	if (isGet)
		printf("  responding to GET command\n");
	else
		printf("  responding to HEAD command\n");
	if (f != NULL)
		fclose(f);
	write(hSocket, pBuffer, pBufferLen);

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
		if (strstr(cmdList[cmdListSize-1], "Content-Length: ") != NULL) {
			char *numS = cmdList[cmdListSize-1] + 16;
			int len = atoi(numS);
			//printf("content length = %d\n", len);
			char *line = malloc(sizeof(char) * BUFFER_SIZE);
			strncpy(line, request, len);
			line[len] = '\0';
			request = request + len;


			cmdList[cmdListSize++] = line;
			if (DEBUG) {
				printf("line = %s\n", line);
				printf("request = %s\n", request);
				int i;
				for (i = 0; i < cmdListSize; i++)
					printf("%s\n", cmdList[i]);
			}
		}
		firstCmd = processFirstCmd();

		if (DEBUG) {
			printf("httpOp = %s\n", firstCmd.httpOp);
		}
		if (strcmp(firstCmd.httpOp, "GET") == 0 || strcmp(firstCmd.httpOp, "HEAD") == 0)
			doGetOrHead(firstCmd, hSocket, webRoot);
		else if (strcmp(firstCmd.httpOp, "DELETE") == 0)
			doDelete(firstCmd, hSocket, webRoot);
		else if (strcmp(firstCmd.httpOp, "TRACE") == 0)
			doTrace(firstCmd, hSocket);
		else if (strcmp(firstCmd.httpOp, "PUT") == 0)
			doPut(firstCmd, hSocket, webRoot);
	}
}

void handleNewConnection(int hSocket, char *webRoot) {
	char *httpRequest = malloc(BUFFER_SIZE * sizeof(char));

	char pBuffer[1000];
	acceptingConn = 1;
	while (acceptingConn) {
		int size = read(hSocket, pBuffer, BUFFER_SIZE);
		if (read > 0) {
			//printf("read > 0");
			int i;

			pBuffer[size] = '\0';

			char *newHttpRequest = malloc(BUFFER_SIZE * sizeof(char));
			newHttpRequest = strncpy(newHttpRequest, pBuffer, size);
			newHttpRequest[size] = '\0';
			httpRequest = strcat(httpRequest, newHttpRequest);

			processHttpRequest(httpRequest, hSocket, webRoot);
			if (strcmp(firstCmd.httpVersion,"HTTP/1.0") == 0) {
				acceptingConn = 0;
			}
		} else {
			//printf("read == 0");
		}
	}
	close(hSocket);
}



