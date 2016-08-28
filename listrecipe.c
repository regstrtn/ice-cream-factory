#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#define QLEN 500

/******************************************************************
 * Filename: listrecipe.c 
 * Created by: Mohammad Luqman
 *
 *
 * ****************************************************************/

//Most functions in this file are not used anymore, but still, don't delete anything. 

//Mention global variables that were defined in process.c file
extern char machinenames[100][100];
extern int nummachines; //Later have this picked up programmatically
extern int machineinstances[100];
extern int numinstances;
extern int numtasks;
extern char semnames[100][100];
extern int timereq[100];
extern char tasks[100][100];

typedef struct jobstruct {
	char name[250];
	char actlist[100][250];
	char actremain[100][250];
	char machineorder[100][250];
	char taskorder[100][250];
	int totaltasks;
	int currenttasknum;
  int inuse;	
} job;

typedef struct taskvariantstruct {
	char taskname[250];
	char machine[250];
	int timereq;
	int machinecount;
} task;

typedef struct machinestruct {
	 char name[250];
	 int instances;
} machine;

typedef struct queuenode {
  job jobnode;
  struct queuenode* next;
} node;


//Declare functions
void insertq(job *jq, job a, int *front, int *rear);
void printq(job *jq, int front, int rear);
job popq(job *jq, int *front, int *rear);

int getnummachines(char *taskfile) {
 FILE *fptask = fopen(taskfile, "r");
 fseek(fptask, 0, SEEK_SET);
 char *buffer = NULL;
 int bufsize = 0;
 int c;
 int machines = 0;
 while((c=getline(&buffer, &bufsize, fptask))>0)machines++;
 return machines;
}

//char* machinenames[4] = {"boil", "mix", "wrap", "freeze"};
//int front[4] = {0};
//int rear[4] = {0};

int isempty(int *front, int *rear) {
	if((*front)%QLEN == (*rear+1)%QLEN) return 1;
	if(*front == *rear) return 1;
	else return 0;
}

int getqueuenum(char *targetmachine) {
	int i = 0;
	for(i=0;i<nummachines;i++) {
		if(strcmp(targetmachine, machinenames[i])==0) return i;
	}
}

int gettasknum(char *taskname) {
	int i = 0;
	for(i=0;i<numtasks;i++) {
		if(strcmp(taskname, tasks[i])==0) return i;
	}
}

void printq(job *jq, int front, int rear) {
	int i = 0;
	i = front;
	for(i=front;i<rear;i++) printf("JobName: %s ", jq[i].name);
}

void insertq(job *jq, job a, int *front, int *rear) {
	int i;
  //printf("Insertq called. Parameters: %d %d %d. ", *front, *rear, QLEN);
	if((*front)%QLEN == (*rear + 1)%QLEN) printf("Queue overflow(rear)\n");
	else {
		jq[*rear] = a;
		*rear = (*rear + 1)%QLEN;
	}
}

job popq(job *jq, int *front, int *rear) {
 int i = *front;
 job element;
 if(*front==*rear) printf("Queue empty (front)\n");
 else {
	 	element = jq[*front];
		*front = (*front + 1)%QLEN;
		//printf("Popped: %s\n", element.name);
		//printf("Front: %d Rear: %d\n", *front, *rear);
		return element;
 } 
}

int gettasktime(char *taskname) {
	int i = 0, j = 0;
	for(i=0;i<numtasks;i++) {
		if(strcmp(taskname, tasks[i])==0) return timereq[i];
	}
}

//Read slave.info file and store machine details, tasknames, and time requirements of each task. Also define semaphore names for each machine. 
int getmachinelist(char *taskfile, char machinenames[][100], int machineinstances[], char tasks[][100], int timereq[], int *numtasks, char semnames[][100]) {
				FILE *fptask = fopen(taskfile, "r");
				fseek(fptask, 0, SEEK_SET);
				char *buffer = NULL;
				size_t bufsize = 0;
				char s[99], m[97], sem[99];
				int bytesread = 0, bytesnow, charsread = 0;
				int machinecount = 0, tasktime=0, totalinstances = 0;
				//struct machinestruct * machinelist = malloc(100*sizeof(machine));
				int i = 0, j = 0, k = 0, r = 0, inst = 0;
				while(getline(&buffer, &bufsize, fptask)>0) {
					sscanf(buffer, "%s %d%n", m, &machinecount, &bytesnow);
					//printf("Pre Sscanf output: %s %d\n", m, machinecount);
				  strcpy(machinenames[i], m);
					strcpy(sem, "/"); strcat(sem, m); strcpy(semnames[i], sem); //Semaphore name needs to start with forward slash
					machineinstances[i] = machinecount;
					bytesread = bytesnow;	
					//Need to tell sscanf where to start reading the buffer from. %n at the end stores bytes read in &bytesnow
					while((charsread = sscanf(buffer+bytesread, "%s %d%n", s, &tasktime, &bytesnow))>0){  
								//printf("Sscanf output: %s %d\n", s, timereq);
								//printf("J: %d \n", j);
								strcpy(tasks[j], s);
								timereq[j] = tasktime;
								bytesread+=bytesnow;
								j++;
					} i++;
				}
			  *numtasks = j; //Number of task variants
			  for(j=0;j<i;j++) inst+=machineinstances[j]; 
				return inst; //Return total number of instances of all machines put together
}

//I think this function never gets called, but I am not sure. 
task* gettasklist(FILE *fptask) {
				fseek(fptask, 0, SEEK_SET);
				char *buffer = NULL;
				size_t bufsize = 0;
				char s[1000], machine[1000];
				int bytesread = 0, bytesnow, charsread = 0;
				int machinecount = 0, timereq = 0;
				task *tasklist = (task*)malloc(100*sizeof(task));
				int i = 0, j = 0;
				while(getline(&buffer, &bufsize, fptask)>0) {
					sscanf(buffer, "%s %d%n", machine, &machinecount, &bytesnow);
					//printf("Pre Sscanf output: %s %d\n", s, machinecount);
					bytesread = bytesnow;	
					while((charsread = sscanf(buffer+bytesread, "%s %d%n", s, &timereq, &bytesnow))>0){ 
								//printf("Sscanf output: %s %d\n", s, timereq);
								strcpy(tasklist[j].taskname,s);
								tasklist[j].timereq = timereq;
								strcpy(tasklist[j].machine, machine);
								tasklist[j].machinecount = machinecount;
								//printf("%s ", s);
								bytesread+=bytesnow;
								j++;
					}
				}
				for(i=0;i<j;i++) ;
				//printf("Task variants: %s %s %d %d\n", tasklist[i].taskname, tasklist[i].machine, tasklist[i].timereq, tasklist[i].machinecount);
	return tasklist;
}

//Read job.info file and insert each job in appropriate queue
int listjobs(FILE *fpjob, job* joblist[], int qinfo[], int jobstoperform) {
				char *buffer = NULL;
				size_t bufsize = 0;
				char s[1000];
				char jobname[1000];
				char *targetmachine, *taskv;
				int queuenum;
			  int front[nummachines+1];
				int rear[nummachines+1];
				int k = 0;
				//For loop below is required because variable sized objects cannot be initialised. 
				for(k=0;k<nummachines+1;k++) {
					front[k] = 0;
					rear[k] = 0;
				}
				int bytesread = 0, bytesnow, charsread = 0;
				int totaljobs = 0;
				//job *joblist = (job*)malloc(100*sizeof(job));
				int i = 0, j = 0;
				//printf("Total jobs to be performed: %d\n", jobstoperform);
				while(jobstoperform>0) {
								fseek(fpjob, 0, SEEK_SET);
				while(getline(&buffer, &bufsize, fpjob)>0) {
					totaljobs++;
					sscanf(buffer, "%s%n", jobname, &bytesnow);	
				  //printf("Jobname: %s ", jobname);
					//strcpy(joblistqueuenum[i].name, s);
					bytesread = bytesnow;
					j = 0;	
					sscanf(buffer+bytesread, "%s%n", s, &bytesnow);
					targetmachine = strtok(s,":");
					queuenum = getqueuenum(targetmachine);
					//printf("Queuenum for targetmachine %s: %d\n", targetmachine, queuenum);
					while((charsread = sscanf(buffer+bytesread, "%s%n", s, &bytesnow))>0){ 
								strcpy(joblist[queuenum][rear[queuenum]].actlist[j],s);
								strcpy(joblist[queuenum][rear[queuenum]].actremain[j],s);
								joblist[queuenum][rear[queuenum]].currenttasknum = 0;
								targetmachine = strtok(s, ":");
								strcpy(joblist[queuenum][rear[queuenum]].machineorder[j],targetmachine);
								taskv = strtok(NULL, ":");
								strcpy(joblist[queuenum][rear[queuenum]].taskorder[j],taskv);
								//printf("%s ", s);
								bytesread+=bytesnow;
								j++;
					}
				  strcpy(joblist[queuenum][rear[queuenum]].name, jobname);	
					//printf("queuenum: %d jobname: %s I: %d\n", queuenum, jobname, rear[queuenum]);
					joblist[queuenum][rear[queuenum]].totaltasks = j;
					rear[queuenum] = rear[queuenum]+1;
					//if(i>3) break;
					//Error: I should not be incremented every time, fix this.
					jobstoperform--; if(jobstoperform<=0) break;
					//printf("Jobs Read: %d Left: %d\n", totaljobs, jobstoperform);
				}
				}
				for(i=0;i<nummachines;i++) {
					qinfo[i+1+nummachines] = rear[i];
					//printf("Rear queue: %d Totaljobs to perform: %d\n", qinfo[i+1+nummachines], totaljobs);
				}
			//	exit(0);
			return totaljobs;	
				//printf("Jobnames: %s Activity list: %s Totaltasks: %d Thirdtask: %s\n", joblist[i].name, joblist[i].actlist[2], joblist[i].totaltasks, joblist[i].actremain[2]);
	//return joblist;
}

/*
void createq(job* joblist) {
  printf("CreateQ was called\n");	
	int i = 0;
	for(i=0;i<10;i++) {
					insertq(joblist[i%2]);
	}
	printq();
}*/

//Main function uncomment below

/*
int main() {
	int i = 0;
	FILE *fpjob	= fopen("job2.info", "r+");
	FILE *fptask = fopen("slave2.info", "r+");
	//job* joblist = listjobs(fpjob);
	//task *variants = gettasklist(fptask);
	//machine *machinelist = getmachinelist(fptask);
	job *qlist[4];
	for(i=0;i<4;i++) {
	  qlist[i] = (job*)malloc(5*sizeof(job));	
	}
	listjobs(fpjob, qlist);
	//createq(joblist);
	return 0;
}*/
