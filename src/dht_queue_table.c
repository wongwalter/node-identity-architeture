#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "hash.h"
#include "dht_queue_table.h"

#define LOCK_TIME 150000  //time in milliseconds

hash_table dht_queue_table;

void init_dht_queue_table(){
  printf("Creating DHT Queue table...\n");
  construct_hash_table(&dht_queue_table, 10);
}

void destroy_dht_queue_table(){
  free_hash_table(&dht_queue_table, NULL);
  printf("Destroying DHT Queue table...\n");
}
  
void dht_queue(const NID nid)
{
  dhtq_entry *de = (dhtq_entry *) calloc(1, sizeof(dhtq_entry));
  de->nid = nid;
  gettimeofday(&de->tstamp, NULL);
  
  hash_insert(nid, de, &dht_queue_table);
}

void dht_unqueue(const NID nid){
  hash_del(nid, &dht_queue_table);
}

unsigned short int is_dht_queued(const NID nid)
{
  static struct timeval now;
  gettimeofday(&now, NULL);
  dhtq_entry *de = (dhtq_entry *) hash_lookup(nid, &dht_queue_table);    
  
  if (de != NULL){
    if (de->tstamp.tv_usec + LOCK_TIME < now.tv_usec){
      dht_unqueue(nid);
      return 0;
    }
    else{
      return (!memcmp(&nid, &de->nid, 16));
    }
  }
  
  return 0;
}

