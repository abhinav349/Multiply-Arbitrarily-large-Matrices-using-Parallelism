#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

FILE *file1, *file2, *transpose;


#define a 50
#define n 20
#define b 50
#define N 5
int *mat1seek, *mat2seek;

void sigstop(){
    printf("sigstop\n");
}
void sigcont(){
    printf("sigcont\n");
}

void initialize(){
    file1=fopen("in1.txt","r");
    file2=fopen("in2.txt","r");
    int matrix1[a][n], matrix2[n][b];
    mat1seek=(int*) malloc(a*n*sizeof(int));
    mat2seek=(int*) malloc(b*n*sizeof(int));
    for(int i=0;i<a;i++){
        for(int j=0;j<n;j++){
            mat1seek[i*n+j]=ftell(file1);
            fscanf(file1,"%d ",&matrix1[i][j]);
        }
    }
    for(int i=0;i<n;i++){
        for(int j=0;j<b;j++){
            fscanf(file2,"%d",&matrix2[i][j]);
        }
    }
    printf("\n");
    transpose=fopen("transpose.txt","a");
    transpose=fopen("transpose.txt","w+");
    
    int k=0;
    for(int j=0;j<b;j++){
        for(int i=0;i<n;i++){
            mat2seek[k++]=ftell(transpose);
            fprintf(transpose,"%d ",matrix2[i][j]);
        }
        fprintf(transpose,"\n");
    }
}

key_t key;
int shmBuf1id;
int (*buf1Ptr)[a+b];

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

pthread_t tid[N];
int array1[a][n],array2[b][n];

void* threadFunc(void* i){
    int* in = (int*) i;
    int index = *in;
    int k=0;
    int x=(a*n)/N, y=(b*n)/N;
    printf("Thread index = %d is running\n",index);
    if(index<N-1){
        k=0;
        while(k<x){
            // n==number of columns in the array
            fseek(file1, mat1seek[index*x+k], 0);
            fscanf(file1,"%d", &array1[(index*x+k)/n][(index*x+k)%n]);
            buf1Ptr[((index*x+k)/n)][((index*x+k)%n)] = array1[((index*x+k)/n)][((index*x+k)%n)];
            ++k;
        }
        int l=0;
        while(l<y){
            fseek(transpose, mat2seek[index*y+l], 0);
            fscanf(transpose,"%d", &array2[(index*y+l)/n][(index*y+l)%n]);
            buf1Ptr[a+(index*y+l)/n][(index*y+l)%n] = array2[(index*y+l)/n][(index*y+l)%n];
            ++l;
        }
    }
    else if(index==N-1){
        k=0;
        while(index*x+k<a*n){
            fseek(file1, mat1seek[index*x+k], 0);
            fscanf(file1,"%d", &array1[(index*x+k)/n][(index*x+k)%n]);
            buf1Ptr[(index*x+k)/n][(index*x+k)%n] = array1[(index*x+k)/n][(index*x+k)%n];
            ++k;
        }
        int l=0;
        while(index*y+l<b*n){
            fseek(transpose, mat2seek[index*y+l], 0);
            fscanf(transpose,"%d", &array2[(index*y+l)/n][(index*y+l)%n]);
        buf1Ptr[a+(index*y+l)/n][(index*y+l)%n] = array2[(index*y+l)/n][(index*y+l)%n];
            ++l;
        }
    }
    printf("Thread index = %d finished its task.\n",index);
    
    return NULL;
}

int main()
{
    initialize();
    createBuf();    
    for(int i=0;i<N;i++){
        pthread_create(&tid[i],NULL,threadFunc, &i);
        pthread_join(tid[i],NULL);
    }

    fclose(file1);
    fclose(file2);
    fclose(transpose);
    
    
    printf("\nReading from shared memory space1  :\n");
    printf("Array1 :\n");
    for(int i=0; i<a; i++){
        for(int j=0; j<n; j++){
            printf("%d ", buf1Ptr[i][j]);
        }
        printf("\n");
    }
    printf("Reading from shared memory space2  :\n");
    printf("Array2  :\n");
    for(int i=a; i<a+b; i++){
        for(int j=0; j<n; j++){
             printf("%d ", buf1Ptr[i][j]);
        }
        printf("\n");
    }
    
    return 0;
}