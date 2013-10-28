#include "utils.h"

pthread_mutex_t THREAD_SAFE_PRINTF_MUTEX = PTHREAD_MUTEX_INITIALIZER;

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

// Check file for existing
int is_file_exists(char *filename)
{
    FILE *fd = fopen(filename, "r");

    if (fd == NULL) {
        return 0;
    } else {
        fclose(fd);
        return 1;
    }
}

// Thread safe version of printf() function
int _printf(const char *format, ...)
{
    int ret_val;

    va_list args;
    va_start(args, format);

    pthread_mutex_lock(&THREAD_SAFE_PRINTF_MUTEX);
    ret_val = vprintf(format, args);
    pthread_mutex_unlock(&THREAD_SAFE_PRINTF_MUTEX);

    va_end(args);

    return ret_val;
}

