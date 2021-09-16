#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "rpcif.h"
#include "nid.h"
#include "configuration.h"
#include "hash.h"
#include "dhtc.h"
#include "dht_queue_table.h"

#define APP_STRING "OpenDHT HIP Interface $Revision: 1.2 $"
#define CLIB_STRING "rpcgen"

extern int sp_dht[2];
hash_table dhtable;

static unsigned hash_nid(const char *string)
{
  unsigned ret_val = 0;
  int i;
  
  while (*string){
    i = (int) *string;
    ret_val ^= i;
    ret_val <<= 1;
    string ++;
  }
  
  return ret_val;
}

static void do_null_call (CLIENT *clnt)
{
  char *null_args 			= NULL; 
  void *null_result			= bamboo_dht_proc_null_2((void*)&null_args, clnt);
  
  if (null_result == (void *) NULL) {
    clnt_perror (clnt, "null call failed.");
    exit (1);
  }
}

  
static CLIENT* connectDHTserver(char *host, int port)
{
  CLIENT *clnt;
  struct sockaddr_in *addr		= malloc(sizeof(struct sockaddr_in));
  struct hostent *h			= NULL;
  int sockp 				= RPC_ANYSOCK; 
  
  printf("Connecting to %s port %d ...\n",host, port);
  //Lookup server
  h = gethostbyname (host); 
  if (h == NULL) {
    printf("Could not resolve %s\n",host);
    exit(1);
  }
  
  //Create sockaddr_in
  bzero (addr, sizeof (struct sockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_port = htons (port);
  addr->sin_addr = *((struct in_addr *) h->h_addr);
  
  //Connect
  if ((clnt = clnttcp_create (addr, BAMBOO_DHT_GATEWAY_PROGRAM, BAMBOO_DHT_GATEWAY_VERSION, &sockp, 0, 0)) == NULL){  
    clnt_pcreateerror ("Connect:");    
  }
  else {
    do_null_call(clnt);
  }
  
  free(addr);
  return clnt;
}


static bamboo_get_res* opendhtget(CLIENT* clnt, bamboo_get_args *get_args, int maxvals)
{
  bamboo_get_res *get_result			= NULL;
  get_args->application = APP_STRING;
  get_args->client_library = CLIB_STRING;
  get_args->maxvals = maxvals;
  
  get_result = bamboo_dht_proc_get_2 (get_args, clnt);
  if (get_result == (bamboo_get_res *) NULL) {
    clnt_perror (clnt, "get failed");    
  }
  
  return get_result;
}

int opendhtput(CLIENT* clnt, char* key, char* value, int ttl)
{   
  bamboo_put_args put_args;
  bamboo_stat *put_result			= NULL;
  
  printf ("Putting key: %s val: %s ttl: %d\n", key, value, ttl);

  memset (&put_args, 0, sizeof (put_args));
  sprintf(put_args.key, "%d", hash_nid(key));
  put_args.value.bamboo_value_val = value;
  put_args.value.bamboo_value_len = strlen(value);
  put_args.application = APP_STRING;
  put_args.client_library = CLIB_STRING;
  put_args.ttl_sec = ttl;

  put_result = bamboo_dht_proc_put_2 (&put_args, clnt);
  
  if (put_result == (bamboo_stat *) NULL) {
    clnt_perror (clnt, "put failed");
  }
  
  return 0;
}


void (*thread_dht_update(void))(void)
{
  CLIENT *clnt;
  char *nid				= nid2char(getNID());
  char *coreIP 				= get_locator(getPrimaryIface());
  int *retval_pthread;
  
  while(1){
  
    clnt = connectDHTserver((char *)nid322char(getDhtIP()), getDhtPort());
    
    if (clnt == NULL){
      printf("Couldn't connect to DHT ... shutting down DHT Update Thread!\n");
      pthread_exit(retval_pthread);
    }
    
    opendhtput(clnt, nid, coreIP, (int)DHT_TTL);
    free(clnt);
    sleep(DHT_UPDATE);    
  } 
}

void (*thread_dht_get(void))(void)
{
  CLIENT *clnt					= NULL;
  bamboo_get_res *get_result			= NULL;
  int nid_size 					= sizeof(NID);
  int maxfd 					= sp_dht[0] + 1;   
  char aux[INET6_ADDRSTRLEN];
  NID32 nid32;
  NID nid;
  bamboo_get_args get_args;
  fd_set readfds;
  int *retval_pthread;

  
  init_dhtable();  
  FD_ZERO(&readfds);
  
  while (1){
    
    FD_SET(sp_dht[0], &readfds);    
    select(maxfd, &readfds, NULL, NULL, NULL);
    
    if (FD_ISSET(sp_dht[0], &readfds)){
      
      read(sp_dht[0], &nid, nid_size);
      clnt = connectDHTserver((char *)nid322char(getDhtIP()), getDhtPort());

      if (clnt == NULL){
	printf("Couldn't connect to DHT ... shutting down DHT Get Thread!\n");
	pthread_exit(retval_pthread);
      }
      
      bzero(&get_args, sizeof(get_args));
      inet_ntop(AF_INET6, &nid, aux, INET6_ADDRSTRLEN);
      sprintf(get_args.key, "%d", hash_nid(aux));
      get_result = opendhtget(clnt, &get_args, 10);

      if (get_result->values.values_len == 0) {
	inet_ntop(AF_INET6, &nid, aux, INET6_ADDRSTRLEN);
	printf("Get NID = %s failed: returned %d values.\n", 
	       aux, get_result->values.values_len);
      }
      else {
	strncpy(aux, get_result->values.values_val[0].bamboo_value_val, 
		get_result->values.values_val[0].bamboo_value_len);
	inet_pton(AF_INET, aux, &nid32);
	dht_unqueue(nid);
	addDHTEntry(nid, nid32);
      }
      
      free(clnt);
    }
  }
}


/* DHT table functions */

void init_dhtable(){
  construct_hash_table(&dhtable, 30);  
}

void destroy_dhtable(){
  free_hash_table(&dhtable, NULL);
  printf("Destroying DHT Mapping table...\n");
}

void addDHTEntry(const NID nid, const NID32 ip)
{
  static char aux[INET6_ADDRSTRLEN];
  conn_entry *ce = (conn_entry *) calloc(1, sizeof(conn_entry));
  
  ce->ip = ip;
  ce->tstamp = time(NULL);

  inet_ntop(AF_INET6, &nid, aux, INET6_ADDRSTRLEN);
  printf("addDHTEntry NID=%s ", aux);
  inet_ntop(AF_INET, &ip, aux, INET_ADDRSTRLEN);
  printf("IP=%s\n", aux);
  hash_insert(nid, ce, &dhtable);
  fflush(stdout);
}

int getDHTEntry(const NID nid, NID32 *dst)
{
  conn_entry *ce = (conn_entry *) hash_lookup(nid, &dhtable);
  
  if (ce){
    if (ce->ip != 0){
      //printf("getDHTEntry NID=%s IP=%s\n", nid2char(nid), nid322char(ce->ip));
      *dst = ce->ip;
      return 1;
    }
  }

  if (!is_dht_queued(nid)){
    dht_queue(nid);
    write(sp_dht[1], &nid, 16);
  }
  
  return 0;
}


