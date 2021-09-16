/*
 * The MIT License (MIT)
 * Copyright (c) 2021, Walter Wong
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  Author: Walter Wong <walterwong@gmail.com>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define MAXBUFLEN 256

static int open_netlink()
{
  static struct sockaddr_nl addr;
  static int nl_socket;
  
  bzero(&addr, sizeof(addr));
  
  if ((nl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0){
    perror("socket Netlink");
    exit(EXIT_FAILURE);
  }
  
  addr.nl_family = AF_NETLINK;
  addr.nl_pid = getpid();
  addr.nl_groups = RTM_DELLINK;
  
  if (bind(nl_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0){
    perror("bind Netlink");
    exit(EXIT_FAILURE);
  }

  return nl_socket;
}


static int read_event(int sock)
{
  static struct sockaddr_nl nladdr;
  static struct msghdr msg;
  static struct iovec iov[2];
  static struct nlmsghdr nlh;
  static char buffer[65536];
  static int ret;

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
    exit(EXIT_FAILURE);
  }

  return (nlh.nlmsg_type == RTM_NEWLINK);
}

int main(int argc, char *argv[])
{
  int count = 0;
  int nl_socket = open_netlink();
  char command[32];
  struct timeval tim;
  double t_start, t_link, t_dhcp;
  
  if (argc < 3){
    printf("Usage: %s <interface> <essid>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  gettimeofday(&tim, NULL);
  t_start = tim.tv_sec + (tim.tv_usec/1000000.0);

  sprintf(command, "iwconfig %s essid off", argv[1]);
  system(command);
  memset(&command, 0, 32);
  
  sprintf(command, "iwconfig %s key off", argv[1]);
  system(command);
  memset(&command, 0, 32);
  
  sprintf(command, "iwconfig %s essid %s", argv[1], argv[2]);
  system(command);
  
  while(1){
    
    if (read_event(nl_socket)){
      count++;
      printf(".");
      if (count >= 5){
	printf("\n");
	gettimeofday(&tim, NULL);
	t_link = tim.tv_sec + (tim.tv_usec/1000000.0);
	system("rm -fr /etc/dhcpc/*");
	sprintf(command, "./dhcpcd -YR %s", argv[1]);
	system(command);
	gettimeofday(&tim, NULL);
	t_dhcp = tim.tv_sec + (tim.tv_usec/1000000.0);
	printf("Tempo para conectar a nova antena:\t\t %.3lf ms\n",
	       1000 * (t_link - t_start));
	printf("Tempo para receber configuracoes do DHCP:\t %.3lf ms\n",
	       1000 * (t_dhcp - t_link));
	printf("Tempo total:\t\t\t\t\t %.3lf ms\n",
	       1000 * (t_dhcp - t_start));
	break;
      }
    }
  }
  
  return 0;
}
