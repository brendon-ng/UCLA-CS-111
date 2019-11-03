//NAME: Brendon Ng
//EMAIL: brendonn8@gmail.com
//ID: 304925492

#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NO_LOCK 'n'
#define MUTEX 'm'
#define SPIN_LOCK 's'
#define COMP_AND_SWAP 'c'

long long counter = 0;
int threads = 1;
int iterations = 1;
int opt_yield = 0;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
int spin =0;
char sync = NO_LOCK;

void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if(opt_yield)
    sched_yield();
  *pointer = sum;
}

void call_add(long long val){
  for(int i=0; i<iterations;i++){
    switch(sync){
    case MUTEX:
      pthread_mutex_lock(&mtx);
      add(&counter, val);
      pthread_mutex_unlock(&mtx);
      break;
    case SPIN_LOCK:
      while(__sync_lock_test_and_set(&spin,1));
      add(&counter, val);
      __sync_lock_release(&spin);
      break;
    case COMP_AND_SWAP:
      ;
      long long old, newval;
      do{
        old = counter;
	newval = old + val;
	if(opt_yield)
	  sched_yield();
      }while(__sync_val_compare_and_swap(&counter, old, newval) != old);
      break;
    case NO_LOCK:
      add(&counter, val);
      break;
    }
  }
}

void threadfunc(){
  call_add(1);
  call_add(-1);
}


int main(int argc, char* argv[]){

  static struct option args[] = {
				 {"threads", 1, NULL, 't'},
				 {"iterations", 1, NULL, 'i'},
				 {"yield", 0, NULL, 'y'},
				 {"sync", 1, NULL, 's'},
				 {0,             0,   0,   0}
  };

  counter = 0;
  threads = 1;
  iterations = 1;

  int opt;
  while((opt = getopt_long(argc, argv, "", args, NULL)) != -1){
    if(opt == '?'){
      fprintf(stderr, "Error: Unrecognized argument\n");
      exit(1);
    }

    switch(opt){
    case 't':
      threads = atoi(optarg);
      break;
    case 'i':
      iterations = atoi(optarg);
      break;
    case 'y':
      opt_yield = 1;
      break;
    case 's':
      if(optarg[0] != 'm' && optarg[0] != 's' && optarg[0] != 'c'){
          fprintf(stderr, "Error: invalid sync argument - usage: --sync=[msc]\n");
          fflush(stderr);
          exit(1);
        }
        sync = optarg[0];
        break;
    }
  } //end while (optparse)

  struct timespec start_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  long long start = (start_time.tv_sec * 1000000000) + start_time.tv_nsec;
  
  
  pthread_t thread_arr[threads];
  
  for(int i=0; i<threads; i++){
    if(pthread_create(&thread_arr[i], NULL, (void *) &threadfunc, NULL)){
      fprintf(stderr, "Error creating thread with pthread_create\n");
      fflush(stderr);
      exit(1);
    }
  }

  for(int i=0; i<threads; i++){
    if(pthread_join(thread_arr[i], NULL)){
      fprintf(stderr, "Error joining thread #%d with pthread_join()\n", i);
      fflush(stderr);
      exit(1);
    }
  }

  //End time
  struct timespec end_time;
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  long long end = (end_time.tv_sec * 1000000000) + end_time.tv_nsec;

  //Make CSV Record
  char testname[5];
  if(sync == NO_LOCK)
    strcpy(testname, "none");
  else{
    char syncstring[2] = {sync,'\0'};
    strcpy(testname,syncstring);
  }

  char yieldstr[7] = "yield-";
  if(!opt_yield)
    yieldstr[0] = '\0';
  
  int operations = threads*iterations*2;
  long long run_time = end-start;
  printf("add-%s%s,%d,%d,%d,%lld,%lld,%lld\n",
	 yieldstr,testname,threads,iterations,operations,run_time,run_time/operations,counter);
  fflush(stdout);

  exit(0);
}
