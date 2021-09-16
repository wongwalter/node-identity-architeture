#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "nid.h"
#include "configuration.h"
#include "rvs_queue_table.h"
#include "tools.h"

extern int sp_rvs2ph[2];
extern int sp_co2rvs[2];
extern int sp_cm2rvs[2];
extern int sp_rvs2gwt[2];
extern int sp_gwt2ph[2];

void *thread_rvs_proxy_send(void)
{
  const int s 				= new_socket();
  const int addr_len			= sizeof(struct sockaddr);
  int recv_bytes			= 0;
  const int maxfd 			= sp_co2rvs[0] + 1;
  const int nid_msg_len 		= sizeof(NID_msg);
  const int rvs_msg_len 		= sizeof(RVS_msg);
  struct sockaddr_in peer;
  fd_set readfds;
  NID_msg nmsg;
  RVS_msg rvs_msg;
  
  peer.sin_family = AF_INET;
  peer.sin_port = htons(RVS_LISTENER);
  inet_pton(AF_INET, "0.0.0.0", &peer.sin_addr.s_addr);
  bzero(&peer.sin_zero, 8);

  FD_ZERO(&readfds);
  
  while (1){
    FD_SET(sp_co2rvs[0], &readfds);    
    select(maxfd, &readfds, NULL, NULL, NULL);

    if (FD_ISSET(sp_co2rvs[0], &readfds)){
      
      recv_bytes = read(sp_co2rvs[0], &nmsg, nid_msg_len);
      
      rvs_msg.type = nmsg.type;
      rvs_msg.srcNID = nmsg.srcNID;
      rvs_msg.destNID = nmsg.destNID;
      rvs_msg.srcNIDgw = nmsg.srcNIDgw;
      rvs_msg.destNIDgw = nmsg.destNIDgw;
      rvs_msg.nid = nmsg.nid;
      rvs_msg.ip = nmsg.ip;

      if (sendto(s, &rvs_msg, rvs_msg_len, 0, (struct sockaddr *)&peer, 
		 (socklen_t) addr_len) < 0) {
	perror("thread_rvs_proxy_send sendto");
	exit(1);
      }      
    }
  }
}


void *thread_rvs_proxy_recv(void)
{
  const int s 				= new_socket();
  const int addr_len			= sizeof(struct sockaddr);
  int recv_bytes			= 0;
  const int nmsg_len 			= sizeof(NID_msg);
  const int rvs_msg_len 		= sizeof(RVS_msg);
  const int maxfd 			= s + 1;
  struct sockaddr_in local;
  struct sockaddr_in peer;
  fd_set readfds;
  RVS_msg rvs_msg;
  NID_msg nmsg;

  local.sin_family = AF_INET;
  local.sin_port = htons(RVS_TALKER);
  local.sin_addr.s_addr = INADDR_ANY;
  bzero(&local.sin_zero, 8);

  set_skt_reuseaddr(s);

  if (bind(s, (struct sockaddr *)&local, addr_len) < 0) {
    perror("thread_rvs_proxy_recv bind");
    exit(1);
  }

  FD_ZERO(&readfds);

  while (1){
    FD_SET(s, &readfds);    
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(s, &readfds)){      
      
      recv_bytes = recvfrom(s, &rvs_msg, rvs_msg_len, 0, (struct sockaddr *)
			    &peer, (socklen_t *)&addr_len);
      
      nmsg.type = rvs_msg.type;
      nmsg.srcNID = rvs_msg.destNID;
      nmsg.destNID = rvs_msg.srcNID;
      nmsg.srcNIDgw = rvs_msg.srcNIDgw;
      nmsg.destNIDgw = rvs_msg.destNIDgw;
      nmsg.nid = rvs_msg.nid;
      nmsg.ip = rvs_msg.ip;

      write(sp_rvs2ph[1], &nmsg, nmsg_len);
      
      if (nmsg.type == RVS_ACK){
	write(sp_rvs2gwt[1], &nmsg, nmsg_len);
      }
    }
  }    
}

void *thread_rvs_update(void)
{  
  NID_msg nmsg;

  bzero(&nmsg, sizeof(nmsg));
  nmsg.type = RVS_UPDATE;
  nmsg.nid = nmsg.srcNID = getNID();
  nmsg.srcNIDgw = getNIDcorehome();
  
  while (1){
    nmsg.destNID = getNIDrvs();
    inet_pton(AF_INET, get_locator(getPrimaryIface()), &nmsg.ip);
    write(sp_rvs2ph[1], &nmsg, sizeof(nmsg));
    sleep(RVS_UPDATE_TIMEOUT);
  }
}

void *thread_rvs_get(void)
{  
  const int maxfd			= sp_cm2rvs[0] + 1;
  const int nmsg_len 			= sizeof(NID_msg);
  NID nid 				= getNID();
  NID nidrvs 				= getNIDrvs();
  fd_set readfds;
  NID_msg nmsg;
  
  bzero(&nmsg, sizeof(nmsg));

  nmsg.type = RVS_GET;
  nmsg.srcNID = nid;
  nmsg.destNID = nidrvs;

  FD_ZERO(&readfds);
  
  while (1){
    
    FD_SET(sp_cm2rvs[0], &readfds);    
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_cm2rvs[0], &readfds)){      
      read(sp_cm2rvs[0], &nmsg.nid, nmsg_len);
      //printf("resolve nid %s\n", nid2char(nmsg.nid));
      fflush(stdout);
      nmsg.destNID = getNIDrvs();     
      write(sp_rvs2ph[1], &nmsg, sizeof(nmsg));      
    }
  }
}

void *thread_rvs2gw(void)
{
  const int nmsg_len 			= sizeof(NID_msg);
  const int maxfd 			= sp_rvs2gwt[0] + 1;
  NID_msg nmsg;
  fd_set readfds;

  bzero(&nmsg, sizeof(nmsg));      
  
  FD_ZERO(&readfds);
  
  while(1){
    
    FD_SET(sp_rvs2gwt[0], &readfds);
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_rvs2gwt[0], &readfds)){           
      read(sp_rvs2gwt[0], &nmsg, nmsg_len);
      
      nmsg.type = REGISTRY;
      nmsg.destNID = getNIDgw();
      write(sp_gwt2ph[1], &nmsg, nmsg_len);
    }
  }
}

