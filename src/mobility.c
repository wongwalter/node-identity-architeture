#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <asm/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "nid.h"
#include "connection_mapper.h"
#include "configuration.h"

#define MAXBUFLEN 256

extern int sp_mo2ph[2], sp_ph2co[2];

static int open_netlink()
{
  struct sockaddr_nl addr;
  int nl_socket;
  
  bzero(&addr, sizeof(addr));
  
  if ((nl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0){
    perror("socket Netlink");
    exit(-1);
  }
  
  addr.nl_family = AF_NETLINK;
  addr.nl_pid = getpid();
  addr.nl_groups = RTM_NEWADDR;
  
  if (bind(nl_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0){
    perror("bind Netlink");
    return -1;
  }

  return nl_socket;
}


static int read_event(int sock)
{
  struct sockaddr_nl nladdr;
  struct msghdr msg;
  struct iovec iov[2];
  struct nlmsghdr nlh;
  char buffer[65536];
  int ret;
  
  iov[0].iov_base = (void *)&nlh;
  iov[0].iov_len = sizeof(nlh);
  iov[1].iov_base = (void *)buffer;
  iov[1].iov_len = sizeof(buffer);
  
  msg.msg_name = (void *)&(nladdr);
  msg.msg_namelen = sizeof(nladdr);
  msg.msg_iov = iov;
  msg.msg_iovlen = sizeof(iov)/sizeof(iov[0]);
  
  if ((ret = recvmsg(sock, &msg, 0)) < 0){
    perror("recvmsg Netlink");
    exit(-1);
  }

  return (nlh.nlmsg_type == RTM_NEWADDR);
}

void rvs_update(void)
{  
  NID_msg nmsg;
  
  memset(&nmsg, 0, sizeof(nmsg));
  nmsg.type = RVS_UPDATE;
  nmsg.nid = nmsg.srcNID = getNID();
  nmsg.destNID = getNIDrvs();
  inet_pton(AF_INET, get_locator(getPrimaryIface()), &nmsg.ip);
  write(sp_rvs2ph[1], &nmsg, sizeof(nmsg));
}

void *thread_mobility(void)
{
  int nl_socket = 0;
  int count = 0;  
  ll_head *ll;
  ll_node *lpt;
  NID_msg nmsg;
  struct timespec tv;
  
  nl_socket = open_netlink();    
  
  while (1){
    if (read_event(nl_socket)){
      count++;

      if (count >= 2){
	memset(&tv, 0, sizeof(tv));
	tv.tv_nsec = 1000000;
	nanosleep(&tv, NULL);
	
	memset(&nmsg, 0, sizeof(nmsg));
	nmsg.nid = nmsg.srcNID = getNID();

	ll = getAllConnections();
	lpt = ll->next;

	if (get_mobility() == INTRADOMAIN){
	  nmsg.type = REDIRECT;
	  while (lpt){
	    nmsg.destNID = lpt->nid;
	    inet_pton(AF_INET, get_locator(getPrimaryIface()), &nmsg.ip);
	    write(sp_mo2ph[1], &nmsg, sizeof(nmsg));	  
	    lpt = lpt->next;
	  }
	}
	/*
	else {
	  while (lpt){
	    nmsg.type = RELOCATE;
	    nmsg.destNID = lpt->nid;
	    write(sp_mo2ph[1], &nmsg, sizeof(nmsg));	  
	    lpt = lpt->next;
	  }	  	  
	}
	*/
	updateCMconf();
	count = 0;
	rvs_update();
	free(ll);
      }
    }
  } 
}


