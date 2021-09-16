#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "nid.h"
#include "packetHandler.h"
#include "configuration.h"
#include "nid_mapper.h"
#include "tools.h"

#define MAXBUFLEN 1500

extern int sp_nf2ph[2];
extern int sp_ph2nr[2];
extern int sp_nr2ph[2];
extern int sp_mo2ph[2];
extern int sp_ph2co[2];
extern int sp_gwt2ph[2];
extern int sp_ph2gwl[2]; 
extern int sp_rvs2ph[2];

NID nidcore;

void *thread_ph_output(void)
{
  int recv_bytes			= 0;
  const int nh_len			= sizeof(NID_header);
  const int size			= MAXBUFLEN - nh_len;
  const int max				= MAX(MAX(sp_nf2ph[0], sp_mo2ph[0]), 
					      MAX(sp_rvs2ph[0], sp_gwt2ph[0])) + 1;    
  static u_char buf[MAXBUFLEN];
  u_char *pt 				= buf + nh_len;
  struct iphdr *ip 			= (struct iphdr *) pt;
  NID_header *nh_pt 			= (NID_header *) buf;
  NID nid 				= getNID();
  NID_msg *nmsg_pt 			= (NID_msg *) pt;
  NMEntry32 *nm_pt;
  fd_set readfds;
  
  nidcore = getNIDcore();

  /* Initial values for the NID Header */
  nh_pt->srcNID = nid;  

  FD_ZERO(&readfds);

  while(1){

    FD_SET(sp_nf2ph[0], &readfds);
    FD_SET(sp_rvs2ph[0], &readfds);
    FD_SET(sp_mo2ph[0], &readfds);
    FD_SET(sp_gwt2ph[0], &readfds);
    
    select(max, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_nf2ph[0], &readfds)){
            
      recv_bytes = read(sp_nf2ph[0], pt, size);
      
      if ((nm_pt = (NMEntry32 *) getNMEntry32(ip->daddr)) != NULL){
	nh_pt->type = DATA;
	nh_pt->srcNIDgw = nidcore;
	nh_pt->destNID = nm_pt->nid;
	nh_pt->destNIDgw = nm_pt->nidgw;
	write(sp_ph2nr[1], buf, recv_bytes + nh_len);
      }
      /*
      else {
	char aux[32];
	inet_ntop(AF_INET, &ip->daddr, (char *)aux, 32);
	printf("Nao encontrou entrada no NM p/ %s\n", aux);
      }
      */
    }
    else {
      if (FD_ISSET(sp_rvs2ph[0], &readfds)){
	/* Receive from the rvs module */	
	recv_bytes = read(sp_rvs2ph[0], pt, size);
	
	nh_pt->type = nmsg_pt->type;
	nh_pt->srcNID = nmsg_pt->srcNID;
	nh_pt->destNID = nmsg_pt->destNID;		

	/* Send packet to NID Routing */
	write(sp_ph2nr[1], buf, recv_bytes + nh_len);
      }
      else {
	if (FD_ISSET(sp_mo2ph[0], &readfds)){
	  /* Received control messages from Mobility */

	  recv_bytes = read(sp_mo2ph[0], pt, size);
	  
	  /* Insert NID Header */
	  nh_pt->type = nmsg_pt->type;
	  nh_pt->srcNID = nmsg_pt->srcNID;
	  nh_pt->destNID = nmsg_pt->destNID;
	  
	  write(sp_ph2nr[1], buf, recv_bytes + nh_len);
	}	
	else {
	  if (FD_ISSET(sp_gwt2ph[0], &readfds)){
	    /* Received control messages from gateway talker */
	    
	    recv_bytes = read(sp_gwt2ph[0], pt, size);
	    
	    nh_pt->type = nmsg_pt->type;
	    nh_pt->srcNID = nmsg_pt->srcNID;
	    nh_pt->destNID = nmsg_pt->destNID;
	    nh_pt->srcNIDgw = nmsg_pt->srcNIDgw;
	    nh_pt->destNIDgw = nmsg_pt->destNIDgw;
	    
	    write(sp_ph2nr[1], buf, recv_bytes + nh_len);
	  }
	}       
      }
    }
  }
}

void *thread_ph_input(void)
{
  int raw_tcp	 			= 0;
  int raw_udp 				= 0;
  int raw_icmp 				= 0;
  int recv_bytes			= 0;
  const int nh_len 			= sizeof(NID_header);
  const int addr_len 			= sizeof(struct sockaddr);
  const int maxfd			= sp_nr2ph[0] + 1;  
  u_char buf[MAXBUFLEN];
  u_char *pt 				= buf + nh_len;
  struct sockaddr_in addr;
  struct iphdr *ip 			= (struct iphdr *) pt;
  NID_header *nh_pt 			= (NID_header *) buf;
  fd_set readfds;

  if ((raw_tcp = socket(PF_INET, SOCK_RAW, IPPROTO_TCP)) < 0){
    perror("(PH) socket raw tcp");
    exit(-1);
  }
  
  if ((raw_udp = socket(PF_INET, SOCK_RAW, IPPROTO_UDP)) < 0){
    perror("(PH) socket raw udp");
    exit(-1);
  }
   
  if ((raw_icmp = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
    perror("(PH) socket raw icmp");
    exit(-1);
  }
  
  set_iphdr_included(raw_tcp);
  set_iphdr_included(raw_udp);
  set_iphdr_included(raw_icmp);
  set_skt_reuseaddr(raw_tcp);
  set_skt_reuseaddr(raw_udp);
  set_skt_reuseaddr(raw_icmp);    

  addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

  bzero(&addr.sin_zero, 8);  
  FD_ZERO(&readfds);
  
  while (1){
    
    FD_SET(sp_nr2ph[0], &readfds);    
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_nr2ph[0], &readfds)){
      
      recv_bytes = read(sp_nr2ph[0], buf, MAXBUFLEN);
      newNMEntry(nh_pt->srcNID, nh_pt->srcNIDgw);
      
      switch (nh_pt->type){
      case DATA:
	switch (ip->protocol){
	case IPPROTO_TCP:
	  if (sendto(raw_tcp, pt, recv_bytes - nh_len, 0, (struct sockaddr *) &addr, 
		     addr_len) < 0){
	    perror("sendto raw_tcp");
	    exit(-1);
	  }
	  break;
	case IPPROTO_UDP:
	  if (sendto(raw_udp, pt, recv_bytes - nh_len, 0, (struct sockaddr *) &addr, 
		     addr_len) < 0){
	    perror("sendto raw_udp");
	    exit(-1);
	  }
	  break;
	case IPPROTO_ICMP:
	  if (sendto(raw_icmp, pt, recv_bytes - nh_len, 0, (struct sockaddr *) &addr, 
		     addr_len) < 0){
	    perror("sendto raw_icmp");
	    exit(-1);
	  }
	  break;
	default:
	  printf("PHI - Unknown protocol %d\n", ip->protocol);
	  break;
	}
	break;
      case REDIRECT:
      case REGISTRY:
      case REDIRECT_CORE:
      case RVS_UPDATE:
      case RVS_ACK:
      case RVS_GET:
      case RVS_RESPONSE:
	write(sp_ph2co[1], pt, recv_bytes - nh_len);
	break;
      default:
	printf("PHI - Unknown message type %d\n", nh_pt->type);
	break;
      }
    }
  }
}


void updatePH(){
  nidcore = getNIDcore();
}
