#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int file_size = 65000;

	if(argc >= 2) {
		file_size = atoi(argv[1]);
	}

	if(mkdir("testfolder") == -1 && errno != EEXIST)
		perror("mkdir() error");

	if(trash_create("testfolder/testfile", file_size) == -1)
		perror("trash_create() error");

	return 0;
}
