#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include "utils.h"

// Returns size of file named 'filename' in bytes
int fsize(char *filename)
{
    struct stat fs;

    if (stat(filename, &fs) == -1)
        return -1;

    return fs.st_size;
}

// Writes string representation of 'num' to 'buf'
int itoa(int num, char *buf)
{
    int bytes_written = sprintf(buf, "%d", num);

    return bytes_written;
}

// Changes signal action for 'signum' to 'sig_handler()'
int set_sig_handler(int signum, void (*sig_handler)(int))
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_handler = sig_handler;
	
	return sigaction(signum, &sa, NULL);
}

// Acquire file name from file path
// E.g.:  foldername/filename.x
// this function returns pointer to string "filename.x"
char *parse_filename(char *path)
{
	char *ptr = path + strlen(path) - 1;
	int length = 0;

	for(; ptr >= path; ptr--)
	{
		if(*ptr == '/') break;
		length++;
	}

	if(length == 0) return NULL;

	char *filename = (char*)malloc(length + 1);

	return strcpy(filename, ptr + 1);
}

// Creates file with specified size and name
// filled by random content
int trash_create(char *path, int size)
{
	if(path == NULL || size < 1) return -1;

	FILE *file = fopen(path, "w+");

	if(file == NULL) return -1;


	int errno = 0;

	for(; size > 0; size--)
	{
		char buffer;

		if(fwrite(&buffer, sizeof(char), 1, file) < 1) 
		{
			errno = -1;
			break;
		}
	}

	close(file);

	return errno;
}
