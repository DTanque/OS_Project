#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>


#include <fcntl.h>
#include <sys/stat.h>

#include "commandlinereader.h"
#include "list.h"

#define MAX_ARGS 7
#define MAXPAR 2

#define MAX_BUF 1024

void *routine_functionMonit( void *ptr );
void mutex_unlock();
void mutex_lock();

int processosFilho=0;
list_t *list;

pthread_mutex_t count_mutex;

pthread_cond_t vazio, cheio;
//consumer- MonitThread;
//producer - Father;

int exitFlag = 0;
int flagData=0;

FILE *fp;
FILE *fp1; //filho
int iteracao = 0;
int tot_exec_t = 0;
char buff[100];
char buffPID[100];

char * myfifo = "/tmp/par-shell-in-76447";

int ligacoesEstab=0;
int *pid_arrTerminals;


void sigint_handler(int);
void send_signal (int pid);

int main(int argc, char *argv[]) {

	pid_arrTerminals = (int *)malloc(ligacoesEstab * sizeof(int));
	
	int i;

	int pid;
	char* argVector[MAX_ARGS];
	time_t start_time;

	list = lst_new();

	pthread_t threadMonit;
	int  iretMonit;

	iretMonit = pthread_create(&threadMonit, NULL, routine_functionMonit, NULL);
	if(iretMonit){
  		fprintf(stderr,"Error - pthread_create() return code: %d\n",iretMonit);
    	exit(EXIT_FAILURE);
  	}

	if(pthread_mutex_init(&count_mutex, NULL) != 0) {
		fprintf(stderr, "Error creating mutex.\n");
		exit(EXIT_FAILURE);
	}

	
	if(pthread_cond_init(&cheio, NULL) != 0){
		fprintf(stderr, "Error creating pthread_cond_init.\n");
		exit(EXIT_FAILURE);
	}
	if(pthread_cond_init(&vazio, NULL) != 0){
		fprintf(stderr, "Error creating pthread_cond_init.\n");
		exit(EXIT_FAILURE);
	}


	//---------------------OPEN FILE------------
	mutex_lock();
	fp = fopen("log.txt", "a+");
	if(fp == NULL){
		fprintf(stderr, "Error - Fail to open file");
		exit(EXIT_FAILURE);
	}

	while(fgets(buff, 500, fp) != NULL){
		sscanf(buff, "iteracao %d\n", &iteracao);
		sscanf(buff, "total execution time: %d s\n", &tot_exec_t);
		flagData=1;
	}
	mutex_unlock();

	//------------------------PIPE---------------------------
	
	
	int fd;

	unlink(myfifo);
    /* create the FIFO (named pipe) */
    mkfifo(myfifo, 0777);

    /* open, read, and display the message from the FIFO */
    fd = open(myfifo, O_RDONLY);	//faz open da fila, para leitura
    
    close(0);   // fecha o stdin
    dup(fd);   // make pipe go to stdin
    //read(fd, buf, MAX_BUF);			//le do fd (no max 1024 caract = MAX_BUF) e guarda no buf
    

	//-------------------------------------------------------

	signal(SIGINT, sigint_handler);


	while(1){
		   
		i = readLineArguments(argVector,MAX_ARGS);




		if(i==0){
			perror("\nError: No input inserted!\n");
		}else if(i==-1){
			
			if(ligacoesEstab<=0){
				

					int fd;

					unlink(myfifo);
				    /* create the FIFO (named pipe) */
				    mkfifo(myfifo, 0777);

				    /* open, read, and display the message from the FIFO */
				    fd = open(myfifo, O_RDONLY);	//faz open da fila, para leitura
				    
				    close(0);   // fecha o stdin
				    dup(fd);   // make pipe go to stdin
				    //read(fd, buf, MAX_BUF);			//le do fd (no max 1024 caract = MAX_BUF) e guarda no buf
    

			}
		}else{
			
			if(strcmp(argVector[0], "exit-global") == 0){
				mutex_lock();
				exitFlag=1;
				pthread_cond_signal( &cheio );
				if(processosFilho==0){
					sigint_handler(SIGINT);
					mutex_unlock();
					break;
				}
				mutex_unlock();
			}else if(strcmp(argVector[0], "stats") == 0){
				printf("There are %d connections alive\n", processosFilho);
				int fdExtra;

			    char * myfifo2 = "/tmp/stats-fifo-76447"; 
				
			    /* create the FIFO (named pipe) */
			    mkfifo(myfifo2, 0776);

			    /* write "Hi" to the FIFO */
			    fdExtra = open(myfifo2, O_WRONLY);
			    char numConnections[10];
			    sprintf(numConnections, "%d", processosFilho);
			    write(fdExtra, numConnections, sizeof(numConnections));
			    close(fdExtra);

			}else if(strncmp(argVector[0], "token76447_Hello:", 17) == 0){

				int pidIntH = atoi(argVector[1]);

				//printf("vai adicionar a list %d \n",pidIntH);

				pid_arrTerminals[ligacoesEstab]=pidIntH;
				ligacoesEstab++;

			}else if(strncmp(argVector[0], "token76447_Over:", 16) == 0){
				
			}else if(strcmp(argVector[0], "exit") == 0){
				//Limpar pipe quando nao ha terminais a correr!!
				if(ligacoesEstab<=0){
					unlink(myfifo);
					    /* create the FIFO (named pipe) */
					    mkfifo(myfifo, 0777);

					    /* open, read, and display the message from the FIFO */
					    fd = open(myfifo, O_RDONLY);	//faz open da fila, para leitura
					    
					    close(0);   // fecha o stdin
					    dup(fd);   // make pipe go to stdin
					    //read(fd, buf, MAX_BUF);			//le do fd (no max 1024 caract = MAX_BUF) e guarda no buf
				}
			}else{

				/* acquire the empty lock */
				mutex_lock();
				while(processosFilho==MAXPAR){
      				pthread_cond_wait(&vazio, &count_mutex);
				}
				mutex_unlock();
				start_time = time(NULL);
				pid = fork();
				if(pid==0){
					//Kid Code
					int kidPid = getpid();

					char buffer[12];
					snprintf(buffer, 12,"%d",kidPid);

					char strN[80];
					strcpy (strN,"par-shell-out-");
					strcat (strN, buffer);
					strcat (strN,".txt");
					//puts (str);

					fflush(stdout);
					fp1 = fopen(strN, "a+");
					
					dup2(fileno(fp1), 1);   // make stdout go to file
    				dup2(fileno(fp1), 2);   // make stderr go to file - you may choose to not do this
                   // or perhaps send stderr to another file
			

					if(execv( argVector[0], argVector) == -1){
						perror("\nError: Bad execv setup\n");
						exit(EXIT_FAILURE);
					}

					fflush(stdout);
					close(fileno(fp1));

				}else if(pid==-1){
					//Error
					perror("\nError: Unable to fork\n");
				}else{
					//Father's Code
					
					mutex_lock();
					processosFilho++;
					insert_new_process(list,pid,start_time);
		    		pthread_cond_signal( &cheio );
					mutex_unlock();
				}
								
			}
		}
   	}			

   	close(fd);
	unlink(myfifo);
	lst_print2(list);
	
	pthread_cond_signal( &cheio );
	
	if(pthread_join(threadMonit,NULL) !=0){
		perror("\nERROR: Fail to join threads\n");
		exit(-1);
	}
	
	//join - bloqueia a espera da terminaçao
	if(pthread_mutex_destroy(&count_mutex) != 0){
		perror("\nError: Fail to destroy mutex\n");
		exit(EXIT_FAILURE);
	}
	lst_destroy(list);
	
	if(pthread_cond_destroy(&cheio) != 0){
		perror("\nError: Fail to destroy cheiol\n");
		exit(EXIT_FAILURE);
	}
	if(pthread_cond_destroy(&vazio) != 0){
		perror("\nError: Fail to destroy vazio\n");
		exit(EXIT_FAILURE);
	}	
	
	if(fclose(fp) != 0){
		perror("\nError:Fail to close file\n");
		exit(EXIT_FAILURE);
	}
	printf("All Done. Bye!\n");
	return 0;
}


/* 
+-----------------------------------------------------------------------*/

void *routine_functionMonit( void *ptr ) {
	int k;

	int status;
	time_t end_time;

	//ponteiro que serviram como arrays de alocacao
	// de forma dinamica para guardar o pid e o
	// status de cada processo filho
	int *pid_arr;
	int *stat_arr;

	//esperar filhos para fazer wait e assim que ha filho faz wait
    while(1){

		mutex_lock();
    	while(processosFilho==0 && exitFlag==0){
    		pthread_cond_wait(&cheio, &count_mutex);
		}
		mutex_unlock();

		mutex_lock();
    	if(processosFilho==0 && exitFlag==1){			
    			break;    		
    	}else{
    		mutex_unlock();
	    	pid_arr = (int *)malloc(processosFilho * sizeof(int));
			stat_arr = (int *)malloc(processosFilho * sizeof(int));

			
			for(k=0;k<processosFilho;k++){
				pid_arr[k]=wait(&status);

				if(WIFEXITED(status)){
					stat_arr[k] = WEXITSTATUS(status);
					
					end_time = time(NULL);
					

					mutex_lock();
					update_terminated_process(list,pid_arr[k], end_time ,stat_arr[k]);
					processosFilho--;
					mutex_unlock();

					mutex_lock();
					if(flagData==1){
						iteracao++;
						flagData=0;
					}
					lst_print(list, fp, iteracao, pid_arr[k], tot_exec_t);
					tot_exec_t=0;
					iteracao++;
					pthread_cond_signal( &vazio );
					mutex_unlock();
				}
			}

    	}
    }
    mutex_unlock();
    return NULL;
}


/* 
+-----------------------------------------------------------------------*/
void mutex_lock() {
  if(pthread_mutex_lock(&count_mutex) != 0)
  {
    fprintf(stderr, "Error in pthread_mutex_lock()\n");
    exit(EXIT_FAILURE);
  }
}


/* 
+-----------------------------------------------------------------------*/
void mutex_unlock() {
  if(pthread_mutex_unlock(&count_mutex) != 0){
    fprintf(stderr, "Error in pthread_mutex_unlock()\n");
    exit(EXIT_FAILURE);
  }
}


/* 
+-----------------------------------------------------------------------*/

 void sigint_handler(int sig)
{
	unlink(myfifo);
    /*do something*/
	int contador=0;
//	while(processosFilho>0){}
	for(contador=0;contador<ligacoesEstab;contador++){
		if(pid_arrTerminals[contador]!=0){
			printf("killing process Par-Shell-Terminal with PID: %d\n",pid_arrTerminals[contador]);
			send_signal(pid_arrTerminals[contador]);
		}
	}

    printf("killing process Par-Shell with PID: %d\n",getpid());
    exit(0);
}

void send_signal (int pid)
{
        int ret;
        ret = kill(pid,SIGHUP);
        //printf("\nret : %d\n",ret);

}