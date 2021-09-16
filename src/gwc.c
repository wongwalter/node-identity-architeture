#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#include "nid.h"
#include "configuration.h"
#include "connection_mapper.h"
#include "nid_routing.h"
#include "gwc.h"

extern int sp_gwt2ph[2];
extern int sp_co2gwl[2];
extern int sp_rvs2gwt[2];
extern int sp_gwl2gwt[2];

hash_table registry_table;

void *thread_gw_listener(void)
{
  const int nmsg_len			= sizeof(NID_msg);
  const int maxfd			= sp_co2gwl[0] + 1;
  NID_msg nmsg;
  fd_set readfds;
  
  FD_ZERO(&readfds);
  
  while (1){
    FD_SET(sp_co2gwl[0], &readfds);
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_co2gwl[0], &readfds)){      
      read(sp_co2gwl[0], &nmsg, nmsg_len);

      newRTEntry(nmsg.nid, nmsg.ip);      
      write(sp_gwl2gwt[1], &nmsg, nmsg_len);
    }    
  }
}


void *thread_gw_core_listener(void)
{
  const int nmsg_len			= sizeof(NID_msg);
  const int maxfd			= sp_co2gwl[0] + 1;
  NID nid = getNID();
  NID_msg nmsg;
  fd_set readfds;
  
  FD_ZERO(&readfds);
  
  while (1){
    FD_SET(sp_co2gwl[0], &readfds);
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_co2gwl[0], &readfds)){      
      read(sp_co2gwl[0], &nmsg, nmsg_len);
      
      newRTEntry(nmsg.nid, nmsg.ip);

      if (!memcmp(&nmsg.srcNIDgw, &nid, 16)){
	/* Esta no seu home */
	addRegistry(nmsg.nid, 0);
      }
      else {
	/* Esta em outra arvore */	
	nmsg.type = REDIRECT_CORE;
	nmsg.srcNID = nid;
	nmsg.destNID = nmsg.srcNIDgw;
	nmsg.destNIDgw = nmsg.srcNIDgw;
	nmsg.srcNIDgw = nid;
	inet_pton(AF_INET, get_locator(getPrimaryIface()), &nmsg.ip);
	write(sp_gwt2ph[1], &nmsg, nmsg_len);
      }
    }    
  }
}


void *thread_gw_talker(void)
{
  const int nmsg_len 		= sizeof(NID_msg);
  const int maxfd		= (MAX(sp_rvs2gwt[0], sp_gwl2gwt[0]) + 1);
  NID_msg nmsg;
  fd_set readfds;

  FD_ZERO(&readfds);
  
  while(1){
    
    FD_SET(sp_rvs2gwt[0], &readfds);
    FD_SET(sp_gwl2gwt[0], &readfds);
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_rvs2gwt[0], &readfds)){      
      read(sp_rvs2gwt[0], &nmsg, nmsg_len);
      
      nmsg.type = REGISTRY;      
      nmsg.destNID = getNIDgw();
      inet_pton(AF_INET, get_locator(getPrimaryIface()), &nmsg.ip);
      write(sp_gwt2ph[1], &nmsg, nmsg_len);
    }
    else {
      if (FD_ISSET(sp_gwl2gwt[0], &readfds)){	
	read(sp_gwl2gwt[0], &nmsg, nmsg_len);

	nmsg.type = REGISTRY;
	nmsg.destNID = getNIDgw();
	inet_pton(AF_INET, get_locator(getPrimaryIface()), &nmsg.ip);
	write(sp_gwt2ph[1], &nmsg, nmsg_len);
      }
    }
  }
}

// Registry Table

void init_registry_table(){
  construct_hash_table(&registry_table, 30);
}

void destroy_registry_table(){
  free_hash_table(&registry_table, NULL);
  printf("Destroying Gateway Core Registry table...\n");
}

void addRegistry(const NID nid, const NID32 ip)
{
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &registry_table);
  
  if (ce){
    ce->ip = ip;
    hash_insert(nid, ce, &registry_table);
    return;
  }
  
  ce = (conn_entry *) calloc(1, sizeof(conn_entry));  
  ce->ip = ip;
  //printf("Added record NID = %s with IP = %s\n", nid2char(nid), nid322char(ip));

  hash_insert(nid, ce, &registry_table);
}

int getRegistry(const NID nid, NID32 *dst)
{
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &registry_table);
  
  if (ce){
    if (ce->ip != 0){
      //printf("getDHTEntry NID=%s IP=%s\n", nid2char(nid), nid322char(ce->ip));
      *dst = ce->ip;
      return 1;
    }    
  }
  
  *dst = 0;
  return 0;
}
