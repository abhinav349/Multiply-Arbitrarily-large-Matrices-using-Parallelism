#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/time.h>

pid_t pids[2];
int status;

long long int current_timestamp(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}


void* RRscheduling() {
	// int *p2 = (int*) z;
	// int p = *p2;
  long long int timeQuantum = 2000;
  int lastProcess = 0;
  long long int lastSwitched = current_timestamp();
  while(1) {
  	// printf("Hello\n");
    while(current_timestamp() - lastSwitched < timeQuantum);
    long long int tmp = current_timestamp();
    kill(pids[lastProcess], SIGSTOP);
    kill(pids[lastProcess], SIGCONT);
    lastProcess = !lastProcess;
    lastSwitched = tmp;
	if(lastProcess)
		printf("Process 2\n");
	else
		printf("Process 1\n");
    }
    // if(waitpid(p, &status, WNOHANG)!=0) return NULL;
}

int z;

void RR() {
    pid_t p = fork();
	
    if(p == 0) {
        pids[0] = getpid();                     // 1st child process
		printf("p1 = %d\n", pids[0]);		
		p = pids[0];
		execl("proc1", "proc1", NULL); 
    } 
	else {
        p = fork();
        if(p == 0) {
            pids[1] = getpid();
			printf("p2 = %d\n", pids[1]);
			p = pids[1];
			z = pids[1];
			execl("proc2", "proc2", NULL);                  // 2nd child process
        }
		else{
            p = getpid();
			pthread_t timer;
			pthread_create(&timer, NULL, RRscheduling, NULL);
			pthread_join(timer, NULL);            
        }
    }
}

int main() {
    RR();
    return 0;
}