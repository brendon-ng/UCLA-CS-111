#include "SortedList.h"
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <stdio.h>
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
  //  fprintf(stdout, "Insert");
  SortedListElement_t* cur = list;
  while(cur->next != list ){
    if(strcmp(cur->next->key, element->key) > 0){
      if(opt_yield & INSERT_YIELD)
	sched_yield();
      SortedListElement_t* temp = cur->next;
      cur->next = element;
      element->prev = cur;
      element->next = temp;
      temp->prev = element;
      return;
    }
    cur = cur->next;
  }

  if(opt_yield & INSERT_YIELD)
    sched_yield();
  
  cur->next = element;
  element-> prev = cur;
  element->next = list;
  list->prev = element;
}

int SortedList_delete(SortedListElement_t *element){
  // fprintf(stdout, "Delete");
  if(element == NULL)
    return 1;
  SortedListElement_t* next = element->next;
  SortedListElement_t* prev = element->prev;
  if(prev->next == element && next->prev == element){
    if(opt_yield & DELETE_YIELD)
      sched_yield();
    prev->next = element->next;
    next->prev = element->prev;

    return 0;
  }
  return 1;
}


SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
  // fprintf(stdout, "Lookup");
  SortedListElement_t* cur = list->next;

  if(opt_yield & LOOKUP_YIELD)
    sched_yield();
  
  while(cur->key != NULL){
    if(strcmp(cur->key, key)==0)
      return cur;
    cur = cur->next;
  }
  return NULL;
}

int SortedList_length(SortedList_t *list){
  // fprintf(stdout, "Length");
  if(list == NULL)
    return -1;
  
  SortedListElement_t* cur = list->next;
  int count =0;

  if(opt_yield & LOOKUP_YIELD)
    sched_yield();
  
  while(cur != list){
    count += 1;
    if(cur->prev->next != cur || cur->next->prev != cur)
      return -1;
    cur = cur->next;
  }
  return count;
}
