#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define a 50
#define n 20
#define b 50
#define N 5


int shmBuf1id;
int (*buf1Ptr)[a+b];
FILE *output;



void createBuf()
{
  shmBuf1id = shmget((key_t)1234, sizeof(long long int)*40000000, 0666|IPC_CREAT);

  if(shmBuf1id == -1 )
  {  
    perror("shmget");
    exit(1);
  }
  else
  {  
    printf("Shared memory space created \n");
    buf1Ptr = shmat(shmBuf1id,0,0);
    if(buf1Ptr == (void*) -1 )
    {  
      perror("shmat");
      exit(1);
    }
  }  
}

int step = 0;
long long int ans[a][b];

void* mul(void* arg){
    int num = (int)arg;
    int i, j, k;
    for(i=0; i<a; i++){
        for(j=0; j<b;j++){
            ans[i][j]=0;
            for(k=0; k<n; k++){     
                ans[i][j]+=buf1Ptr[i][k]*buf1Ptr[a+j][k];
            }
        }
    }
}



int main()
{
    createBuf();
    pthread_t tid[N];
    usleep(200000);    
    for(int i=0; i<N; i++){
        int *p;
        pthread_create(&tid[i], NULL, mul, (void *)i);
    }
    for(int i=0; i<N; i++){
        pthread_join(tid[i], NULL);
    }
    
    printf("Multiplication Result: \n");
    for(int i=0; i<a; i++){
        for(int j=0; j<b; j++){
            printf("%lld ", ans[i][j]);
        }
        printf("\n");
    }
    output = fopen("out.txt","a");
    output = fopen("out.txt","w");
    for(int i=0; i<a; i++){
        for(int j=0; j<b; j++){
            fprintf(output, "%lld ", ans[i][j]);
        }
        fprintf(output, "\n");
    }
    shmctl(shmBuf1id, IPC_RMID, NULL);
    return 0;
}