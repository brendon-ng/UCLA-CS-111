// NAME: Brendon Ng
// EMAIL: brendonn8@gmail.com
// ID: 304925492

#include "SortedList.h"
#include <pthread.h>
#include <time.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define NO_LOCK 'n'
#define MUTEX 'm'
#define SPIN_LOCK 's'

int threads = 1;
int iterations = 1;
int yield = 0;
char sync = NO_LOCK;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
int spin = 0;
int opt_yield = 0;

SortedList_t* list;
SortedListElement_t* elements;

void handler(int sig){
  if(sig == SIGSEGV){
    fprintf(stderr, "Error: Segmentation Fault! SIGSEGV signal caught\n");
    fflush(stderr);
    exit(2);
  }
}

void random_string(char* rstring, int size){
  int i;
  for(i=0; i<size; i++){
    char randchar = (rand() & 26) + 'a';
    rstring[i] = randchar;
  }
  rstring[i] = '\0';
}

void lock(){
  if(sync == MUTEX){
    pthread_mutex_lock(&mtx);
    return;
  }
  else if(sync == SPIN_LOCK){
    while(__sync_lock_test_and_set(&spin, 1));
    return;
  }
  return;
}

void unlock(){
  if(sync == MUTEX){
    pthread_mutex_unlock(&mtx);
    return;
  }
  else if(sync == SPIN_LOCK){
    __sync_lock_release(&spin);
    return;
  }
  return;
}
void threadfunc(void* i){
  int index = *((int*)i);
  
  for(int j=index; j<iterations; j++){
    lock();
    SortedList_insert(list, &elements[index+j]);
    unlock();
  }

  lock();
  if(SortedList_length(list) == -1){
    fprintf(stderr, "Error in calling SortedList_length! List is corrupted\n");
    fflush(stderr);
    exit(2);
  }
  unlock();

  for(int j=index; j<iterations; j++){
    lock();
    SortedListElement_t* toDelete = SortedList_lookup(list, elements[j].key);
    unlock();
    
    if(toDelete == NULL){
      fprintf(stderr,"Error in SortedList_lookup! cannot find element with key '%s'\n", elements[j].key);
      fflush(stderr);
      exit(2);
    }

    lock();
    int stat = SortedList_delete(toDelete);
    unlock();
    
    if(stat){
      fprintf(stderr, "Error deleting element! List is corrupt\n");
      fflush(stderr);
      exit(2);
    }
  }
  return;
}

int main(int argc, char* argv[]){
  signal(SIGSEGV, handler);
  
  static struct option args[] = {
				 {"threads", 1, NULL, 't'},
				 {"iterations", 1, NULL, 'i'},
				 {"yield", 1, NULL, 'y'},
				 {"sync", 1, NULL, 's'},
				 {0,0,0,0}
  };

  threads = 1;
  iterations = 1;
  yield = 0;
  char yieldstr[5];

  int opt;
  while((opt=getopt_long(argc, argv, "", args, NULL)) != -1) {
    if(opt == '?'){
      fprintf(stderr, "Error: Unrecognized argument\n");
      exit(1);
    }

    switch(opt){
    case 't':
      threads = atoi(optarg);
      break;
    case 'i':
      iterations =  atoi(optarg);
      break;
    case 'y':
      for(int i=0; optarg[i]!='\0'; i++){
	switch(optarg[i]){
	case 'i':
	  yield |= INSERT_YIELD;
	  break;
	case 'd':
	  yield |= DELETE_YIELD;
	  break;
	case 'l':
	  yield |= LOOKUP_YIELD;
	  break;
	default:
	  fprintf(stderr, "Error: invalid yield argument %c- usage: --yield=[idl]\n", optarg[i]);
	  fflush(stderr);
	  exit(1);
	}
      }
      strcpy(yieldstr, optarg);
      break;
    case 's':
      if(optarg[0] != 'm' && optarg[0] != 's'){
	fprintf(stderr, "Error: invalid sync argument - usage: --sync=[ms]\n");
        fflush(stderr);
        exit(1);
      }
      sync = optarg[0];
      break;
    }
  } 


  //Initialize Empty List
  list = malloc(sizeof(SortedList_t));
  list ->next = list;
  list->prev = list;
  list->key = NULL;

  //Initialize List Elements
  srand(time(NULL));
  int numElements = threads * iterations;
  elements = malloc(numElements * sizeof(SortedListElement_t));
  for(int i=0; i<numElements; i++){
    char rstring[6];
    random_string(rstring, 5);
    elements[i].key = rstring; 
  }

  //High res time start
  struct timespec start_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  long long start = (start_time.tv_sec * 1000000000) + start_time.tv_nsec;

  pthread_t thread_arr[threads];
  int indexes[threads];
  for(int i=0; i<threads; i++){
    indexes[i] = i*iterations;
  }

  for(int i=0; i<threads; i++){
    if(pthread_create(&thread_arr[i], NULL, (void*) &threadfunc, (void*) &indexes[i])){
      fprintf(stderr, "Error creating a thread with pthread_create()\n");
      fflush(stderr);
      exit(1);
    }
  }

  //wait
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

  //check length of list
  if(SortedList_length(list) != 0){
    fprintf(stderr, "Linked List is not of length 0!\n");
    fflush(stderr);
    exit(1);
  }
  
  //Print CSV record
  if(yield==0)
    strcpy(yieldstr, "none");
  char syncstr[5];
  if(sync == NO_LOCK)
    strcpy(syncstr, "none");
  else{
    syncstr[0] = sync;
    syncstr[1] = '\0';
  }

  int ops = threads*iterations*3;
  long long run_time = end - start;
  long long avg = run_time/ops;
  printf("list-%s-%s,%d,%d,%d,%d,%lld,%lld\n",
	 yieldstr,syncstr,threads,iterations,1,ops,run_time,avg);
  fflush(stdout);

  free(list);
  free(elements);
  exit(0);
}
