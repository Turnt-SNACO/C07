#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define STDOUT 1
#define STDIN 0
#define WRITE 1
#define READ 0

void redirect(int old, int new);
void printPipeError();
void printArgError();
void printForkError();
int pipeCount(int argc, char ** argv, int pipeIndex[argc], int nargs[argc]);
void exec(char ** cmdargs, int n);

int main(int argc, char ** argv) {
	int pipeIndex[argc];
	int nargs[argc];
	//get the number of pipes 
	int numPipes = pipeCount(argc, argv, pipeIndex, nargs);
	int nCmd = numPipes + 1;
	char *** groups = malloc(nCmd);
	int i = 0; int p = 0; int l = 0;
	//allocate memory for commands
	for (i = 0; i < nCmd; i++) {
		groups[i] = malloc(100);
		for (p = 0; p < 100; p++) {
			//groups[i][p] = malloc(256);
		}
	}
	//move commands to the group array
	for (i = 1, p = 0, l = 0; i < argc; i++) {
		if (strcmp("@", argv[i]) == 0) { p++; l = 0; }
		else {
			groups[p][l] = malloc(strlen(argv[i]));
			strcpy(groups[p][l++], argv[i]);
		}
	}
	int status;	pid_t child;  
	char * out = malloc(10000);
	int in = STDIN_FILENO;
	int fd[2];
	if (nCmd == 1) {
		exec(groups[0], nargs[0]);
	}
	for (i = 0; i < nCmd; i++) {				//for each command
		if (pipe(fd) == -1) {					//make the pipe check for bad pipe
			printPipeError();
			exit(1);
		}
		child = fork();							//fork and set child to pid of child process
		switch (child) {
			case -1:printForkError(); break;	//if fork fails
			case 0:								//child process
				close(fd[0]);					//close the other end of the pipe
				redirect(in, STDIN_FILENO);		//make fd swap for in
				redirect(fd[1], STDOUT_FILENO);	//make fd swap for out
				exec(groups[i], nargs[i]);		//run command
				exit(1);						//exit with code 1 only triggers if exec fails
				break;
			default:
				close(fd[1]);					//close write end of pipe
				close(in);						//close read end of pipe
				in = fd[0];						//update in to be the pipe
				break;
		}
	}
	read(fd[READ], out, 10000);					//read what's left in the pipe
	printf("%s", out);							//print what was left in the pipe to STDOUT
	return 0;
}
/*
 * redirects old pipe to the new pipe so when next command 
 * reads it gets the info from the old pipe
*/
void redirect(int old, int new) { 
	if (old != new) {
		if (dup2(old, new) != -1)
			close(old);
		else
			fprintf(stderr, "pipe error\n");
	}
}
/*
* Adds NULL to the end of the array of arguments so execvp doesn't complain
*/
void exec(char ** cmdargs, int n) {
	char ** argn;
	argn = malloc(n+1);
	int i; for (i = 0; i < n; i ++) {
		argn[i] = malloc(strlen(cmdargs[i]));
		strcpy(argn[i], cmdargs[i]);
	}
	argn[n] = NULL;
	execvp(cmdargs[0], argn);
	exit(0);
}
/*
* counts the pipes and keeps track of their indecies and makes sure no @ @ happens
*/
int pipeCount(int argc, char ** argv, int pipeIndex[argc], int nargs[argc]) {
	int i = 1, c = 0;
	int timeBomb = 0;
	for(i; i < argc; i++) {
		if (strcmp("@", argv[i])==0){
			timeBomb++;
			pipeIndex[c] = i;
			if (c == 0)
				nargs[c] = i -1;
			else
				nargs[c] = i - nargs[c-1]-2;
			c++;
			if (timeBomb == 2) {
				printArgError();
				exit(1);
			}
		} 
		else timeBomb = 0;
	}
	nargs[c] = argc - pipeIndex[c-1]-1;
	pipeIndex[c] = argc;
	return c;
}

void printForkError() {
	fprintf(stderr, "Process forking error.\n");
}

void printPipeError() {
	perror("Error making pipe\n.");
}

void printArgError() {
	fprintf(stderr, "pipe: syntax error near \'@\'\n");
	fprintf(stderr, "Usage: pipe cmd cmd-arg1 ... [ @ cmd cmd-arg1 ... ]\n");
}
