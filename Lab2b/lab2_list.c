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
int lists = 1;
int yield = 0;
char sync = NO_LOCK;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
int spin = 0;
int opt_yield = 0;

SortedList_t* list;
SortedListElement_t* elements;
int numElements;

long long* waitTimes;

struct Sublist{
  SortedList_t* list;
  pthread_mutex_t mtx;
  int spin;
};

struct Sublist* sublist_arr;

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
    char randchar = (rand() % 26) + 'a';
    rstring[i] = randchar;
  }
  rstring[i] = '\0';
}

long long lock(struct Sublist* subl){
  if(sync == MUTEX){
    struct timespec locks, locke;
    clock_gettime(CLOCK_MONOTONIC, &locks);
    pthread_mutex_lock(&subl->mtx);
    clock_gettime(CLOCK_MONOTONIC, &locke);
    return((locke.tv_sec * 1000000000) + locke.tv_nsec)-((locks.tv_sec * 1000000000) + locks.tv_nsec);
  }
  else if(sync == SPIN_LOCK){
    while(__sync_lock_test_and_set(&subl->spin, 1));
    return 0;
  }
  return 0;
}

void unlock(struct Sublist* subl){
  if(sync == MUTEX){
    pthread_mutex_unlock(&subl->mtx);
    return;
  }
  else if(sync == SPIN_LOCK){
    __sync_lock_release(&subl->spin);
    return;
  }
  return;
}

int hash(char* key){
  int sum =0;
  for(int i=0; key[i]!='\0'; i++)
    sum+= (int)key[i];
  return sum % lists;
}

void threadfunc(void* i){
  int index = *((int*) i);
  int endind = index+iterations;
  int waitind = index/iterations;
  for(int j=index; j<endind; j++){
    char* key = malloc(6*sizeof(char));
    strcpy(key, elements[j].key);
    struct Sublist* cur = &sublist_arr[hash(key)];
    waitTimes[waitind] += lock(cur);
    SortedList_insert(cur->list, &elements[j]);
    unlock(cur);
  }

  int totLen = 0;
  for(int j=0; j<lists; j++){
    waitTimes[waitind] += lock(&sublist_arr[j]);
    int len = SortedList_length(sublist_arr[j].list);
    totLen+=len;
    unlock(&sublist_arr[j]);

    if( len == -1){
      fprintf(stderr, "Error in calling SortedList_length! List is corrupted\n");
      fflush(stderr);
      exit(2);
    }
  }

  if(totLen < 0){
    fprintf(stderr,"Error: Total of sublist lengths is negative! Length:%d\n",totLen);
    fflush(stderr);
    exit(2);
  }

  for(int j=index; j<endind; j++){
    char* key = malloc(6*sizeof(char));
    strcpy(key, elements[j].key);
    struct Sublist* cur = &sublist_arr[hash(key)];
    waitTimes[waitind] += lock(cur);
    SortedListElement_t* toDelete = SortedList_lookup(cur->list, key);
    unlock(cur);
    
    if(toDelete == NULL){
      fprintf(stderr,"Error in SortedList_lookup! cannot find element with key '%s'\n", elements[j].key);
      fflush(stderr);
      exit(2);
    }

    waitTimes[waitind] += lock(cur);
    int stat = SortedList_delete(toDelete);
    unlock(cur);
    
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
				 {"lists", 1, NULL, 'l'},
				 {0,0,0,0}
  };

  threads = 1;
  iterations = 1;
  lists = 1;
  yield = 0;
  char* yieldstr = malloc(5*sizeof(char));

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
	  break;
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
    case 'l':
      lists = atoi(optarg);
      break;
    }
  } 

  //Initialize Sublists
  sublist_arr = malloc(lists* sizeof(struct Sublist));

  for(int i=0; i<lists; i++){
    sublist_arr[i].list = malloc(sizeof(SortedList_t));
    sublist_arr[i].list->next = sublist_arr[i].list;
    sublist_arr[i].list->prev = sublist_arr[i].list;
    sublist_arr[i].list->key = NULL;
    if(sync == MUTEX){
      if(pthread_mutex_init(&sublist_arr[i].mtx, NULL)){
	fprintf(stderr, "Error initializing mutex lock!\n");
	fflush(stderr);
	exit(2);
      }
    }
    else if(sync == SPIN_LOCK)
      sublist_arr[i].spin = 0;
  }

  //Initialize List Elements
  srand(time(NULL));
  numElements = threads * iterations;
  elements = malloc(numElements * sizeof(SortedListElement_t));
  for(int i=0; i<numElements; i++){
    char* rstring = malloc(6*sizeof(char));
    random_string(rstring, 5);
    elements[i].key = rstring; 
  }

  //High res time start
  struct timespec start_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  long long start = (start_time.tv_sec * 1000000000) + start_time.tv_nsec;

  pthread_t* thread_arr = malloc(threads* sizeof(pthread_t));
  int* indexes = malloc(threads * sizeof(int));
  for(int i=0; i<threads; i++){
    indexes[i] = i*iterations;
  }
  waitTimes = malloc(threads* sizeof(long long));

  
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
  for(int i =0; i<lists; i++){
    if(SortedList_length(sublist_arr[i].list) != 0){
      fprintf(stderr, "Linked List is not of length 0!\n");
      fflush(stderr);
      exit(1);
    }
  }
  
  //Print CSV record
  if(yield==0)
    strcpy(yieldstr, "none");
  char* syncstr = malloc(5* sizeof(char));
  if(sync == NO_LOCK)
    strcpy(syncstr, "none");
  else{
    syncstr[0] = sync;
    syncstr[1] = '\0';
  }

  long long avgWaitTime = 0;
  for(int i =0; i<threads; i++)
    avgWaitTime += waitTimes[i];

  avgWaitTime /= threads*(iterations*3 + 1);
  
  int ops = threads*iterations*3;
  long long run_time = end - start;
  long long avg = run_time/ops;
  printf("list-%s-%s,%d,%d,%d,%d,%lld,%lld,%lld\n",
	 yieldstr,syncstr,threads,iterations,lists,ops,run_time,avg,avgWaitTime);
  fflush(stdout);

  free(list);
  free(elements);
  free(thread_arr);
  free(indexes);
  exit(0);
}
