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

int main(int argc, char **argv){
 pid_t pid;
 sem_t *smp1, *smp2;
 smp1 = sem_open("/smp", O_CREAT|O_EXCL, 0644, 1);
 if((pid=fork())==0) {
	smp2 = sem_open("/smp", 0);
	sem_wait(smp2);
	printf("Entered critical section\n");
	sleep(1);
	printf("Exiting critical section\n");
	sem_post(smp2);
 }
 else {
	sem_wait(smp1);
	printf("Parents critical section\n");
	sleep(1);
	printf("Exiting parent\n");
	sem_post(smp1);
 }
	return 0;
}
