#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include "tools.h"

/* set_non_blocking receives a socket as parameters and set it
 * to be non-blocking.
 */
void set_skt_nonblock(const int s)
{
  static int flags;
  
  if (( flags = fcntl(s, F_GETFL, 0)) < 0){
    perror("fcntl F_GETFL");
  }

  flags |= O_NONBLOCK;
  
  if (fcntl(s, F_SETFL, flags) < 0){
    perror("fcntl F_SETFL");
  }
}

/* get_sndbuf receives a socket as parameter and returns the
 * the send buffer of that socket
 */
int get_skt_sndbuf(const int socket)
{
  static int sndbuf, size;
  size = sizeof(int);
  
  if (getsockopt(socket, SOL_SOCKET, SO_SNDBUF, (void *) &sndbuf,
		 (socklen_t *) &size) < 0){
    perror("getsockopt SO_SNDBUF");
    return 0;
  }
  
  return sndbuf;
}


/* get_rcvbuf receives a socket as parameter and returns the
 * the receive buffer of that socket
 */
int get_skt_rcvbuf(const int socket)
{
  static int rcvbuf, size;
  size = sizeof(int);
  
  if (getsockopt(socket, SOL_SOCKET, SO_RCVBUF, (void *) &rcvbuf, 
		 (socklen_t *) &size) < 0){
    perror("getsockopt SO_RCVBUF");
    return 0;
  }
  
  return rcvbuf;
}

void set_skt_sndbuf(const int socket, const int sndbuf)
{
  if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (void *) &sndbuf, sizeof(sndbuf)) < 0){
    perror("setsockopt SO_SNDBUF");
  }
}


void set_skt_rcvbuf(const int socket, const int rcvbuf)
{
  if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (void *) &rcvbuf, sizeof(rcvbuf)) < 0){
    perror("setsockopt SO_RCVBUF");
  }
}

void set_skt_broadcast(const int s)
{
  static int on = 1;

  if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0){
    perror("setsockopt SO_BROADCAST");
  }    
}

/* Tells the socket that the IP header of the packet is include in the
 * data passed as parameter.
 */
void set_iphdr_included(const int s)
{
  static int on = 1;
  
  if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
    perror("setsockopt IP_HDRINCL");
  }
}


void set_skt_reuseaddr(const int s)
{
  static int on = 1;

  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
    perror("setsockopt SO_REUSEADDR");
  }    
}

int new_socket()
{
  static int s;
  
  if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("socket");
    exit(EXIT_FAILURE);
  }
  
  return s;
}
