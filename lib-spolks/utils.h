// Header file for utils lib
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

// Returns size of file named 'filename' in bytes
int fsize(char *filename);

// Writes string representation of 'num' to 'buf'
int itoa(int num, char *buf);

// Changes signal action for 'signum' to 'sig_handler()'
int set_sig_handler(int signum, void (*sig_handler)(int));

// Acquire file name from file path
// E.g.:  foldername/filename.x
// this function returns pointer to string "filename.x"
char *parse_filename(char *path);

// Creates file with specified size and name
// filled by random content
int trash_create(char *path, int size);

