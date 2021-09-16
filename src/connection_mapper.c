#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "configuration.h"
#include "connection_mapper.h"
#include "rvs_queue_table.h"

extern int sp_cm2rvs[2], sp_rvs2ph[2], sp_mo2ph[2];

static hash_table connmap;
static NID cm_nidgw;
static NID cm_nidrvs;
static NID32 cm_gatewayIP;
static NID32 cm_rvsIP;

void newCMEntry(const NID nid, const NID32 ip)
{
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &connmap);
  
  if (ce){
    if (ce->tstamp == 0)
      return;    
    else{
      ce->ip = ip;
      ce->tstamp = time(NULL);
      hash_insert(nid, ce, &connmap);
      return;
    }
  }
  
  ce = (conn_entry *) calloc(1, sizeof(conn_entry));  
  
  ce->ip = ip;
  ce->tstamp = time(NULL);

  hash_insert(nid, ce, &connmap);
}


void staticCMEntry(const NID nid, const NID32 ip)
{
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &connmap);
  
  if (ce){        
    ce->ip = ip;
    hash_insert(nid, ce, &connmap);
  }
  else {
    ce = (conn_entry *) calloc(1, sizeof(conn_entry));
    
    ce->ip = ip;
    ce->tstamp = 0;
    hash_insert(nid, ce, &connmap);
  }
}

ll_head *getAllConnections()
{
  ll_head *ll = init_ll();  
  hash_enumerate(&connmap, &ll);
  
  return ll;
}

int getCMEntry(const NID nid, NID32 *dst)
{ 
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &connmap);
  static char aux[INET6_ADDRSTRLEN];
  
  if (ce){
    if ((ce->tstamp == 0) || (ce->tstamp + RVS_PROACTIVE >  time(NULL))){
      *dst = ce->ip;
      return 1;
    }    
    else {
      /* pro-active */
      if (ce->tstamp + RVS_UPDATE_TIMEOUT >  time(NULL)){
	*dst = ce->ip;
	//printf("tstamp = %d isQueued(%s)=%d\n", time(NULL), nid2char(nid), isQueued(nid));
	if (!isQueued(nid)){
	  inet_ntop(AF_INET6, &nid, aux, INET6_ADDRSTRLEN);
	  //printf("RVS_REQUEST\ntstamp = %d for NID = %s\n", (int)time(NULL), aux);
	  queue(nid);
	  write(sp_cm2rvs[1], &nid, 16);
	  return 1;
	}
	return 1;	
      }
      else {
	/* timeout */
	hash_del(nid, &connmap);
      }
    }
  }
  
  newCMEntry(nid, cm_gatewayIP);
  write(sp_cm2rvs[1], &nid, 16);
  *dst = cm_gatewayIP;
  
  return 0;
}

void delCMEntry(const NID nid){
  hash_del(nid, &connmap);  
}

void cleanCM(){
  ll_head *ll = getAllConnections();  
  ll_node *lpt = ll->next;

  hash_enumerate(&connmap, &ll);
  
  while (lpt){
    hash_del(lpt->nid, &connmap);
    lpt = lpt->next;
  }
  
  free(ll);
}

void init_conn_mapper()
{ 
  cm_nidgw = getNIDgw();
  cm_nidrvs = getNIDrvs();
  cm_gatewayIP = getGatewayIP();
  cm_rvsIP = getRvsIP();
  printf("Creating Connection Mapper table...\n");
  construct_hash_table(&connmap, 20);
}

void destroy_conn_mapper()
{
  free_hash_table(&connmap, NULL);
  printf("Destroying Connection Mapper table...\n");
}

void updateCMconf()
{ 
  ll_head *ll;
  ll_node *lpt;
  NID_msg nmsg;
    
  hash_del(cm_nidgw, &connmap);
  hash_del(cm_nidrvs, &connmap);
  
  cleanCM();    
  
  cm_nidgw = getNIDgw();
  cm_nidrvs = getNIDrvs();
  cm_gatewayIP = getGatewayIP();
  cm_rvsIP = getRvsIP();
  
  staticCMEntry(cm_nidgw, cm_gatewayIP);
  staticCMEntry(cm_nidrvs, cm_rvsIP);  
}
