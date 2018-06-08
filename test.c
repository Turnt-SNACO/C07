#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char ** argv) {
	int pfd[2];
	pipe(pfd);
	int status;
	pid_t child;
	int i = 4;
	for (i; i >= 0; i--) {
		child = fork();
		switch (child) {
		case -1: printf("Danger\n"); break;
		case 0: 
			if (i == 0) {
				dup2(pfd[1], 1);
				dup2(pfd[0], 0);
				fprintf(stderr, "%d sending output\n", i);
				printf("hey i'm p%d ", i);
				exit(0);
			}
			else {
				dup2(pfd[0], 0);
				char buff[255];
				fprintf(stderr, "%d waiting for input...\n", i);
				read(0, buff, 255);
				fprintf(stderr, "%d got input!\n", i);
				printf("%s %d ", buff, i);
			}
			
		default:
			if (i == 0) {
				while (wait(&status) != child);
			}
		}
	}
	return 0;
}