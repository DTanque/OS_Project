#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "list.h"

#define MAX_BUF 1024

int tot_exec_t = 0;

FILE *fp;

list_t *list;

int main(int argc, char *argv[]){

	list = lst_new();

	int fdout;
	char str[1024];
	char * myfifo = argv[1];

	/* write "Hi" to the FIFO */
	fdout = open(myfifo, O_WRONLY);
	
	int hello = 0;
	int over = 0;

	if(hello==0){
		int mypid = getpid();
		char pidChar[50];
		snprintf(pidChar, 50 ,"token76447_Hello: %d \n", mypid);
		write(fdout, pidChar , strlen(pidChar));
		hello++;
	}

	while(fgets (str, sizeof(str)-1, stdin) != NULL && open(myfifo, O_WRONLY)!=-1){
		

		printf("%s",str);
		if(strncmp(str, "exit-global", 11) == 0){
			//O programa par-shell-terminal deve aceitar o comando exit que permite sair do programa
			//par-shell-terminal.
			if(over==0){
				int mypid = getpid();
				char pidChar[50];
				snprintf(pidChar, 50 ,"token76447_Over: %d \n", mypid);
				write(fdout, pidChar , strlen(pidChar));
				write(fdout, str , strlen(str));
				close(fdout);
				over++;
			}
			break;
		}else if(strncmp(str, "exit", 4) == 0){
			
			break;
		}else if(strncmp(str, "stats", 5) == 0){
			write(fdout, str , strlen(str));

			int fdExtra;
			char * myfifo2 = "/tmp/stats-fifo-76447"; 
			unlink(myfifo2);
			mkfifo(myfifo2, 0776);
		    char buf[MAX_BUF];

		    /* open, read, and display the message from the FIFO */
		    fdExtra = open(myfifo2, O_RDONLY);
		    read(fdExtra, buf, MAX_BUF);
		    printf("Existem %s processos em ligados ao par-shell\n", buf);
		    close(fdExtra);

			char buffE[100];

			fp = fopen("log.txt", "a+");
			if(fp == NULL){
				fprintf(stderr, "Error - Fail to open file");
				exit(EXIT_FAILURE);
			}

			while(fgets(buffE, 500, fp) != NULL){
				sscanf(buffE, "total execution time: %d s\n", &tot_exec_t);
			}

			fclose(fp);

			printf("Tempo de execucao: %d \n", tot_exec_t);

		}else{
			write(fdout, str , strlen(str));
		}

	}if(open(myfifo, O_WRONLY)==-1){
		printf("Main was shutdown, so do I...\n");

		close(fdout);

		printf("All Done. Bye!\n");
		return 0;
		//break;
	}

	close(fdout);

	printf("All Done. Bye!\n");
	return 0;
}


