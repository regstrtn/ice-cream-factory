#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int unittest(int nummachines, char machinenames[][100], int machineinstances[], int numinst, int timereq[], char tasks[][100], char semnames[][100]) {
	int i = 0, j = 0;
	printf("Machines: %d Instances: %d\n", nummachines, numinst);
	for(i=0;i<nummachines;i++) {
		printf("%d Machine: %s Semaphore: %s Instances: %d", i, machinenames[i], semnames[i], machineinstances[i]);
	}
	for(j=0;j<2;j++) {
	printf("Execution reaches unittest\n");
		printf(" Task: %s Timereq: %d",  tasks[0], timereq[0]); 
	}

}
