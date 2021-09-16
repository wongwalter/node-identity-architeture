#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nid.h"
#include "hash.h"
#include "configuration.h"
#include "tools.h"

#define MAXBUFLEN 256

static int init_rvs_table();
static void destroy_rvs_table();
static int addEntry(const NID nid, const NID32 ip);
NID32 getEntry(const NID nid);

static hash_table rvs_table;

int main(int argc, char **argv)
{
  const int talker 				= new_socket();
  const int listener 				= new_socket();
  int numbytes 					= 0;
  const socklen_t addr_len 			= sizeof(struct sockaddr);
  struct sockaddr_in local;
  struct sockaddr_in peer;
  char buf[MAXBUFLEN];
  char aux[INET_ADDRSTRLEN];
  RVS_msg *rvs_pt 				= (RVS_msg *) buf;
  
  local.sin_family = AF_INET;
  local.sin_port = htons(RVS_LISTENER);
  local.sin_addr.s_addr = INADDR_ANY;
  bzero(&local.sin_zero, 8);
  
  set_skt_reuseaddr(listener);

  if (bind(listener, (struct sockaddr *)&local, addr_len) < 0) {
    perror("bind");
    exit(1);
  }  

  printf("RVS up! Ready to receive messages at port %d\n", RVS_LISTENER);
  init_rvs_table();

  while (1){
    
    if ((numbytes = recvfrom(listener, buf, MAXBUFLEN-1, 0, (struct sockaddr *)
			     &peer, (socklen_t *)&addr_len)) < 0) {
      perror("recvfrom");
      exit(1);
    }        
    
    switch(rvs_pt->type){
    case RVS_UPDATE:
      printf("RVS_UPDATE\n");
      print_nid(rvs_pt->nid, "rvs");
      print_ipv4(rvs_pt->ip, "rvs");
      fflush(stdout);
      addEntry(rvs_pt->nid, rvs_pt->ip);
      rvs_pt->type = RVS_ACK;
      break;
    case RVS_GET:
      printf("RVS_GET\n");
      print_nid(rvs_pt->nid, "rvs");
      fflush(stdout);
      rvs_pt->ip = getEntry(rvs_pt->nid);
      if (rvs_pt->ip){
	inet_ntop(AF_INET, &rvs_pt->ip, aux, INET_ADDRSTRLEN);
	printf(">>>ip: %s\n", aux);
      }
      rvs_pt->type = RVS_RESPONSE;
      break;
    default:
      printf("type %d undefined\n", rvs_pt->type);
      break;
    }        
    
    peer.sin_port = htons(RVS_TALKER);
    
    if ((numbytes = sendto(talker, buf, numbytes, 0, (struct sockaddr *)
			   &peer, (socklen_t)addr_len)) < 0) {
      perror("sendto");
      exit(1);
    }
  }
  
  close(listener);
  close(talker);
  destroy_rvs_table();

  return 0;
} 


/* Internal RVS table functions */

static int init_rvs_table(){ 
  return (construct_hash_table(&rvs_table, 100) ? 1 : 0);
}

static void destroy_rvs_table(){
  free_hash_table(&rvs_table, NULL);
}

static int addEntry(const NID nid, const NID32 ip)
{
  conn_entry *ce = (conn_entry *) calloc(1, sizeof(conn_entry));
  
  ce->ip = ip;
  ce->tstamp = time(NULL);
  
  return (hash_insert(nid, ce, &rvs_table) ? 1 : 0);
}

NID32 getEntry(const NID nid)
{
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &rvs_table);
  
  if (ce){    
    if (ce->tstamp + RVS_TIMEOUT >  time(NULL)){
      return ce->ip;
    }
    else {
      hash_del(nid, &rvs_table);
    }
  }  
  
  return 0;
}
