#include<stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/resource.h>
//#include <sys/pthread.h>
#include<errno.h>
#include<signal.h>
#include <unistd.h>
#include<stdlib.h>
#include <sys/shm.h> 
#include <fcntl.h>       
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>
#include<string.h>
#define size 1000
mode_t perms =S_IRUSR | S_IWUSR | S_IRGRP |S_IWGRP | S_IROTH |S_IWOTH;
void parent();

struct sample_slave
   {
      char *machine_name;
      int instances;
      pid_t *pid;
      char **activity_names;
      int *time_required;
      
   } *sampleslave;
   
   struct job_pool
   {
      char * icecream_name;
      char **activity_names;
      char **machine_name;
      int remaining_activity_index;
      int remaining_activity_count;
      
   } *jobpool;
  
   int job_count=0; 
   int count_machine_type=0; 
   int MAX;
   int *queue[100];
   int *front;
   int *rear; 
   int *share_update;
   sem_t *semvar;
   sem_t *semvar1;
   int *share1,*share2;
   
int main(int agrc,char *argv[])
{
   
   key_t key1; /* key to be passed to shmget() */ 
   int shmflg; /* shmflg to be passed to shmget() */ 
   int size1; /* size to be passed to shmget() */ 
   FILE *sample_slave_file,*sample_master_file;
   char buffer[size];
   int total_child_process =0 ;
   int semid; 

   int shmid[100],shmid_front,shmid_rear; /* return value from shmget() */ 
   
   int nsems = 1;
   
   /*
     * We'll name our shared memory segment
     * "5678".
     */
    key1 = 5678;
   /*opening the input files*/
   sample_slave_file=fopen("Sample-slave.info","r");
   sample_master_file=fopen("Sample-job.info","r");
   if(sample_slave_file==NULL || sample_master_file==NULL)
   {
      printf("error opening file");
      perror("fopen");
      exit(1);
   }
   
   /**************************************************/
           /*processing sample_slave_file*/
   /**************************************************/
   
   /*counting the number of types of machines available*/
  
   while(fgets(buffer,sizeof(buffer),sample_slave_file)!=NULL)
   {
      count_machine_type++;
   }
   printf("m/c types= %d\n",count_machine_type);
   rewind(sample_slave_file);
   
   /*allocating memory to the structure*/
   sampleslave=(struct sample_slave*)calloc(count_machine_type,sizeof(struct sample_slave));
   
   /*reading data from file-- machine_name, instances*/ 
   int index=0,count_index=0,index_line=0;
   int *count=(int *)calloc(count_machine_type,sizeof(int));  
   while(fgets(buffer,sizeof(buffer),sample_slave_file)!=NULL)
   { 
      char *token;
      index=0;
      token=strtok(buffer," ");
      while(token!=NULL)
      {
         index++;
         if(index==1)
            sampleslave[index_line].machine_name=strdup(token);
         else if(index==2)
            {
            	
               sampleslave[index_line].instances=atoi(strdup(token));
               
               total_child_process = total_child_process + sampleslave[index_line].instances;
               sampleslave[index_line].pid=(pid_t *)calloc(sampleslave[index_line].instances,sizeof(int));
            }
         else
            count[count_index]++;
         token=strtok(NULL," ");
      }
      /*allocating memory to the structure members*/
      sampleslave[index_line].activity_names=(char **)calloc(count[count_index]/2,sizeof(char *));
      sampleslave[index_line].time_required=(int *)calloc(count[count_index]/2,sizeof(int));
      index_line++;
      
   }
   rewind(sample_slave_file);
   
   
   /*reading data from file-- activities,time_required*/ 
   index_line=0;
   while(fgets(buffer,sizeof(buffer),sample_slave_file)!=NULL)
   { 
      char *token;
      index=0;
      int act_index=0;
      token=strtok(buffer," ");
      while(token!=NULL)
      {
         index++;
         if(index>2 && index%2!=0)
            sampleslave[index_line].activity_names[act_index]=strdup(token);
         else  if(index>2 && index%2==0)
            {
               sampleslave[index_line].time_required[act_index]=atoi(strdup(token));
               act_index++;
            }
         token=strtok(NULL," ");
      }
      index_line++;
      
   }
   fclose(sample_slave_file);
   
   /**************************************************/
         /*processing sample_master_file*/
   /**************************************************/
   
  
   while(fgets(buffer,sizeof(buffer),sample_master_file)!=NULL)
   {
      job_count++;
   }
   MAX=job_count+1;
   rewind(sample_master_file);
   
   /*allocating memory to the structure job_pool*/
   jobpool=(struct job_pool*)calloc(job_count,sizeof(struct job_pool));
   
   /*reading data from file-- icecream_names*/ 
   index=0,count_index=0,index_line=0;
   int *counter=(int *)calloc(job_count,sizeof(int));  
   while(fgets(buffer,sizeof(buffer),sample_master_file)!=NULL)
   { 
      char *token;
      index=0;
      token=strtok(buffer," ");
      while(token!=NULL)
      {
         index++;
         if(index==1)
            jobpool[index_line].icecream_name=strdup(token);
         else
            counter[count_index]++;
         token=strtok(NULL," ");
      }
      
      /*allocating memory to the structure members*/
      jobpool[index_line].activity_names=(char **)calloc(counter[count_index],sizeof(char *));
      jobpool[index_line].machine_name=(char **)calloc(counter[count_index],sizeof(char *));
      index_line++;
      
   }
   rewind(sample_master_file);
   
   /*reading data from file-- activities_names,machine_names*/ 
    index_line=0;
   while(fgets(buffer,sizeof(buffer),sample_master_file)!=NULL)
   { 
        printf("%s\n",buffer);
      char *token;
      char *string1;
      index=0;
      token=strtok_r(buffer," ",&string1);
      
      while(token!=NULL)
      {
        //printf("token= %s\n",token);
         index++;
         int act_index=0;
         char *string2;
         if(index>1)
         {
            char *token2;
            
            int flag=0;
            token2=strtok_r(token,":",&string2);
             while(token2!=NULL)
             {
             
             
               if(flag==0)
                {
                  jobpool[index_line].machine_name[act_index]=strdup(token2);
                  flag=1;
                  printf("flag= %d name = %s\n",flag, jobpool[index_line].machine_name[act_index]);
                }
               else if(flag==1)
               {   
               
                jobpool[index_line].activity_names[act_index]=strdup(token2);
                  flag=0;
                   printf("flag= %d name = %s\n",flag,jobpool[index_line].activity_names[act_index]);
                  act_index++;
               }
               token2=strtok_r(NULL,":",&string2);
             }
             jobpool[index_line].remaining_activity_index=0;
             jobpool[index_line].remaining_activity_count=counter[index_line]/2;
         }
         
         token=strtok_r(NULL," ",&string1);
         printf("token =%s\n",token);
         
      }
      index_line++;
      
   }
   fclose(sample_master_file);
   printf("activity name = %s\n",jobpool[1].activity_names[1]);
   
   /*creating queue,front,end*/
   int queue_index=0,ii;char f_num[5];
   for(queue_index=0;queue_index<count_machine_type;queue_index++)
   {
        char * s = (char*)calloc(5,sizeof(char));
        sprintf(f_num,"%d",key1);
        strncpy(s,f_num,4);
       
        
   	 shmid[queue_index] = shm_open(s, O_CREAT | O_RDWR,perms);
   	 if(shmid[queue_index]==-1)
   	 printf("error form shmid[%d]\n",queue_index);
   	 if(ftruncate(shmid[queue_index],1) == -1)
   	        exit(1);
   	 queue[queue_index] = (int *)mmap(NULL,job_count * sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid[queue_index],0);
   	 key1+=1;
   }
   char * s = (char*)calloc(5,sizeof(char));
   sprintf(f_num,"%d",key1);
        strncpy(s,f_num,4);
           key1++;
   
   shmid_front = shm_open(s,O_CREAT | O_RDWR,perms);
   if(shmid_front==-1)
   	 printf("error form shmid_front\n");
   if(ftruncate(shmid_front,1) == -1)
   	        exit(1);
   front = (int *)mmap(NULL,count_machine_type * sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid_front,0);
   sprintf(f_num,"%d",key1);
        strncpy(s,f_num,4);
   
   shmid_rear = shm_open(s,O_CREAT | O_RDWR,perms);
   if(shmid_rear==-1){
   	 printf("error form shmid_rear\n");
   	 exit(1);
   	 }
   	 if(ftruncate(shmid_rear,1) == -1)
   	        exit(1);
   rear = (int *)mmap(NULL,count_machine_type * sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid_rear,0);
   close(shmid_rear);
   
    int shmid1 = shm_open("share_update",O_CREAT | O_RDWR,perms);
   if(shmid1==-1)
   	 printf("error form shmid1\n");
   if(ftruncate(shmid1,1) == -1)
   	        exit(1);
   share_update = (int *)mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid1,0);
  *share_update=-1;
  
   shmid1 = shm_open("share1",O_CREAT | O_RDWR,perms);
   if(shmid1==-1)
   	 printf("error form shmid1\n");
   if(ftruncate(shmid1,1) == -1)
   	        exit(1);
   share1 = (int *)mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid1,0);
  *share1=-1;
  
   shmid1 = shm_open("share2",O_CREAT | O_RDWR,perms);
   if(shmid1==-1)
   	 printf("error form shmid1\n");
   if(ftruncate(shmid1,1) == -1)
   	        exit(1);
   share2 = (int *)mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid1,0);
  *share2=-1;
  
   for(ii=0;ii<count_machine_type;ii++)
    {
        rear[ii]=-1;
        front[ii]=-1;
    }
    
    /*creating and initializing semaphore*/
    
    sem_unlink("/semvar");
    semvar=sem_open("/semvar", O_CREAT|O_EXCL,0644,1);
    perror("sem_open: ");
    
     sem_unlink("/semvar1");
    semvar=sem_open("/semvar1", O_CREAT|O_EXCL,0644,1);
    perror("sem_open: ");


   /*enqueue - the first job of all the activities*/
   int job_index=0,machine_index=0;
   for(job_index=0;job_index<job_count;job_index++)
   {
        for(machine_index=0; machine_index < count_machine_type; machine_index++)
        {
                if(strcmp(sampleslave[machine_index].machine_name,jobpool[job_index].machine_name[jobpool                             [job_index].remaining_activity_index])==0)
                {
                        sem_wait(semvar);
                        if(enqueue(machine_index,job_index,front,rear,queue)) 
                        sem_post(semvar);
                         break; 
                }
        }
   }
 
 signal(SIGUSR1,parent);
 
        
   /*forking child proccesses*/
   int creating_process_index=0, sample_slave_index=0,check=0;
   pid_t p_id;
   for(creating_process_index=0; creating_process_index<total_child_process; creating_process_index++)
   {
   			char * my_name = sampleslave[sample_slave_index].machine_name;
   			int my_index = sample_slave_index;
   			sampleslave[sample_slave_index].pid[check] = p_id;
   			check++;
   			
   			if(check == sampleslave[sample_slave_index].instances)
   			 {
   			 		sample_slave_index++;
   					check=0;
   			 }
   			if ((p_id = fork()) < 0) 
                        {
                                perror("fork");
                                return 1;
                        }
   			if(p_id == 0)
   			{
   			        int element=-1;
   			 	int *front;
                                int *queue[100];
                                int shmid;
                                int *share_update;
                                int *rear,*share1,*share2;
                                char f_num[5];
                                key1=5678;
                                int queue_index=0,ii;
                                for(queue_index=0;queue_index<count_machine_type;queue_index++)
                                 {    
                                   char * s = (char*)calloc(5,sizeof(char));
                                   sprintf(f_num,"%d",key1);
                                   strncpy(s,f_num,4);
                                   shmid = shm_open(s, O_RDWR,perms);
   	                           if(shmid==-1)
   	                             printf("error form shmid[%d]\n",queue_index);
   	                           queue[queue_index] = (int *)mmap(NULL,job_count * sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid,0);
   	                           key1+=1;
                                 }
                                 char * s = (char*)calloc(5,sizeof(char));
                                 sprintf(f_num,"%d",key1);
                                 strncpy(s,f_num,4);
                                 key1++;
                                 shmid = shm_open(s, O_RDWR,perms);
                                 if(shmid==-1)
   	                           printf("error form shmid_front\n");
                                 front = (int *)mmap(NULL,count_machine_type * sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid,0);
                                 sprintf(f_num,"%d",key1);
                                 strncpy(s,f_num,4);
                                 shmid = shm_open(s, O_RDWR,perms);
                                 if(shmid==-1)
   	                          printf("error form shmid_rear\n");
                                 rear = (int *)mmap(NULL,count_machine_type * sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid,0);
   			 	                  
   			 	 shmid = shm_open("share_update", O_RDWR,perms);
                                 if(shmid==-1)
   	                           printf("error form shmid_front\n");
                                 share_update=(int *)mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid,0);                 
   			 	 
   			 	 shmid = shm_open("share1", O_RDWR,perms);
                                 if(shmid==-1)
   	                           printf("error form shmid_front\n");
                                 share1=(int *)mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid,0);                 
   			 	 
   			 	 shmid = shm_open("share2", O_RDWR,perms);
                                 if(shmid==-1)
   	                           printf("error form shmid_front\n");
                                 share2=(int *)mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,shmid,0);                 
   			 	 
   			 	 sem_t *semvar;
   			 	 sem_t *semvar1;
   			 	                              			
   				while(1)  /*child never exits and goes on in a loop which wont allow it to create any further processes*/
   			 	 {
   			 	  
   			 	        
   			 	        semvar=sem_open("/semvar",0); 
   			 	        semvar1=sem_open("/semvar1",0); 
                                        
                                        sem_wait(semvar);
                                        if(front[my_index]!=-1)
                                        element=dequeue(my_index,front,rear,queue);
   					sem_post(semvar);
   					
   					int cmp_index;
   					printf("activity[%d].. %s index=%d\n",element,jobpool[element].activity_names[1],jobpool[element].remaining_activity_index);
   					for(cmp_index=0;cmp_index<count[my_index];cmp_index++)
   					{
   					       if(strcmp(sampleslave[my_index].activity_names[cmp_index],jobpool[element].activity_names[jobpool[element].remaining_activity_index])==0)
   					       {
   					       printf("sleeping for %d\n",sampleslave[my_index].time_required[cmp_index]);
   					       //sleep(sampleslave[my_index].time_required[cmp_index]);
   					       break;
   					       }
   					}
   					jobpool[element].remaining_activity_count--;
   					jobpool[element].remaining_activity_index++;
   					printf("... %d\n",jobpool[element].remaining_activity_index);
   					
   					sem_wait(semvar1);
   					*share1=jobpool[element].remaining_activity_index;
   					*share2=jobpool[element].remaining_activity_count;
   					*share_update=element;
   					kill(getppid(),SIGUSR1);
   					
   			 	 }
   			 
   			}
   			
   			
   			
   	}
   	
   	
   	int status=0;
   	pid_t wpid;
   	while ((wpid = wait(&status)) > 0);  //waits for all the child proccessse to complete
        
        /*detaching and removing shared memory*/
        sem_unlink("/semvar");
        sem_unlink("/semvar1"); 
        sem_unlink("/share_update");
        sem_unlink("share1");
        sem_unlink("share2");
         key1=5678;
        for(queue_index=0;queue_index<count_machine_type;queue_index++)
        {
        
                sprintf(f_num,"%d",key1);
                strncpy(s,f_num,4);
                shm_unlink(s);
                key1+=1;
        }
        sprintf(f_num,"%d",key1);
        strncpy(s,f_num,4);
        shm_unlink(s);
        key1+=1;
        sprintf(f_num,"%d",key1);
        strncpy(s,f_num,4);
        shm_unlink(s);
   	
   
   
   
return 1;
}

int enqueue(int index,int additem,int *front,int *rear,int **queue)  
{
    int i;
    if (rear[index] == MAX - 1)
		printf("Queue Overflow \n");
	else
	{
	    if (front[index] == - 1)
                front[index] = 0;                /*If queue is initially empty */
          rear[index] = rear[index] + 1;
          i=rear[index];
          queue[index][i] = additem;
          printf("enqueued %d in queue[%d]   front = %d rear= %d\n",queue[index][i],index,front[index],rear[index]);

       }
    return 1;

} /*End of insert()*/

int dequeue(int index,int *front,int *rear,int **queue)
{
    int element;
    //printf("from process %d front =%d rear = %d\n",index,front[index],rear[index]);
    //printf("dqueueing from queue[%d]\n",index);
    if(rear[index]<front[index] || front[index]==-1)
    {
        printf("Underflow!!\n\n\n");
        
    }
    else
    {
        if(front[index]==rear[index])
         {       
        int temp=front[index];
        element=queue[index][temp];
        front[index]=-1;
        rear[index]=-1;
        }
        else
        {
        int temp=front[index];
        element=queue[index][temp];
        front[index]++;
        }
        
        printf("\nElement Dequeued is %d front = %d rear =%d\n\n",element,front[index],rear[index]);
    }
    return element;
}
/*void display()
{
    int i;
    if(front==rear)
    {
        printf("\nQueue is Empty!!!");
    }
    else
    {
        printf(" \n");
        for(i=front;i<max;i++)
        {
            printf(" | %d ",queue[i]);
        }
            printf("|");
    }
}*/

void parent()
{
        printf("in parent\n");
        if(*share_update!=-1)
        {
                
               // int queue_index1=jobpool[*share_update].remaining_activity_index;
                printf("share_update= %d  count=%d  index=%d\n",*share_update,*share2,*share1);
                printf("%s\n",jobpool[*share_update].machine_name[1]);
                
                        int iii=0;
                        for(iii=0;iii<count_machine_type;iii++)
                        {
                                
                                if(strcmp(sampleslave[iii].machine_name,jobpool[*share_update].machine_name[*share1])==0)
                                {
                                        int next_queue = iii;
                                        sem_wait(semvar); 
                                        enqueue(iii,*share_update,front,rear,queue);
                                        sem_post(semvar); 
                                        *share_update=-1;
                                        break;
                                }
                        }
                
        }
        sem_post(semvar1); 
        signal(SIGUSR1,parent);  
}
