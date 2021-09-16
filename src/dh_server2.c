#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/dh.h>
#include <openssl/engine.h>
#include <openssl/bio.h>
#include <openssl/bn.h>

#include "dh.h"

#define PORT 20001
#define MAXBUFLEN 1500

int main(int argc, char *argv[])
{
  int sockfd, peerfd;
  struct sockaddr_in local_addr, peer_addr; 
  int addr_len, numbytes, i, ret, size;
  u_char ubuf[MAXBUFLEN], *ubuf2, tmp[33];
  DH *server, *client;
  BIO *out;
  dh_struct dh_msg, *dh_pt;
  
  out = BIO_new(BIO_s_file());
  BIO_set_fp(out, stdout, BIO_NOCLOSE);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }
  
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(PORT);
  local_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(local_addr.sin_zero), 8);

  if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1) {
    perror("bind");
    exit(1);
  }
  
  addr_len = sizeof(struct sockaddr);
  
  if (listen(sockfd, 10) == -1) {
    perror("listen");
    exit(1);
  }

  printf("Listening on port %d\n", PORT);    
  
  while(1){
    
    if ((peerfd = accept(sockfd, (struct sockaddr *)&peer_addr, (socklen_t *)&addr_len)) == -1) {
      perror("accept");
      exit(0);
    }
    
    printf("Got connection from %s\n", inet_ntoa(peer_addr.sin_addr));
    
    if (!fork()){   
      close(sockfd);
      
      if ((numbytes = recv(peerfd, ubuf, MAXBUFLEN, 0)) < 0){
	perror("send");
	exit(0);
      }
      
      printf("recv %d bytes\n", numbytes);
      
      // Diffie-Hellman
      
      dh_pt = (dh_struct *) ubuf;
       
      strncpy(tmp, dh_pt->p, PRIME_LEN);
      tmp[PRIME_LEN] = '\0';
      
      client = DH_new();  
       
      BN_hex2bn(&client->p, tmp);
      BN_hex2bn(&client->g, dh_pt->g);
      
      strncpy(tmp, dh_pt->pub_key, PRIME_LEN);
      BN_hex2bn(&client->pub_key, tmp);
      
      printf("client->p = ");
      BN_print(out, client->p);
      printf("\nclient->g = ");
      BN_print(out, client->g);
      printf("\nclient->pub_key = ");
      BN_print(out, client->pub_key);
      printf("\n\n");
      
      if ((server = DH_new()) == NULL){
	perror("DH_new");
	exit(0);
      }
       
      server->p = BN_dup(client->p);
      server->g = BN_dup(client->g);
      
      printf("server->p = ");
      BN_print(out, server->p);
      printf("\nserver->g = ");
      BN_print(out, server->g);
      printf("\n");
       
      if ((server->p == NULL) || (server->g == NULL))
	exit(0);
      
      if ((ret = DH_generate_key(server)) < 0){
	perror("DH_generate_key");
	exit(0);
      }
      
      BIO_puts(out,"server->priv_key = ");
      BN_print(out, server->priv_key);
      BIO_puts(out,"\nserver->pub_key = ");
      BN_print(out, server->pub_key);
      BIO_puts(out,"\n");
       
      size = DH_size(server);
      ubuf2 = (u_char *) OPENSSL_malloc(size);
      
      if ((ret = DH_compute_key(ubuf2, client->pub_key, server)) < 0){
	perror("DH_compute_key");
	exit(0);
      }
      
      printf("\nGenerated shared-key: ");
      
      for (i = 0; i < ret; i++){
	sprintf(tmp, "%02X", ubuf2[i]);
	BIO_puts(out, tmp);
      }
      
      bzero(&dh_msg, sizeof(dh_msg));
  
      strcpy(dh_msg.p, BN_bn2hex(server->p));
      strcpy(dh_msg.g, BN_bn2hex(server->g));  
      strcpy(dh_msg.pub_key, BN_bn2hex(server->pub_key));

      // Diffie-Hellman
      
      if ((numbytes = send(peerfd, &dh_msg, sizeof(dh_struct), 0)) < 0){
	perror("send");
	exit(0);
      }
      
      printf("\n\nsent %d bytes\n", numbytes);

      close(peerfd);
      exit(0);
      }
    
    close(peerfd);    
  }
  
  return 0;
}
