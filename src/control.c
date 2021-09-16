#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#include "nid.h"
#include "configuration.h"
#include "connection_mapper.h"
#include "rvs_queue_table.h"
#include "gwc.h"

extern int sp_co2gwl[2];
extern int sp_gwt2ph[2];
extern int sp_ph2co[2];
extern int sp_co2rvs[2];

void (*thread_control(void))(void)
{    
  const int maxfd 			= sp_ph2co[0] + 1;
  const int nmsg_len 			= sizeof(NID_msg);
  NID_msg nmsg;
  fd_set readfds;
  static char aux[INET6_ADDRSTRLEN];
  
  FD_ZERO(&readfds);
  
  while(1){
    FD_SET(sp_ph2co[0], &readfds);
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_ph2co[0], &readfds)){
      read(sp_ph2co[0], &nmsg, nmsg_len);
      
      switch (nmsg.type){
      case REDIRECT:
	printf("REDIRECT\n");
	newCMEntry(nmsg.nid, nmsg.ip);
	break;
      case RELOCATE:
	printf("RELOCATE\n");
	delCMEntry(nmsg.nid);
	break;
      case RVS_ACK:
	//printf("RVS_ACK\n");
	//print_nid(nmsg.nid, "thr_control");
	//print_nid32(nmsg.ip, "thr_control");
	break;
      case RVS_RESPONSE:
	//printf("RVS_RESPONSE\n");
	if (nmsg.ip != 0){	  
	  //inet_ntop(AF_INET6, &nmsg.nid, aux, INET6_ADDRSTRLEN);
	  //printf("tstamp = %d for NID = %s ", (int) time(NULL), aux);
	  //inet_ntop(AF_INET, &nmsg.ip, aux, INET_ADDRSTRLEN);
	  //printf("is IP = %s\n", aux);
	  unqueue(nmsg.nid);
	  newCMEntry(nmsg.nid, nmsg.ip);
	}
	break;
      default:
	break;
      }
    }
  }  
}


void (*thread_control_rvs(void))(void)
{    
  const int maxfd 			= sp_ph2co[0] + 1;
  const int nmsg_len			= sizeof(NID_msg);
  fd_set readfds;
  NID_msg nmsg;
  
  FD_ZERO(&readfds);

  while(1){
    FD_SET(sp_ph2co[0], &readfds);
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_ph2co[0], &readfds)){
      
      read(sp_ph2co[0], &nmsg, nmsg_len);
      
      switch (nmsg.type){
      case REDIRECT:
	printf("REDIRECT\n");
	newCMEntry(nmsg.nid, nmsg.ip);
	break;
      case RELOCATE:
	printf("RELOCATE\n");
	delCMEntry(nmsg.nid);
	break;
      case RVS_UPDATE:
	//printf("RVS_UPDATE\n");      
      case RVS_GET:
	//printf("RVS_GET\n");
	write(sp_co2rvs[1], &nmsg, nmsg_len);
	break;
      default:
	break;
      }      
    }
  }  
}


void (*thread_control_gw(void))(void)
{    
  const int maxfd			= sp_ph2co[0] + 1;
  const int nmsg_len			= sizeof(NID_msg);
  NID_msg nmsg;
  fd_set readfds;
  
  FD_ZERO(&readfds);
  
  while(1){
    FD_SET(sp_ph2co[0], &readfds);
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_ph2co[0], &readfds)){
      
      read(sp_ph2co[0], &nmsg, nmsg_len);
      
      switch (nmsg.type){
      case REDIRECT:
	printf("REDIRECT\n");
	newCMEntry(nmsg.nid, nmsg.ip);
	break;
      case RELOCATE:
	printf("RELOCATE\n");
	delCMEntry(nmsg.nid);
	break;
      case REGISTRY:
	//printf("REGISTRY\n");
	write(sp_co2gwl[1], &nmsg, nmsg_len);
	break;
      case RVS_ACK:
	//printf("RVS_ACK\n");
	//print_nid(nmsg.nid, "thr_control");
	//print_nid32(nmsg.ip, "thr_control");
	break;
      case RVS_RESPONSE:
	//printf("RVS_RESPONSE\n");
	//print_nid(nmsg.nid, "thr_control");
	//print_nid32(nmsg.ip, "thr_control");
	break;
      default:
	break;
      }      
    }
  }  
}


void (*thread_control_gw_core(void))(void)
{    
  const int maxfd			= sp_ph2co[0] + 1;
  const int nmsg_len			= sizeof(NID_msg);
  NID_msg nmsg;
  fd_set readfds;
  
  FD_ZERO(&readfds);
  
  while(1){
    FD_SET(sp_ph2co[0], &readfds);
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_ph2co[0], &readfds)){
      
      read(sp_ph2co[0], &nmsg, nmsg_len);

      switch (nmsg.type){
      case REDIRECT:
	printf("REDIRECT\n");
	newCMEntry(nmsg.nid, nmsg.ip);
	break;
      case RELOCATE:
	printf("RELOCATE\n");
	delCMEntry(nmsg.nid);
	break;
      case REGISTRY:
	//printf("REGISTRY\n");
	write(sp_co2gwl[1], &nmsg, nmsg_len);
	break;
      case REDIRECT_CORE:
	printf("REDIRECT_CORE\n");
	addRegistry(nmsg.nid, nmsg.ip);
	break;
      case RVS_ACK:
	//printf("RVS_ACK\n");
	//print_nid(nmsg.nid, "thr_control");
	//print_nid32(nmsg.ip, "thr_control");
	break;
      case RVS_RESPONSE:
	//printf("RVS_RESPONSE\n");
	//print_nid(nmsg.nid, "thr_control");
	//print_nid32(nmsg.ip, "thr_control");
	break;
      default:
	printf("control protocol unknown %d\n", nmsg.type);
	break;
      }      
    }
  }  
}


