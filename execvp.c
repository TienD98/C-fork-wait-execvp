#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#define MAXSTR 255
#define ARGCNT 5

int main(int argc, char *argv[]) {
	char cmd[MAXSTR];
	char arg1[MAXSTR];
	char arg2[MAXSTR];
	char file[MAXSTR];
	// Additional code goes here
	printf("cmd->");
	fscanf(stdin, "%s %s %s %s", cmd, arg1, arg2, file);	
	printf("my input: %s %s %s %s ", cmd, arg1, arg2, file);
	printf("char %lu=%d\n",strlen(cmd),cmd[strlen(cmd)]);
	char ** args = malloc(sizeof(char *) * ARGCNT);
	*(args + 0) = cmd;
	*(args + 1) = arg1;
	*(args + 2) = arg2;
	*(args + 3) = file;
	
	 for (int i=0;i<ARGCNT;i++) { 
 		printf("i=%d args[i]=%s\n",i,*(args + i));
 	}
 	// store the value of execvp only if the function not executed
 	int status = execvp(args[0], args);
 	printf("STATUS CODE=%d\n",status);
 	
	return 0;
}
