/*
 * list.c - implementation of the integer list functions
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "list.h"



list_t* lst_new()
{
   list_t *list;
   list = (list_t*) malloc(sizeof(list_t));
   list->first = NULL;
   list->total_exec_t=0;
   return list;
}


void lst_destroy(list_t *list)
{
	struct lst_iitem *item, *nextitem;

	item = list->first;
	while (item != NULL){
		nextitem = item->next;
		free(item);
		item = nextitem;
	}
	free(list);
}


void insert_new_process(list_t *list, int pid, time_t starttime)
{
	lst_iitem_t *item;

	item = (lst_iitem_t *) malloc (sizeof(lst_iitem_t));
	item->pid = pid;
	item->starttime = starttime;
	item->endtime = 0;
	item->status = 0;
	item->next = list->first;
	list->first = item;
}


void update_terminated_process(list_t *list, int pid, time_t endtime, int status)
{
	lst_iitem_t *item = list->first;

	while(item != NULL){
		if(item->pid==pid){
			item->endtime = endtime;
			item->status = status;
			return;
		}

		item=item->next;
	}
	printf("list.c: update_terminated_process() error: pid %d not in list.\n", pid);
}

//Imprimir o que ja imprimia anteriormente

void lst_print(list_t *list, FILE *fp, int iteracao, int pid, int totFile)
{
	lst_iitem_t *item;
  int value = 0;
  item = list->first;

  while(item != NULL){
    if(item->pid==pid){
      value = item->endtime - item->starttime;

	    fprintf(fp, "iteracao %d\n", iteracao);
	    // printf("pid2: %d \n", item->pid);
		 	fprintf(fp, "pid: %d execution time: %d s\n", pid , value);
      break;
    }
    item=item->next;
  }

	
  list->total_exec_t = (list->total_exec_t + value + totFile);
  //printf("T2: %d\n", list->total_exec_t);
	fprintf(fp, "total execution time: %d s\n", list->total_exec_t);
	fflush(fp);
}

void lst_print2(list_t *list)
{
	lst_iitem_t *item;

	printf("Listing PIDs, Status and Execution time:\n");
	item = list->first;

	while (item != NULL){
    if(WIFEXITED(item->status))
      printf("pid: %d exited normally; status=%d.", item->pid, WEXITSTATUS(item->status));
    else
      printf("pid: %d terminated without calling exit.", item->pid);
    printf(" Execution time: %d s\n", (int)(item->endtime - item->starttime));

    item = item->next;
  }
  
  printf("End of list.\n");
}