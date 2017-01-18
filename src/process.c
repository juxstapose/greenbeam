#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "process.h"

void Process_Run(char* argv[]) {
	signal(SIGCHLD, Process_Proc_Exit);
	pid_t   child;
    	if ((child = fork()) == 0) {                 /*Child Process*/
        	int status = execvp(argv[0],argv);       
        	fprintf(stderr, "RSI: %s: command not found\n",argv[0]); /*If execvp failes*/
        	if(status == -1) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		exit(1);

    	} else {          /*Parent process*/
        	if (child == (pid_t)(-1)) {
        		fprintf(stderr,"Fork failed\n"); 
			exit(1);
        	}
	       	else {
               		printf("%ld Started child process\n",(long)getpid());
		} //end else
    	}//end else parent process
}

void Process_Proc_Exit() {
	
	int wstatus;
	pid_t pid;
	while(1) {
		pid = waitpid(-1, &wstatus, WNOHANG);
		if(pid == 0) {
			printf("process did not change state\n");
			printf("wstatus: %i\n", wstatus);
			break;

		} else if(pid == -1) {
			printf("there was an error\n");
			printf("wstatus: %i\n", wstatus);
			break;

		} else {
			printf("pid %i changed state\n", pid);
			printf("wstatus: %i\n", wstatus);
		}

		//if(WIFEXITED(wstatus) == 1) { 
		//	printf("process terminated normally\n");
		//}
	}
}

