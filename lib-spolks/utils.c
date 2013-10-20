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

// Calculate 8-bit CRC checksum for byte array
unsigned char crc8(unsigned char *array, int length)
{
	unsigned char crc = 0xFF;
	int i;
	int j;

	for (i = 0; i < length; i++) {
		crc ^= array[i];

		for (j = 0; j < 8; j++) {
			if (crc & 0x80) {
				crc <<= 1;
				crc ^= 0x31;
			} else {
				crc <<= 1;
			}
		}
	}

	return crc;
}

