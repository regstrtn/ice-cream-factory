#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/shm.h>
#include "listrecipe.c"

/******************************************************************
 * Filename: 
 * Created by: Mohammad Luqman
 *
 *
 * ****************************************************************/

char *machinenames[] = {"boil", "mix", "wrap", "freeze"};
int nummachines = 4; //Later have this picked up programmatically
int machineinstances[4] = {2, 3, 2, 3};
int numinstances = 10;
char *semnames[] = {"/boil", "/mix", "/wrap", "/freeze"};

int populateq(job** qlist, int qinfo[]) {
	FILE *fpjob = fopen("job2.info", "r");
  int i = 0;
	for(i=0;i<10;i++) {
		qinfo[i] = 0;
		printf("FrontRear: %d ", qinfo[i]);
	}
	//qlist[0].totaltasks = 100;
	// strcpy(qlist->name, "dafuq");
	listjobs(fpjob, qlist, qinfo);
  //for(i=0;i<3;i++) printf("%s\n", qlist[i].name);
	//printf("First job name: %d\n", qlist[0].totaltasks );
}


int startmachine(job* job_child, int *statusvar,int instance, char *sem_name, int qinfo_child[],  char *machinename, job* partialq) {
	usleep(10000);
	int j = getqueuenum(machinename);
	//*(statusvar+j) = 0; 
	//lr_init();
	int i = 0;
	for(i=0;i<10;i++) { 
	//		qinfo_child[i] = 0;
	//		printf("%d\n", qinfo_child[i]);
	}
	int * sem_q = sem_open(sem_name, 0);
	sem_wait(sem_q);
	printf("Instance: %d. Machine: %s. Entered critical section.", instance, machinename);
	sleep(1);
	//for(i=0;i<10;i++) printf("Qinfo %d: %d\n", i, qinfo_child[i]);
	popq(job_child, &qinfo_child[0], &qinfo_child[5]);
	//printf("qinfo_child qinfo_child+5 %d %d\n", *qinfo_child, *(qinfo_child+5));
	 *(statusvar)=1;
	printf(" Exited critical section\n");
	fflush(NULL);
	sem_post(sem_q);
	exit(0);
}

int pollchildren(job* qlist[], int *statusarr, int instance, int qinfo[], job* partialq){
	int i = 0;
	printf("Instances: %d\n", instance);
	for(i=0;i<nummachines;i++) {
		printf("Status of %d is now %d\n", i, *(statusarr+i));
	}
	sleep(4);
	for(i=0;i<nummachines;i++) {
		printf("Status of %d is now %d\n", i, *(statusarr+i));
	}
	return 0;
}

int buildjobqs() {
	int shmid[nummachines], sh_status, sh_qinfo;
  int	*statusarr, *statusvar, *qinfo, *qinfo_child, *b;
  int i=0, j=0, k=0, instance=0, a;
	job *qlist[nummachines];
	job *job_child, *job_parent;
	task *tasklist;
	int pid;
	//Variables for partially completed queue
	int partial_id;
	job *partial_parent, *partial_child;
	sem_t *partial_sem;
	partial_sem = sem_open("/partial", O_CREAT|O_EXCL, 0644, 1);
	perror("semopen: ");
  partial_id = shmget(IPC_PRIVATE, 5*sizeof(job), 0666|IPC_CREAT);
	partial_parent = (job *)shmat(partial_id, 0, 0);
	//int sem_q[nummachines];
	//Get semaphores
	//for(i=0;i<nummachines;i++) {
	int*	sem_q[4];
  sem_unlink("/boil"); sem_unlink("/mix"); sem_unlink("/wrap"); sem_unlink("/freeze"); sem_unlink("/partial");
	for(i=0;i<nummachines;i++) {
				sem_q[i] =  sem_open(semnames[i], O_CREAT|O_EXCL, 0644, 1);
				if(errno == EEXIST) { sem_unlink(semnames[i]); sem_q[i] =  sem_open(semnames[i], O_CREAT|O_EXCL, 0644, 1);}
				perror("semopen: ");
	}
	//}	
	//Allocate shared memory
	sh_status = shmget(IPC_PRIVATE, numinstances*sizeof(int), 0777|IPC_CREAT); //Get memory for polling array
	statusarr = (int*)shmat(sh_status, 0,0); //Attach array that will be polled
	sh_qinfo = shmget(IPC_PRIVATE, 10*sizeof(int), 0777|IPC_CREAT); //Required for front and rear of queues
	qinfo = (int*)shmat(sh_qinfo, 0, 0);
	i = 0;
	for(i=0;i<nummachines;i++) {	
		shmid[i] = shmget(IPC_PRIVATE,5*sizeof(job), 0666|IPC_CREAT);
		if(shmid[i]<0) printf("shmid %d failed\n", i);
		qlist[i] = (job*)shmat(shmid[i], 0, 0); //attach queue memory to parent
	}
	//job_parent[0].totaltasks = 100;
	//strcpy(qlist[0][0].name, "dafuq");
	if(sh_status <0) printf("sh_status failed\n");
  if(sh_qinfo <0) printf("sh_qinfo failed\n");
 	if(partial_id<0) printf("partial_id failed\n");

	printf("From calling function: %d\n", errno);
	populateq(qlist, qinfo);
	for(i=0;i<nummachines;i++) {
		for(j=0;j<machineinstances[i];j++){
			if((pid=fork())==0) {
				//sleep(3);
				job_child = (job*)shmat(shmid[i],0,0); //attach queue memory to child
			  statusvar = (int*)shmat(sh_status, 0, 0);
				qinfo_child = (int*)shmat(sh_qinfo, 0, 0);
				partial_child = (job *)shmat(partial_id, 0, 0);
				//printf("Can i access qinfo from here:%d ", qinfo_child[instance]);
				startmachine(job_child, statusvar, instance, semnames[i], qinfo_child, machinenames[i], partial_child);	
			} instance++;
		}
	}	
		i = 0;
		printf("Parent here\n");
		pollchildren(qlist, statusarr, instance, qinfo, partial_parent);
		for(i=0;i<nummachines;i++)	{
			shmctl(shmid[i], IPC_RMID, 0);
			sem_unlink(semnames[i]);
		}
		sem_unlink("/partial");
		shmctl(sh_status, IPC_RMID, 0);
		shmctl(sh_qinfo, IPC_RMID, 0);
}

void sighandler(int sig_num) {
	int i = 0;
	printf("SIGINT handler called");
	for(i=0;i<nummachines;i++) {
		sem_unlink(semnames[i]);
		perror("Signal handler. sem_unlink: ");
	}
	sem_unlink("/partial");
	exit(0);
}

int main(int argc, char **argv){
  signal(SIGINT, sighandler);
	buildjobqs();
	return 0;
}
