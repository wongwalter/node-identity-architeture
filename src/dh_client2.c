#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/dh.h>
#include <openssl/engine.h>
#include <openssl/bio.h>
#include <openssl/bn.h>

#include "dh.h"

#define PORT 20001
#define MAXBUFLEN 1500

int main(int argc, char *argv[])
{
  int sockfd, numbytes, i, ret, size;
  struct sockaddr_in peer_addr; 
  char buf[MAXBUFLEN], tmp[PRIME_LEN + 1];
  DH *server, *client;
  u_char *ubuf;
  dh_struct dh_msg, *dh_pt;
  
  BIO *out;
  out = BIO_new(BIO_s_file());
  BIO_set_fp(out, stdout, BIO_NOCLOSE);

  if (argc != 2) {
    fprintf(stderr,"usage: %s <peer-addr>\n", argv[0]);
    exit(1);
  }
  
  if ((client = DH_generate_parameters(4 * PRIME_LEN, DH_GENERATOR_5, NULL, NULL)) == NULL){
    perror("DH_generate_parameters");
    exit(0);
  }
  
  if (!DH_check(client, &i) < 0){
    perror("DH_check");
    exit(0);
  }
  
  switch(i){
  case DH_CHECK_P_NOT_PRIME:
    printf("p value is not prime\n");
    break;
  case DH_CHECK_P_NOT_SAFE_PRIME:
    printf("p value is not a safe prime\n");
    break;
  case DH_UNABLE_TO_CHECK_GENERATOR:
    printf("unable to check the generator value\n");
    break;    
  case DH_NOT_SUITABLE_GENERATOR:
    printf("the g value is not a generator\n");
    break;
  }
  
  BIO_puts(out, "\nclient->p = ");
  BN_print(out, client->p);  
  BIO_puts(out, "\nclient->g = ");
  BN_print(out, client->g);
  BIO_puts(out, "\n");
  
  if ((numbytes = DH_generate_key(client)) < 0){
    perror("DH_generate_key");
    exit(0);
  }
  
  BIO_puts(out,"client->priv_key= ");
  BN_print(out, client->priv_key);
  BIO_puts(out,"\nclient->pub_key = ");
  BN_print(out, client->pub_key);
  BIO_puts(out,"\n");

  bzero(&dh_msg, sizeof(dh_msg));
  
  strcpy(dh_msg.p, BN_bn2hex(client->p));
  strcpy(dh_msg.g, BN_bn2hex(client->g));  
  strcpy(dh_msg.pub_key, BN_bn2hex(client->pub_key));

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }
  
  peer_addr.sin_family = AF_INET;
  peer_addr.sin_port = htons(PORT);
  peer_addr.sin_addr.s_addr = inet_addr(argv[1]);
  bzero(&(peer_addr.sin_zero), 8);
  
  if (connect(sockfd, (struct sockaddr *)&peer_addr, sizeof(struct sockaddr)) == -1) {
    perror("connect");
    exit(1);
  }

  if ((numbytes = send(sockfd, &dh_msg, sizeof(dh_struct), 0)) < 0){
    perror("send");
    exit(0);
  }  

  printf("sent %d bytes\n\n", numbytes);

  bzero(buf, MAXBUFLEN);
  
  if ((numbytes = recv(sockfd, buf, MAXBUFLEN - 1, 0)) < 0) {
    perror("recv");
    exit(1);
  }

  printf("received %d bytes\n", numbytes);
  
  dh_pt = (dh_struct *) buf;
  
  strncpy(tmp, dh_pt->p, PRIME_LEN);
  tmp[PRIME_LEN] = '\0';
  
  server = DH_new();  

  BN_hex2bn(&server->p, tmp);
  BN_hex2bn(&server->g, dh_pt->g);
  strncpy(tmp, dh_pt->pub_key, PRIME_LEN);
  BN_hex2bn(&server->pub_key, tmp);
  
  printf("server->p = ");
  BN_print(out, server->p);
  printf("\nserver->g = ");
  BN_print(out, server->g);
  printf("\nserver->pub_key = ");
  BN_print(out, server->pub_key);
  printf("\n");

  size = DH_size(client);
  ubuf = (u_char *) OPENSSL_malloc(size);
  
  if ((ret = DH_compute_key(ubuf, server->pub_key, client)) < 0){
    perror("DH_compute_key");
    exit(0);
  }
  
  printf("\nGenerated shared-key: ");
  
  for (i = 0; i < ret; i++){
    sprintf(tmp, "%02X", ubuf[i]);
    BIO_puts(out, tmp);
  }
  
  printf("\n");
    
  close(sockfd);
  
  return 0;
}
