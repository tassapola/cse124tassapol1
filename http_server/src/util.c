#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

char *getNowGMTDate() {
	time_t timer;                // Define the timer
	struct tm *tblock;           // Define a structure for time block
    timer = time(NULL);
    tblock = gmtime(&timer);
    char *ans = malloc(sizeof(char) * 10000);
	ans[0] = '\0';
	strcat(ans, "Date: ");
	switch(tblock->tm_wday) {
	  case 0: strcat(ans,"Sun "); break;
	  case 1: strcat(ans,"Mon "); break;
	  case 2: strcat(ans,"Tue "); break;
	  case 3: strcat(ans,"Wed "); break;
	  case 4: strcat(ans,"Thu "); break;
	  case 5: strcat(ans,"Fri "); break;
	  case 6: strcat(ans,"Sat "); break;
	}
	sprintf(ans,"%s%d ",ans, tblock->tm_mday);
	switch(tblock->tm_mon) {
	  case 0: strcat(ans,"Jan "); break;
	  case 1: strcat(ans,"Feb "); break;
	  case 2: strcat(ans,"Mar "); break;
	  case 3: strcat(ans,"Apr "); break;
	  case 4: strcat(ans,"May "); break;
	  case 5: strcat(ans,"Jun "); break;
	  case 6: strcat(ans,"Jul "); break;
	  case 7: strcat(ans,"Aug "); break;
	  case 8: strcat(ans,"Sep "); break;
	  case 9: strcat(ans,"Oct "); break;
	  case 10: strcat(ans,"Nov "); break;
	  case 11: strcat(ans,"Dec "); break;
	}
	sprintf(ans,"%s%d ",ans, 1900 + tblock->tm_year);
	sprintf(ans,"%s%02d:%02d:%02d GMT",ans, tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
	return ans;
}

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


char *extractFirstWordByColon(char **ptr) {
	char *s = *ptr;
	char *blankPos = strchrnul(s, ':'); //return pointer to space or to end of string
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

struct tm* getModifiedSinceTime(char *line) {

	int day, month, year;
	int hour, min, sec;
	char *s;
	char *months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	extractFirstWord(&line);
	extractFirstWord(&line);
	//printf("%s\n", extractFirstWord(&line)); //If-Modified-Since
	//printf("%s\n", extractFirstWord(&line)); //Mon
	s = extractFirstWord(&line); //25
	day = atoi(s);
	s = extractFirstWord(&line); //Oct
	int i;
	for (i=0; i < 12; i++)
		if (strcmp(s, months[i]) == 0) {
			month = (i+1);
			break;
		}
	s = extractFirstWord(&line); //2010
	year = atoi(s);
	s = extractFirstWord(&line); //07:54:17
	char *s2;
	s2 = extractFirstWordByColon(&s); //07
	hour = atoi(s2);
	s2 = extractFirstWordByColon(&s); //54
	min = atoi(s2);
	s2 = extractFirstWordByColon(&s); //17
	sec = atoi(s2);
	s = extractFirstWord(&line); //GMT

	//printf("day = %d\n", day);
	//printf("month = %d\n", month);
	//printf("year = %d\n", year);
	//printf("hour = %d\n", hour);
	//printf("min = %d\n", min);
	//printf("sec = %d\n", sec);
	struct tm *result = malloc(sizeof(struct tm));
	result->tm_mday = day;
	result->tm_mon = month - 1;
	result->tm_year = year - 1900;
	result->tm_hour = hour;
	result->tm_min = min;
	result->tm_sec = sec;
	printStructTm(result);
	return result;
}

void printStructTm(struct tm* t) {
	printf("%d/%d/%d %d:%d:%d\n", t->tm_mon, t->tm_mday, t->tm_year, t->tm_hour, t->tm_min, t->tm_sec);
}

int isFileModifiedAfterSinceTime(char *file, struct tm* since) {
	struct stat st;

	if (!stat(file, &st)) {
		struct tm* fileT = gmtime(&st.st_mtime);
		//char t[100];
		//strftime(t, 100, "%d/%m/%Y %H:%M:%S", fileT);
		//printf("\nLast modified date and time = %s\n", t);
		printStructTm(since);
		printStructTm(fileT);
		if (fileT->tm_year > since->tm_year) return 1;
		else {
			if (fileT->tm_year == since->tm_year) {
				if (fileT->tm_mon > since->tm_mon) {
					return 1;
				}
				else if (fileT->tm_mon == since->tm_mon)
					if (fileT->tm_mday > since->tm_mday) return 1;
					else if (fileT->tm_mday == since->tm_mday)
						if (fileT->tm_hour > since->tm_hour) return 1;
						else if (fileT->tm_hour == since->tm_hour)
							if (fileT->tm_min > since->tm_min) return 1;
							else if (fileT->tm_min == since->tm_min)
								if (fileT->tm_sec > since->tm_sec) return 1;
			}

		}
	}
	return 0;
}
