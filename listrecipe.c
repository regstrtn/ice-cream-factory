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
 * Filename: 
 * Created by: Mohammad Luqman
 *
 *
 * ****************************************************************/

typedef struct jobstruct {
	char name[250];
	char actlist[100][250];
	char actremain[100][250];
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


extern char *machinenames[];
//char* machinenames[4] = {"boil", "mix", "wrap", "freeze"};
//int front[4] = {0};
//int rear[4] = {0};

int getqueuenum(char *targetmachine) {
	int i = 0;
	for(i=0;i<4;i++) {
		if(strcmp(targetmachine, machinenames[i])==0) return i;
	}
}

void printq(job *jq, int front, int rear) {
	int i = 0;
	i = front;
	for(i=front;i<rear;i++) printf("JobName: %s\n", jq[i].name);
}

void insertq(job *jq, job a, int *front, int *rear) {
	int i;
	if(*rear == QLEN) printf("Queue overflow\n");
	else {
		jq[*rear] = a;
		(*rear)++;
	}
}

job popq(job *jq, int *front, int *rear) {
 int i = *front;
 job element;
 if(*front>=*rear) printf("Queue empty\n");
 //printf("Popq called. Parameters: %d %d. ", i, *rear);
 else {
	 	element = jq[*front];
		(*front)++;
		printf("Popped: %s\n", element.name);
		printf("Front: %d\n", *front);
		return element;
 } 
}


machine* getmachinelist(FILE *fptask) {
				fseek(fptask, 0, SEEK_SET);
				char *buffer = NULL;
				size_t bufsize = 0;
				char s[1000], m[1000];
				int bytesread = 0, bytesnow, charsread = 0;
				int machinecount = 0, timereq = 0;
				struct machinestruct * machinelist = malloc(100*sizeof(machine));
				int i = 0, j = 0;
				while(getline(&buffer, &bufsize, fptask)>0) {
					sscanf(buffer, "%s %d%n", m, &machinecount, &bytesnow);
					//printf("Pre Sscanf output: %s %d\n", m, machinecount);
				  strcpy(machinelist[i].name, m);
					machinelist[i].instances = machinecount;
					bytesread = bytesnow;	
					while((charsread = sscanf(buffer+bytesread, "%s %d%n", s, &timereq, &bytesnow))>0){ 
								//printf("Sscanf output: %s %d\n", s, timereq);
								//printf("%s ", s);
								bytesread+=bytesnow;
								j++;
					} i++;
				}
				for(i=0;i<4;i++) ;
				//printf("Machines: %s %d\n", machinelist[i].name, machinelist[i].instances);
				return machinelist;
}

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

void listjobs(FILE *fpjob, job* joblist[], int qinfo[]) {
				char *buffer = NULL;
				size_t bufsize = 0;
				char s[1000];
				char jobname[1000];
				char *targetmachine;
				int queuenum;
			  int front[5] = {0};
				int rear[5] = {0};
				int bytesread = 0, bytesnow, charsread = 0;
				//job *joblist = (job*)malloc(100*sizeof(job));
				int i = 0, j = 0;
				while(getline(&buffer, &bufsize, fpjob)>0) {
					sscanf(buffer, "%s%n", jobname, &bytesnow);	
				  //strcpy(joblistqueuenum[i].name, s);
					bytesread = bytesnow;
					j = 0;	
					sscanf(buffer+bytesread, "%s%n", s, &bytesnow);
					targetmachine = strtok(s,":");
					queuenum = getqueuenum(targetmachine);
					printf("Queuenum for targetmachine %s: %d\n", targetmachine, queuenum);
					while((charsread = sscanf(buffer+bytesread, "%s%n", s, &bytesnow))>0){ 
								strcpy(joblist[queuenum][rear[queuenum]].actlist[j],s);
								strcpy(joblist[queuenum][rear[queuenum]].actremain[j],s);
								joblist[queuenum][rear[queuenum]].currenttasknum = 0;
								//printf("%s ", s);
								bytesread+=bytesnow;
								j++;
					}
				  strcpy(joblist[queuenum][rear[queuenum]].name, jobname);	
					printf("queuenum: %d jobname: %s I: %d\n", queuenum, jobname, rear[queuenum]);
					joblist[queuenum][rear[queuenum]].totaltasks = j;
					rear[queuenum] = rear[queuenum]+1;
					//if(i>3) break;
					//Error: I should not be incremented every time, fix this.
				}
				for(i=0;i<5;i++) {
					qinfo[i+5] = rear[i];
				}
				for(i=0;i<4;i++){
					//printq(joblist[i], 0, 3);
				}	
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
