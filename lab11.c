#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>

float globalSum=0.0;
HANDLE mutex;

//worker thread
DWORD WINAPI threadCode(LPVOID argument)
{
  float threadSum=0;
  float *array=(float *)argument;
  int arraySize=array[0];
  DWORD WINAPI currentThreadID = GetCurrentThreadId();

  printf("Thread #%ld arraySize=%d\n",currentThreadID,arraySize);
  for(int i=1;i<arraySize+1;i++)
    threadSum+=array[i];
  printf("Thread #%ld threadSum=%f\n",currentThreadID,threadSum);

  //critical section - updating global variable
  WaitForSingleObject(mutex,INFINITE);
  globalSum+=threadSum;
  ReleaseMutex(mutex);

  return 0;
}

int main(int argc, char *argv[]){
  if(argc>3){
    fprintf(stderr, "Too many arguments!\n");
    return 1;
  }
  if(argc==1)
  {
    fprintf(stderr,"Too few arguments!\n");
    return 1;
  }

  int noOfData = atoi(argv[1]); //n
  int threads = atoi(argv[2]); //w

  if(threads>=noOfData)
  {
    fprintf(stderr, "Number of threads have to be less than number of data!\n");
    return 1;
  }

  //allocating memory for array for random numbers
  float *data = malloc(sizeof(float)*noOfData);

  if(!data)
  {
    fprintf(stderr,"Memory allocation failed.");
    return 1;
  }

  srand(0);
  for(int i=0;i<noOfData;i++)
    data[i]=1000.*rand()/RAND_MAX;


  int divide = noOfData/threads;
  int remainder = noOfData%threads;
  //prepearing seperate arrays which will be send to threadCode
  float **dividedArray=malloc(sizeof(float*)*threads);

  for(int i=0;i<threads-1;i++)
    dividedArray[i]=malloc(sizeof(float)*divide+1);
  dividedArray[threads-1]=malloc(sizeof(float)*(divide+remainder+1));

  //distributing data to other arrays
  for(int i=0,k=0;i<noOfData-divide-remainder;k++)
  {
    dividedArray[k][0]=divide;
    for(int j=1;j<divide+1;j++,i++)
      dividedArray[k][j]=data[i];
  }
  dividedArray[threads-1][0]=divide+remainder;
  for(int i=noOfData-divide-remainder,j=1;i<noOfData;j++,i++)
    dividedArray[threads-1][j]=data[i];


//DWORD *threadIDs=malloc(sizeof(DWORD)*threads);
HANDLE *threadHandles=malloc(sizeof(HANDLE)*threads);
HANDLE mutex = CreateMutex(NULL,FALSE,NULL);
                        //security attributes for mutex; if NULL - default values
                        //!=0 - create and make busy; =0 - only create
                        //mutex's name; if NULL - anonymous mutex
clock_t start=clock();
//creating threads
for(int i=0;i<threads;i++)
  threadHandles[i]=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) threadCode,dividedArray[i],0,NULL);//threadIDs+i);
                    //security attributes for the new thread; if NULL - default values
                    //stack size for the new thread; if =0 - default size
                    //pointer to thread function; it's important to be LPTHREAD_START_ROUTINE type
                    //pointer to thread function's arguments
                    //bit mask concerning creation options for the new thread; if =0 - default values
                    //pointer to variable which will store new thread's ID

//waiting for all threads to finish
for(int i=0;i<threads;i++)
{
  WaitForSingleObject(threadHandles[i],INFINITE);
  CloseHandle(threadHandles[i]);
}
clock_t stop=clock();
CloseHandle(mutex);

printf("w /Threads: globalSum=%f, time=%fs\n",globalSum,(stop-start)/(float)CLOCKS_PER_SEC);

//calculating sum without threads
globalSum=0.0;
start=clock();
for(int i=0;i<noOfData;i++)
  globalSum+=data[i];
stop=clock();

printf("wo/Threads: globalSum=%f, time=%fs\n",globalSum,(stop-start)/(float)CLOCKS_PER_SEC);

//freeing allocated memory
  for(int i=0;i<threads;i++)
    free(dividedArray[i]);
  free(dividedArray);
  //free(threadIDs);
  free(threadHandles);
  return 0;
}