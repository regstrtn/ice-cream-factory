#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <semaphore.h>

/******************************************************************
 * Filename: 
 * Created by: Mohammad Luqman
 *
 *
 * ****************************************************************/

char *semnames[] = {"/boil", "/mix", "/wrap", "/freeze", "/partial"};

int main(int argc, char **argv){
	int i = 0;
	for(i=0;i<5;i++) 
					sem_unlink(semnames[i]);
	perror("sem_unlink: ");
	return 0;
}
