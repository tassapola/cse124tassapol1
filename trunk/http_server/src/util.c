#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>


void addCRLF(char *buffer, int *bufferLen) {
	buffer[*bufferLen] = 13;
	buffer[(*bufferLen) + 1] = 10;
	buffer[(*bufferLen) + 2] = '\0';
	(*bufferLen) += 2;
}


void addResponse(char *buffer, int *bufferLen, char *text, int textLen) {
	int i;
	for (i=0; i < textLen; i++) {
		buffer[i + (*bufferLen)] = text[i];
	}
	(*bufferLen) += textLen;
	buffer[*bufferLen] = '\0';
	addCRLF(buffer, bufferLen);
}

char *getContentType(char *path) {
	char *res = malloc(sizeof(char) * 10000);
	char *dot = strchr(path, '.');
	if (dot == NULL) {
		return "Content-Type: text/html";
	} else {
		char *extension = malloc(sizeof(char) * 10000);
		strncpy(extension, (dot+1), (path+strlen(path) - dot));
		if (strcasecmp(extension, "htm") == 0 || strcasecmp(extension, "html") == 0) {
			return "Content-Type: text/html";
		} else
			if (strcasecmp(extension, "txt") == 0) {
				return "Content-Type: text/plain";
			} else
				if (strcasecmp(extension, "jpg") == 0 || strcasecmp(extension, "jpeg") == 0) {
					return "Content-Type: image/jpeg";
				} else
					if (strcasecmp(extension, "gif") == 0) {
						return "Content-Type: image/gif";
					}
	}

}

char *extractFirstWord(char **ptr) {
	char *s = *ptr;
	char *blankPos = strchrnul(s, ' '); //return pointer to space or to end of string
	//printf("s = %s, blankPos = %s\n", s, blankPos);
	char *word = malloc(sizeof(char) * (strlen(s) + 1));
	strncpy(word, s, blankPos - s);
	//printf("blankPos - s = %d\n", blankPos - s);
	word[blankPos - s + 1] = '\0';
	if (strlen(word) > 0) {
		char c = word[strlen(word)-1];
		if (!((c >= 'A' && c <= 'Z') || (c >='a' && c <= 'z') || (c >='0' && c <= '9')))
			word[strlen(word)-1] = '\0';
	}
	//printf("word = %s\n", word);
	*ptr = blankPos + 1;
	return word;
}
