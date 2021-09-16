#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "configuration.h"

#define MSG_PATH "/tmp/dhcp2nidd"
#define MAXBUFLEN 128

void *thread_dhcp(void)
{
  int sockfd, newsockfd, servlen, clilen;
  struct sockaddr_un  cli_addr, serv_addr;
  char buf[MAXBUFLEN];
  char aux[INET6_ADDRSTRLEN];
  struct timespec tv;
  char *dhcp_response, *token;
  NID nid;

  if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0){
    perror("unix socket");
    exit(-1);
  }
   
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, MSG_PATH);
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

  unlink(MSG_PATH);

  if (bind(sockfd,(struct sockaddr *)&serv_addr,servlen) < 0){
    perror("binding socket");
    exit(-1);
  }
   
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);
  
  memset(aux, 0, MAXBUFLEN);
  memset(&tv, 0, sizeof(tv));
  tv.tv_nsec = 100000000;

  while (1){
    memset(buf, 0, MAXBUFLEN);
    
    if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, 
			    (socklen_t *)&clilen)) < 0){
      perror("accepting");
      exit(-1);
    }
    
    if (read(newsockfd, buf, MAXBUFLEN) < 0){
      perror("(DHCP) read");
      exit(-1);
    }
    
    if (buf){
    
      dhcp_response = strdup(buf);
      token = strtok(dhcp_response, "|");
      token = strtok(NULL, "|");    
      nid = getNIDgw();
      inet_ntop(AF_INET6, &nid, aux, INET6_ADDRSTRLEN);    
      set_mobility(strcmp(token, aux) ? INTERDOMAIN : INTRADOMAIN);
      
      set_conf_dhcp(buf);
      free(dhcp_response);
      print_dhcp();
      /*sleep for 100 ms*/
      //nanosleep(&tv, NULL);

      close(newsockfd);
    }
    else {
      printf("Warning: No parameters returned from the DHCP Server!!!\n");
    }
  }
}

