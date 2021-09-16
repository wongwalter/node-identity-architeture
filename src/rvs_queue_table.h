#include "nid.h"
#include "hash.h"

typedef struct _rvs_queue_struct{
  NID nid;
  struct timeval tstamp;
}rvsq_entry;

void init_rvs_queue_table();
void destroy_rvs_queue_table();
void queue(const NID nid);
void unqueue(const NID nid);
unsigned short int isQueued(const NID nid);
