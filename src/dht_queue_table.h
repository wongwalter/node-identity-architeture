#include "nid.h"
#include "hash.h"

typedef struct _dht_queue_struct{
  NID nid;
  struct timeval tstamp;
}dhtq_entry;

void init_dht_queue_table();
void destroy_dht_queue_table();
void dht_queue(const NID nid);
void dht_unqueue(const NID nid);
unsigned short int is_dht_queued(const NID nid);
