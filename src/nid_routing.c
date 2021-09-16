#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <fcntl.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <time.h>

#include "hash.h"
#include "configuration.h"
#include "connection_mapper.h"
#include "nid_routing.h"
#include "tools.h"
#include "gwc.h"
#include "dhtc.h"

#define MAXBUFLEN 			1500
#define SOCKET_BUFFER_FACTOR       	2

extern int sp_ph2nr[2];
extern int sp_nr2ph[2];
extern int sp_nr[2];

hash_table rtable;

NID    nidgw;
NID32  gatewayIP;

void *thread_nr_output(void)
{
  const int s 				= new_socket();
  const int maxfd			= sp_ph2nr[0] + 1;
  const int sock_len			= sizeof(struct sockaddr);  
  int recv_bytes			= 0;
  u_char buf[MAXBUFLEN];
  NID_header *nh_pt 			= (NID_header *) buf;
  struct sockaddr_in peer;
  fd_set readfds;
  
  set_skt_nonblock(s);
  set_skt_reuseaddr(s);
  //set_skt_sndbuf(s, get_skt_sndbuf(s) * SOCKET_BUFFER_FACTOR);

  peer.sin_family = AF_INET;  
  peer.sin_port = htons(DATA_PORT);
  bzero(&peer.sin_zero, 8);
  
  FD_ZERO(&readfds);

  while(1){
    
    FD_SET(sp_ph2nr[0], &readfds); 
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_ph2nr[0], &readfds)){
      /* Coming from Packet Handler Output */
            
      recv_bytes = read(sp_ph2nr[0], buf, MAXBUFLEN);
      getCMEntry(nh_pt->destNID, &peer.sin_addr.s_addr);
      
#ifdef DEBUG_NR_OUT
      printf("NR sendto NID=%s IP=%s\n", nid2char(nh_pt->destNID), 
	     nid322char(peer.sin_addr.s_addr));
#endif 
      
      if (sendto(s, buf, recv_bytes, 0, (struct sockaddr *) &peer, sock_len) < 0){
	perror("(NR) sendto");
      }
    }
  }
}


void *thread_nr_output_gw(void)
{
  const int s 				= new_socket();
  const int sock_len 			= sizeof(struct sockaddr);
  const int maxfd 			= MAX(sp_ph2nr[0], sp_nr[0]) + 1;
  int recv_bytes			= 0;
  u_char buf[MAXBUFLEN];
  NID_header *nh_pt 			= (NID_header *) buf;
  struct sockaddr_in peer;
  fd_set readfds;

  set_skt_reuseaddr(s);
  set_skt_nonblock(s);
  //set_skt_sndbuf(s, get_skt_sndbuf(s) * SOCKET_BUFFER_FACTOR);
  
  peer.sin_family = AF_INET;  
  peer.sin_port = htons(DATA_PORT);
  bzero(&peer.sin_zero, 8);
  
  FD_ZERO(&readfds);
  
  while(1){
    
    FD_SET(sp_ph2nr[0], &readfds); /* Coming from Packet Handler Output */
    FD_SET(sp_nr[0], &readfds);    /* Coming from NID Routing Output */
    
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_ph2nr[0], &readfds)){
      /* Incoming packets from PH */
      
      recv_bytes = read(sp_ph2nr[0], buf, MAXBUFLEN);


      if (!getCMEntry(nh_pt->destNID, &peer.sin_addr.s_addr)){	
	getRTEntry(nh_pt->destNID, &peer.sin_addr.s_addr);
      }
      
      if (sendto(s, buf, recv_bytes, 0, (struct sockaddr *) &peer, sock_len) < 0){
	perror("(NR) sendto");	
	printf("NR sendto error IP=%s\n", nid322char(peer.sin_addr.s_addr));
      }
#ifdef DEBUG_NR_OUT
      printf("NR sendto NID=%s IP=%s\n", 
	     nid2char(nh_pt->destNID), 
	     nid322char(peer.sin_addr.s_addr));
#endif
      
    }
    else {
      if (FD_ISSET(sp_nr[0], &readfds)){
	/* Incoming packets from NR then it means that the Gateway 
	 is routing. Don't need to look the CM because it isn't a connection,
	 just a routing action.
	*/
	
	recv_bytes = read(sp_nr[0], buf, MAXBUFLEN);
	
	/* Routing Table lookup. If there isn't any entry in the RT, then
	   it will return the locator of the next gateway */
	getRTEntry(nh_pt->destNID, &peer.sin_addr.s_addr);
	
	if (sendto(s, buf, recv_bytes, 0, (struct sockaddr *)&peer, sock_len) < 0){
	  perror("(NR) sendto");
	  printf("NR sendto error IP=%s\n", nid322char(peer.sin_addr.s_addr));
	}
#ifdef DEBUG_NR_OUT
	printf("NR-GW sendto NID=%s IP=%s\n", nid2char(nh_pt->destNID), 
	       nid322char(peer.sin_addr.s_addr));
#endif
      }
    }
  }
}


void *thread_nr_output_core(void)
{
  const int s 				= new_socket();
  const int sock_len			= sizeof(struct sockaddr);
  const int maxfd 			= MAX(sp_ph2nr[0], sp_nr[0]) + 1;
  int recv_bytes			= 0;
  u_char buf[MAXBUFLEN];
  NID_header *nh_pt 			= (NID_header *) buf;
  struct sockaddr_in peer;
  fd_set readfds;

  set_skt_reuseaddr(s);
  set_skt_nonblock(s);
  //set_skt_sndbuf(s, get_skt_sndbuf(s) * SOCKET_BUFFER_FACTOR);
  
  peer.sin_family = AF_INET;  
  peer.sin_port = htons(DATA_PORT);
  bzero(&peer.sin_zero, 8);
  
  FD_ZERO(&readfds);
  
  while(1){
    
    FD_SET(sp_ph2nr[0], &readfds); /* Coming from Packet Handler Output */
    FD_SET(sp_nr[0], &readfds);    /* Coming from NID Routing Output */
    
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_ph2nr[0], &readfds)){
      /* Incoming packets from PH 
	 Gateway Core is acting as a node */      
      
      recv_bytes = read(sp_ph2nr[0], buf, MAXBUFLEN);

      if (!getCMEntry(nh_pt->destNID, &peer.sin_addr.s_addr)){
	if (!getRTEntry(nh_pt->destNID, &peer.sin_addr.s_addr)){
	  if (!getRegistry(nh_pt->destNID, &peer.sin_addr.s_addr)){
	    if (!getDHTEntry(nh_pt->destNIDgw, &peer.sin_addr.s_addr)){
	      continue;
	    }
	  }
	}
      }
      
      if (sendto(s, buf, recv_bytes, 0, (struct sockaddr *) &peer, sock_len) < 0){
	perror("NR sendto");	
	printf("NRO_GWC sendto error IP=%s\n", nid322char(peer.sin_addr.s_addr));
      }

#ifdef DEBUG_NR_OUT
      printf("NRO-GWC sendto NID=%s IP=%s\n", nid2char(nh_pt->destNID), 
	     nid322char(peer.sin_addr.s_addr));
#endif
    }
    else {
      if (FD_ISSET(sp_nr[0], &readfds)){
	/* Incoming packets from NR then it means that the Gateway Core
	   is routing. Don't need to look the CM because it isn't a connection,
	   just a routing action.
	*/
	
	recv_bytes = read(sp_nr[0], buf, MAXBUFLEN);
	
	if (!getRTEntry(nh_pt->destNID, &peer.sin_addr.s_addr)){
	  if (!getRegistry(nh_pt->destNID, &peer.sin_addr.s_addr)){
	    if (!getDHTEntry(nh_pt->destNIDgw, &peer.sin_addr.s_addr)){
	      continue;
	    }
	  }
	}
	
	if (sendto(s, buf, recv_bytes, 0, (struct sockaddr *)&peer, sock_len) < 0){
	  perror("NRO-GWC sendto");
	  printf("NRO-GWC sendto error IP=%s\n", nid322char(peer.sin_addr.s_addr));
	}
	
#ifdef DEBUG_NR_OUT
	printf("NRO-GWC sendto NID=%s IP=%s\n", nid2char(nh_pt->destNID), 
	       nid322char(peer.sin_addr.s_addr));
#endif
      }
    }
  }
}

void *thread_nr_input(void)
{
  const int s 				= new_socket();
  const int addr_len 			= sizeof(struct sockaddr_in);
  const int maxfd 			= s + 1;
  int recv_bytes			= 0;
  u_char buf[MAXBUFLEN];
  NID_header *nh_pt 			= (NID_header *) buf;
  struct sockaddr_in local;
  struct sockaddr_in recv_addr; 
  NID nid 				= getNID();
  fd_set readfds;
  
  //set_skt_rcvbuf(s, get_skt_rcvbuf(s) * SOCKET_BUFFER_FACTOR);

  local.sin_family = AF_INET;
  local.sin_port = htons(DATA_PORT);
  local.sin_addr.s_addr = INADDR_ANY;
  bzero(&local.sin_zero, 8);
  
  if (bind(s, (struct sockaddr *)&local, sizeof(struct sockaddr)) < 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }
  
  FD_ZERO(&readfds);
    
  while(1){

    FD_SET(s, &readfds);

    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(s, &readfds)){ 
      /* Receiving packets from network */      
      
      if ((recv_bytes = recvfrom(s, buf, MAXBUFLEN, 0, (struct sockaddr *)
			       &recv_addr, (socklen_t *) &addr_len)) < 0){
	perror("recvfrom");
      }
      
#ifdef DEBUG_NR_IN
      printf("NRI recvfrom %s\n", nid322char(recv_addr.sin_addr.s_addr));
#endif
      
      if (!memcmp(&nh_pt->destNID, &nid, 16)){
	newCMEntry(nh_pt->srcNID, recv_addr.sin_addr.s_addr);
	write(sp_nr2ph[1], buf, recv_bytes);
      }
    }
  }
}


void *thread_nr_input_gw(void)
{
  const int s 				= new_socket();
  const int addr_len 			= sizeof(struct sockaddr_in);
  const int maxfd 			= s + 1;
  int recv_bytes			= 0;
  u_char buf[MAXBUFLEN];
  NID_header *nh_pt 			= (NID_header *) buf;
  struct sockaddr_in local;
  struct sockaddr_in recv_addr; 
  NID nid 				= getNID();
  fd_set readfds;
  
  //set_skt_rcvbuf(s, get_skt_rcvbuf(s) * SOCKET_BUFFER_FACTOR);

  local.sin_family = AF_INET;
  local.sin_port = htons(DATA_PORT);
  local.sin_addr.s_addr = INADDR_ANY;
  bzero(&local.sin_zero, 8);
  
  if (bind(s, (struct sockaddr *)&local, addr_len) < 0) {
    perror("bind");
    exit(EXIT_FAILURE);
  }
  
  FD_ZERO(&readfds);
  
  while(1){
    
    FD_SET(s, &readfds);    
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(s, &readfds)){ 
      /* Receiving packets from network */
      
      if ((recv_bytes = recvfrom(s, buf, MAXBUFLEN, 0, (struct sockaddr *)
			       &recv_addr, (socklen_t *) &addr_len)) < 0){
	perror("NRI-GW recvfrom");
      }
      
#ifdef DEBUG_NR_IN
      printf("NRI-GW recvfrom %s\n", nid322char(recv_addr.sin_addr.s_addr));
#endif      
      
      if (!memcmp(&nh_pt->destNID, &nid, 16)){
	/* It is local, send to PHI */
	newCMEntry(nh_pt->srcNID, recv_addr.sin_addr.s_addr);
	write(sp_nr2ph[1], buf, recv_bytes);
      }
      else {
	/* Not local, then route */
	write(sp_nr[1], buf, recv_bytes);
      }
    }
  }

  return 0;
}


/* Routing table functions */

void init_rtable(){ 
  construct_hash_table(&rtable, 30);
  nidgw = getNIDgw();
  gatewayIP = getGatewayIP();
}

void destroy_rtable(){
  free_hash_table(&rtable, NULL);
  printf("Destroying NID Routing table...\n");
}

void newRTEntry(const NID nid, const NID32 ip)
{
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &rtable);
  
  if (ce){
    if (ce->tstamp == 0)
      return;
    else {
      ce->ip = ip;
      ce->tstamp = time(NULL);
      hash_insert(nid, ce, &rtable);
      return;
    }
  }
  
  ce = (conn_entry *) calloc(1, sizeof(conn_entry));
  ce->ip = ip;
  ce->tstamp = time(NULL);
  
  hash_insert(nid, ce, &rtable);
}

void staticRTEntry(const NID nid, const NID32 ip)
{
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &rtable);
  
  if (ce){        
    ce->ip = ip;
    hash_insert(nid, ce, &rtable);
  }
  else {
    ce = (conn_entry *) calloc(1, sizeof(conn_entry));
    
    ce->ip = ip;
    ce->tstamp = 0;
    hash_insert(nid, ce, &rtable);
  }
}

int getRTEntry(const NID nid, NID32 *dst)
{
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &rtable);
  
  if (ce){
    if ((ce->tstamp + GATEWAY_TIMEOUT >  time(NULL)) || (ce->tstamp == 0)){
      if (ce->ip != 0){
	*dst = ce->ip;
	return 1;
      }
    }
    else {
      hash_del(nid, &rtable);
    }
  }
  
  *dst = gatewayIP;
  
  return 0;
}

ll_head *getAllRTEntries()
{
  ll_head *ll = init_ll();  
  hash_enumerate(&rtable, &ll);
  
  return ll;
}

void updateRT()
{
  hash_del(nidgw, &rtable);
  
  nidgw = getNIDgw();
  gatewayIP = getGatewayIP();
}

/* Thread debug RT*/

void *thread_debug_rt(void)
{
  ll_head *ll;
  ll_node *lpt;
  char aux[128];

  while (1){
    
    ll = getAllRTEntries();
    lpt = ll->next;      

    printf("----------Routing Table----------\n");
    printf("RT size = %d\n", ll->size);
    
    while (lpt){
      inet_ntop(AF_INET6, &lpt->nid, aux, 128);
      printf("NID: %s \tIP: %s\n", aux, lpt->ip);
      lpt = lpt->next;
      fflush(stdout);
    }
    
    destroy_ll(ll);
    sleep(DEBUG_PRINT_RT_TIME);
  }
}
