#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "hash.h"
#include "rvs_queue_table.h"

//#define TEST
#define LOCK_TIME 150000  //time in milliseconds

hash_table queue_table;

void init_rvs_queue_table(){
  printf("Creating RVS Queue table...\n");
  construct_hash_table(&queue_table, 10);
}

void destroy_rvs_queue_table(){
  free_hash_table(&queue_table, NULL);
  printf("Destroying RVS Queue table...\n");
}
  
void queue(const NID nid)
{
  rvsq_entry *re = (rvsq_entry *) calloc(1, sizeof(rvsq_entry));
  re->nid = nid;
  gettimeofday(&re->tstamp, NULL);
  
  hash_insert(nid, re, &queue_table);
}

void unqueue(const NID nid){
  hash_del(nid, &queue_table);
}

unsigned short int isQueued(const NID nid)
{
  static struct timeval now;
  gettimeofday(&now, NULL);
  rvsq_entry *re = (rvsq_entry *) hash_lookup(nid, &queue_table);    
  
  if (re != NULL){
    if (re->tstamp.tv_usec + LOCK_TIME < now.tv_usec){
      unqueue(nid);
      return 0;
    }
    else{
      return (!memcmp(&nid, &re->nid, 16));
    }
  }
  
  return 0;
}

#ifdef TEST

int main()
{
  NID nid1;
  
  inet_pton(AF_INET6, "dead::beef", &nid1);
  init_queue_table();
  
  printf("query 1 = %d\n", isQueued(nid1)); // 0
  queue(nid1);
  printf("query 2 = %d\n", isQueued(nid1)); // 1
  unqueue(nid1);
  printf("query 3 = %d\n", isQueued(nid1)); // 0
  queue(nid1);
  sleep(2);
  printf("query 4 = %d\n", isQueued(nid1)); // 0
  queue(nid1);
  printf("query 5 = %d\n", isQueued(nid1)); // 1
  queue(nid1);
  printf("query 6 = %d\n", isQueued(nid1)); // 1
  unqueue(nid1);
  printf("query 7 = %d\n", isQueued(nid1)); // 0
  
  return 0;
}

#endif
