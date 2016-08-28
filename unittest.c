#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int unittest(int nummachines, char machinenames[][100], int machineinstances[], char tasks[][100], int timereq[], int numinstances, int numtasks, char semnames[][100]) {
	int i = 0, j = 0;
	//printf("Machines: %d Instances: %d Tasks: %d\n", nummachines,numinstances, numtasks);
	for(i=0;i<nummachines;i++) {
		//printf("%d Machine: %s Semaphore: %s Instances: %d\n", i, machinenames[i], semnames[i], machineinstances[i]);
	}
	for(j=0;j<numtasks;j++) {
		//printf("Task: %s Timereq: %d\n",  tasks[j], timereq[j]); 
	}

}
