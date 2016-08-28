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
#include "unittest.c"

/******************************************************************
 * Filename: process.c 
 * Created by: Mohammad Luqman
 * This is the master file which creates all sorts of delicious ice-cream flavors.
 * A randomized approach has been used to implement scheduling. 
 *
 * ****************************************************************/

//Declare global variables. Have main function fill these up from slave and job files

char machinenames[100][100];
int nummachines = 0; 
int machineinstances[100];
int numinstances = 0;
int numtasks = 0;
char semnames[100][100];
int timereq[100];
char tasks[100][100];
int jobsdone = 0;
int jobstoperform = 0;
int tasksdone[100] = {0};
char taskfile[100], jobfile[100]; //Not really required to be global, just being lazy

//Parent sends SIGINT to child processes. Before exiting, print out task summary of each child.
void sighandler(int sig_num) {
	int i = 0;
	printf("%d %d ", getpid(), jobsdone);
	for(i=0;i<numtasks;i++) {
		if(tasksdone[i]>0) {
			printf("%s %d ", tasks[i], tasksdone[i]);
		}
	}
	printf("\n");
	for(i=0;i<nummachines;i++) {
		sem_unlink(semnames[i]);
		//perror("Signal handler. sem_unlink: ");
	}
	sem_unlink("/partial");
	exit(0);
}

//This function calls the listjobs function from listrecipe.c file and picks up all the jobs
int populateq(job** qlist, int qinfo[]) {
	FILE *fpjob = fopen(jobfile, "r");
  int i = 0;
	int totaljobs = 0;
	for(i=0;i<2*nummachines+2;i++) {
		qinfo[i] = 0;
	}
	//qlist[0].totaltasks = 100;
	// strcpy(qlist->name, "dafuq");
	totaljobs = listjobs(fpjob, qlist, qinfo, jobstoperform);
	for(i=0;i<nummachines+1;i++) {
		//printf("Front Rear: %d %d", qinfo[i], qinfo[5+i]);
	}
	fflush(NULL);
	return totaljobs;
	//for(i=0;i<3;i++) printf("%s\n", qlist[i].name);
	//printf("First job name: %d\n", qlist[0].totaltasks );
}

//Each child process takes its machine name as argument and performs the tasks required
int startmachine(job* job_child, int *statusvar,int instance, char *sem_name, int qinfo_child[],  char *machinename, job* partialq) {
	signal(SIGINT, sighandler);
	int j = getqueuenum(machinename);
	*(statusvar+j) = 0;
	int i = 0;
	int qempty = 1;
	int * sem_q = sem_open(sem_name, 0);
	int *semp = sem_open("/partial", 0);
	int tasktype = 0;
	printf("%s %d\n", machinename, getpid());
	while(1) {     //Keep waiting till parent sends SIGINT
		sem_wait(sem_q);
	  if((qempty = isempty(&qinfo_child[j], &qinfo_child[nummachines+1+j])==1)) { 
										sem_post(sem_q);
										sleep(1);
										continue;	
		}
		else {
			job currjob = popq(job_child, &qinfo_child[j], &qinfo_child[nummachines+1+j]); //Pop job from the front of the queue
			printf("%s %s:%s %d", currjob.name, machinename, currjob.taskorder[currjob.currenttasknum], getpid());
			tasktype = gettasknum(currjob.taskorder[currjob.currenttasknum]);
			usleep(timereq[tasktype]*1000); //Sleep for required time
			tasksdone[tasktype]++;
			currjob.currenttasknum++;
			if(currjob.currenttasknum==currjob.totaltasks) printf(" %d finished\n", currjob.totaltasks-currjob.currenttasknum);
			else printf(" %d waiting \n",currjob.totaltasks-currjob.currenttasknum);
			while(isfull(&qinfo_child[nummachines], &qinfo_child[2*nummachines+1])) usleep(100*1000);
			insertq(partialq, currjob, &qinfo_child[nummachines], &qinfo_child[2*nummachines+1]);
			statusvar[j] = statusvar[j]+1;
			//*(statusvar+j)++;
			//printf(" Exited critical section\n");
			fflush(NULL);
			sem_post(sem_q);
			jobsdone++;
			usleep(rand()*100/RAND_MAX); //For uniform scheduling, make child sleep for a random time after completion
	//	break;
		}
	}
	exit(0);
}

//Check status of each machine periodically for completed tasks
int pollchildren(job* qlist[], int *statusarr, int instance, int qinfo[], job* partialq, sem_t *partial_sem, int totaljobs){
	job partialjob;
	int finishedjobs = 0;
	int i = 0, j = 0, qnum, finish = 0, tasksdone = 0;
	while(finishedjobs<totaljobs) {
		for(i=0;i<nummachines;i++) {
			if(*(statusarr+i)>=1) {
				tasksdone++;
				sem_wait(partial_sem);
				partialjob = popq(partialq, &qinfo[nummachines], &qinfo[2*nummachines+1]);
				sem_post(partial_sem);
			  qnum = getqueuenum(partialjob.machineorder[partialjob.currenttasknum]);
				if(partialjob.currenttasknum < partialjob.totaltasks) {
					
						while(isfull(&qinfo[qnum], &qinfo[qnum+nummachines+1])) usleep(100*1000);
						insertq(qlist[qnum],partialjob, &qinfo[qnum], &qinfo[qnum+nummachines+1]);
				}
				else {
								finish = 1; finishedjobs++;
				}
				statusarr[i] = statusarr[i]-1;
				//*(statusarr+i)--;
			}
		} j++; //if(j>10) break; if(finish ==1 ) break;
			usleep(1000*100);
			//printf("Total jobs: %d finished: %d tasksdone: %d\n", totaljobs, finishedjobs, tasksdone);
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
	int pid[numinstances];
	int totaljobs = 0;
	//Variables for partially completed queue
	int partial_id;
	job *partial_parent, *partial_child;
	sem_t *partial_sem;
	partial_sem = sem_open("/partial", O_CREAT|O_EXCL, 0644, 1);
	//perror("semopen: ");
  partial_id = shmget(IPC_PRIVATE, QLEN*sizeof(job), 0666|IPC_CREAT);
	partial_parent = (job *)shmat(partial_id, 0, 0);
	//int sem_q[nummachines];
	//Get semaphores
	//for(i=0;i<nummachines;i++) {
	int*	sem_q[nummachines];
  sem_unlink("/boil"); sem_unlink("/mix"); sem_unlink("/wrap"); sem_unlink("/freeze"); sem_unlink("/partial");
	for(i=0;i<nummachines;i++) {
				sem_q[i] =  sem_open(semnames[i], O_CREAT|O_EXCL, 0644, 1);
				if(errno == EEXIST) { sem_unlink(semnames[i]); sem_q[i] =  sem_open(semnames[i], O_CREAT|O_EXCL, 0644, 1);}
				//perror("semopen: ");
	}
	//}	
	//Allocate shared memory
	sh_status = shmget(IPC_PRIVATE, nummachines*sizeof(int), 0777|IPC_CREAT); //Get memory for polling array
	statusarr = (int*)shmat(sh_status, 0,0); //Attach array that will be polled
	sh_qinfo = shmget(IPC_PRIVATE, (2*nummachines+2)*sizeof(int), 0777|IPC_CREAT); //Required for front and rear of queues
	qinfo = (int*)shmat(sh_qinfo, 0, 0);
	i = 0;
	for(i=0;i<nummachines;i++) {	
		shmid[i] = shmget(IPC_PRIVATE,QLEN*sizeof(job), 0666|IPC_CREAT);
		if(shmid[i]<0) printf("shmid %d failed\n", i);
		qlist[i] = (job*)shmat(shmid[i], 0, 0); //attach queue memory to parent
	}
	//job_parent[0].totaltasks = 100;
	//strcpy(qlist[0][0].name, "dafuq");
	if(sh_status <0) printf("sh_status failed\n");
  if(sh_qinfo <0) printf("sh_qinfo failed\n");
 	if(partial_id<0) printf("partial_id failed\n");

	totaljobs = populateq(qlist, qinfo);
	for(i=0;i<nummachines;i++) {
		for(j=0;j<machineinstances[i];j++){
			if((pid[instance]=fork())==0) {
				//sleep(3);
				job_child = (job*)shmat(shmid[i],0,0); //attach queue memory to child
			  statusvar = (int*)shmat(sh_status, 0, 0);
				qinfo_child = (int*)shmat(sh_qinfo, 0, 0);
				partial_child = (job *)shmat(partial_id, 0, 0);
				//printf("Value of instance passed to child: %d\n", instance);
				//Start child process with the required parameters
				startmachine(job_child, statusvar, instance, semnames[i], qinfo_child, machinenames[i], partial_child);	
			} 
			//printf("Machine: %s PID: %d\n", machinenames[i], pid[instance]);
			instance++;
		} 
	}	
		i = 0;
		pollchildren(qlist, statusarr, instance, qinfo, partial_parent, partial_sem, totaljobs);
		//End everything gracefully after jobs done
		usleep(300*1000);
		for(i=0;i<instance;i++)  kill(pid[i], SIGINT);//kill child process
		for(i=0;i<nummachines;i++)	{
			shmctl(shmid[i], IPC_RMID, 0);
			sem_unlink(semnames[i]);
		}
		sem_unlink("/partial");
		shmctl(sh_status, IPC_RMID, 0);
		shmctl(sh_qinfo, IPC_RMID, 0);
}


int main(int argc, char **argv){
 // signal(SIGINT, sighandler);
  strcpy(taskfile, argv[1]);
	strcpy(jobfile, argv[2]);
	jobstoperform = atoi(argv[3]);
	nummachines = getnummachines(taskfile);
	char l_machinenames[nummachines][100];
  char l_semnames[nummachines][100];
	int l_machineinstances[nummachines];
	int l_timereq[100];
	char l_tasks[100][100];
	int l_numtasks = 0;
	int l_instances =  getmachinelist(taskfile, machinenames, machineinstances, tasks, timereq, &l_numtasks, semnames); 
	numtasks = l_numtasks; //set global variables 
	numinstances = l_instances;
	unittest(nummachines, machinenames, machineinstances, tasks, timereq,numinstances, numtasks, semnames);
	//setglobalvars(nummachines, machinenames, machineinstances, tasks, timereq, numtasks, semnames);
	buildjobqs();
	return 0;
}
