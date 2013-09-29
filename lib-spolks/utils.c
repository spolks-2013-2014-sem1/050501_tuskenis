#include <stdio.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

int fsize(char *filename)
{
    struct stat fs;

    if (stat(filename, &fs) == -1)
        return -1;

    return fs.st_size;
}

int itoa(int num, char *buf)
{
    int bytes_written = sprintf(buf, "%d", num);

    return bytes_written;
}

int set_sig_handler(int signum, void (*sig_handler)(int))
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_handler = sig_handler;
	sa.sa_flags = SA_RESETHAND;
	
	return sigaction(signum, &sa, NULL);
}
