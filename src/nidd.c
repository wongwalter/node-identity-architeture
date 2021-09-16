#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/netfilter.h>
#include <libipq/libipq.h>
#include <sys/un.h>
#include <pthread.h>
#include <signal.h>
#include <dlfcn.h>
#include <arpa/inet.h>

#include "services.h"
#include "connection_mapper.h"
#include "nid_mapper.h"
#include "configuration.h"
#include "nid_routing.h"
#include "tools.h"
#include "rvs_queue_table.h"
#include "dht_queue_table.h"
#include "gwc.h"
#include "dhtc.h"

//Rodolfo
#include "fqdn_table.h"
#include "dnshandler.h"

int tap;
int lib_counter = 0;
void *lib_db[12];

extern int sp_nf2ph[2];
extern int sp_ph2nr[2];
extern int sp_nr2ph[2];
extern int sp_nr[2];
extern int sp_mo2ph[2];
extern int sp_cm2rvs[2];
extern int sp_gwt2ph[2];
extern int sp_co2gwl[2];
extern int sp_ph2co[2];
extern int sp_rvs2ph[2];
extern int sp_co2rvs[2];
extern int sp_rvs2gwt[2];
extern int sp_gwl2gwt[2];
extern int sp_dht[2];

pthread_t dhcp_thr;             /* NID DHCP Client              */
pthread_t nidfilter_thr;        /* NID Filter thread            */
pthread_t ph_output_thr;        /* Packet Handler Output thread */
pthread_t ph_input_thr;         /* Packet Handler Input thread  */  
pthread_t nr_output_thr;        /* NID Routing Output thread    */
pthread_t nr_input_thr;         /* NID Routing Input thread     */
pthread_t nr_debug_thr;         /* NID Routing Debug thread     */
pthread_t rvs_thr;              
pthread_t update_conn_thr;     
pthread_t mobility_thr;    
pthread_t control_thr;     
pthread_t rvs_proxy_send_thr;
pthread_t rvs_proxy_recv_thr;
pthread_t rvs2gw_thr;
pthread_t dht_up_thr;
pthread_t dht_get_thr;
pthread_t dhcp_thr;

//Rodolfo
pthread_t dnshandler_thr;

static void close_lib(char *lib);

void catch_int(int int_num)
{
  int i;
  
  /* Close dynamic libraries */
  for (i = 0; i < 12; i++){
    if (lib_db[i] != NULL){
      close_lib(lib_db[i]);
    }
  }  
  
  //signal(SIGINT, catch_int);
  printf("Closing tap device.....\n");
  close(tap);
  destroy_nid_mapper();
  destroy_conn_mapper();
  destroy_rtable();
  destroy_rvs_queue_table();
  destroy_dht_queue_table();
  destroy_dhtable();
  destroy_registry_table();
  
  fflush(stdout);
  if (get_dhcp()){
    system("pkill -9 dhcpcd");
  }
  exit(EXIT_SUCCESS);
}

static void (*open_lib(char *lib, char *func))()
{
  void *lib_handle;
  void (*lib_pt)();
  char *error;
  
  if (!(lib_handle = dlopen(lib, RTLD_LAZY))){
    fprintf(stderr, "dlopen: %s\n", dlerror());
    exit(EXIT_FAILURE);
  }
  
  lib_db[lib_counter++] = lib_handle;
  
  lib_pt = dlsym(lib_handle, func);
  error = dlerror();
  if (error){
    fprintf(stderr, "dlsym: %s\n", error);
    exit(EXIT_FAILURE);
  }
  
  return lib_pt;
}

static void close_lib(char *lib){
  dlclose(lib);
}

int main(int argc, char *argv[])
{
  void (*lib_pt)();
  int i;

  for (i = 0; i < 10; i++)
    lib_db[i] = NULL;

  read_config(CONFIG_FILE);  
  print_conf();
  
  signal(SIGINT, catch_int);
  init_nid_mapper();
  init_conn_mapper();
  init_rtable();
  init_rvs_queue_table();
  init_dht_queue_table();
  init_dhtable();
  init_registry_table();
    
  NID nid = getNIDrvs();
  NID32 rvsIP = getRvsIP();
  NID nidgw = getNIDgw();  
  
  tap = init_tap(DEVICE, getNID(), getNID32());
  
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_nf2ph);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_ph2nr);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_nr2ph);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_nr);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_mo2ph);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_ph2co);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_gwt2ph);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_co2gwl);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_rvs2ph);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_co2rvs);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_cm2rvs);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_rvs2gwt);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_gwl2gwt);
  socketpair(AF_UNIX, SOCK_DGRAM, 0, sp_dht);

  printf("------- Starting NID daemon services --------\n");

  i = 0;

  switch(getType()){
  case NODE:
    
    newNMEntry(nid, getNIDcore());
    staticCMEntry(nid, rvsIP);
    staticCMEntry(nidgw, getGatewayIP());

    lib_pt = open_lib("libdhcp.so", "thread_dhcp");
    printf("Starting NID DHCP Client..........\t\t");
    if (pthread_create(&dhcp_thr, NULL, (void *)lib_pt, NULL) < 0){
      perror("dhcp thread");
      exit(EXIT_FAILURE);
    }
    
    if (get_dhcp()){
      char aux[32];
      sprintf(aux, "rm -fr /etc/dhcpc/* && ./dhcpcd -YR %s", getPrimaryIface());
      system(aux);
    }

    lib_pt = open_lib("libnf.so", "thread_nidfilter");
    printf("ok!\nStarting NID Filter Thread....\t\t\t");
    if (pthread_create(&nidfilter_thr, NULL, (void *)lib_pt, NULL) < 0){
      perror("nidfilter thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libph.so", "thread_ph_output");
    printf("ok!\nStarting Packet Handler Output Thread....\t");
    if (pthread_create(&ph_output_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("packet handler output thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libph.so", "thread_ph_input");
    printf("ok!\nStarting Packet Handler Input Thread....\t");
    if (pthread_create(&ph_input_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("packet handler input thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libnr.so", "thread_nr_output");
    printf("ok!\nStarting NID Routing Output Thread ....\t\t");
    if (pthread_create(&nr_output_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing output thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libnr.so", "thread_nr_input");
    printf("ok!\nStarting NID Routing Input Thread....\t\t");
    if (pthread_create(&nr_input_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("librvs.so", "thread_rvs_update");
    printf("ok!\nStarting RVS Update Thread....\t\t\t");
    if (pthread_create(&rvs_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("rvs thread");
      exit(EXIT_FAILURE);
    }    
    
    lib_pt = open_lib("librvs.so", "thread_rvs_get");
    printf("ok!\nStarting Proactive RVS Resolution Thread....\t");
    if (pthread_create(&update_conn_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("rvs get thread");
      exit(EXIT_FAILURE);
    }    
    
    lib_pt = open_lib("libmo.so", "thread_mobility");
    printf("ok!\nStarting Mobility Thread....\t\t\t");
    if (pthread_create(&mobility_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("mobility thread");
      exit(EXIT_FAILURE);
    }    
    
    lib_pt = open_lib("libco.so", "thread_control");
    printf("ok!\nStarting Control Thread....\t\t\t");
    if (pthread_create(&control_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("control thread");
      exit(EXIT_FAILURE);
    }    

    //Rodolfo
    if (get_dns()){
      lib_pt = open_lib("libdnsh.so", "thread_dnshandler");
      printf("ok!\nStarting DNS Handler Thread ....\t\t");
      if (pthread_create(&dnshandler_thr, NULL, (void *) lib_pt, NULL) < 0){
	perror("dns handler thread");
	exit(0);
      }
    }

    printf("ok!\nAll services started!\n");
    pthread_join(nidfilter_thr, NULL);
    break;    
  case RVS:

    newNMEntry(nid, getNIDcore());
    staticCMEntry(nid, rvsIP);
    staticCMEntry(nidgw, getGatewayIP());
    
    lib_pt = open_lib("libnf.so", "thread_nidfilter");
    printf("Starting NID DHCP Client..........\t\t");
    if (pthread_create(&nidfilter_thr, NULL, (void *)lib_pt, NULL) < 0){
      perror("nidfilter thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libph.so", "thread_ph_output");
    printf("ok!\nStarting Packet Handler Output Thread....\t");
    if (pthread_create(&ph_output_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("packet handler output thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libph.so", "thread_ph_input");
    printf("ok!\nStarting Packet Handler Input Thread....\t");
    if (pthread_create(&ph_input_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("packet handler input thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libnr.so", "thread_nr_output");
    printf("ok!\nStarting NID Routing Output Thread ....\t\t");
    if (pthread_create(&nr_output_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing output thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libnr.so", "thread_nr_input");
    printf("ok!\nStarting NID Routing Input Thread....\t\t");
    if (pthread_create(&nr_input_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("librvs.so", "thread_rvs_proxy_recv");
    printf("ok!\nStarting RVS Proxy Recv Thread....\t\t");
    if (pthread_create(&rvs_proxy_recv_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("rvs proxy recv thread");
      exit(EXIT_FAILURE);
    }

    lib_pt = open_lib("librvs.so", "thread_rvs_proxy_send");
    printf("ok!\nStarting RVS Proxy Send Thread....\t\t");
    if (pthread_create(&rvs_proxy_send_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("rvs proxy send thread");
      exit(EXIT_FAILURE);
    }

    lib_pt = open_lib("libco.so", "thread_control_rvs");
    printf("ok!\nStarting RVS Control Thread....\t\t\t");
    if (pthread_create(&control_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("control thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("librvs.so", "thread_rvs_update");
    printf("ok!\nStarting RVS Update Thread....\t\t\t");
    if (pthread_create(&rvs_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("rvs thread");
      exit(EXIT_FAILURE);
    }

    lib_pt = open_lib("librvs.so", "thread_rvs2gw");
    printf("ok!\nStarting RVS Message Propagation Thread....\t");
    if (pthread_create(&rvs2gw_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("rvs2gw thread");
      exit(EXIT_FAILURE);
    }

    if (get_dns()){
      //Rodolfo
      lib_pt = open_lib("libdnsh.so", "thread_dnshandler");
      printf("ok!\nStarting DNS Handler Thread ....\t\t");
      if (pthread_create(&dnshandler_thr, NULL, (void *) lib_pt, NULL) < 0){
	perror("dns handler thread");
	exit(0);
      }
    }

    printf("ok!\nAll services started!\n");
    pthread_join(ph_output_thr, NULL);    
    break;
  case GATEWAY:

    newNMEntry(nid, getNIDcore());
    staticCMEntry(nid, rvsIP);
    staticCMEntry(nidgw, getGatewayIP());
    staticRTEntry(nid, rvsIP);
    staticRTEntry(nidgw, getGatewayIP());

    lib_pt = open_lib("libdhcp.so", "thread_dhcp");
    printf("Starting NID DHCP Client..........\t\t");
    if (pthread_create(&dhcp_thr, NULL, (void *)lib_pt, NULL) < 0){
      perror("dhcp thread");
      exit(EXIT_FAILURE);
    }
    
    if (get_dhcp()){
      char aux[32];
      sprintf(aux, "rm -fr /etc/dhcpc/* && ./dhcpcd -YR %s", getPrimaryIface());
      system(aux);
    }
    
    lib_pt = open_lib("libnf.so", "thread_nidfilter");
    printf("ok!\nStarting NID DHCP Client..........\t\t");
    if (pthread_create(&nidfilter_thr, NULL, (void *)lib_pt, NULL) < 0){
      perror("nidfilter thread");
      exit(EXIT_FAILURE);
    }

    lib_pt = open_lib("libph.so", "thread_ph_output");
    printf("ok!\nStarting Packet Handler Output Thread....\t");
    if (pthread_create(&ph_output_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("packet handler output thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libph.so", "thread_ph_input");
    printf("ok!\nStarting Packet Handler Input Thread....\t");
    if (pthread_create(&ph_input_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("packet handler input thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libnr.so", "thread_nr_output_gw");
    printf("ok!\nStarting NID Routing Output Thread ....\t\t");
    if (pthread_create(&nr_output_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing output thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libnr.so", "thread_nr_input_gw");
    printf("ok!\nStarting NID Routing Input Thread....\t\t");
    if (pthread_create(&nr_input_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing input thread");
      exit(EXIT_FAILURE);
    }

#ifdef DEBUG_PRINT_RT

    lib_pt = open_lib("libnr.so", "thread_debug_rt");
    printf("ok!\nStarting NID Routing Debug Thread....\t\t");
    if (pthread_create(&nr_debug_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing debug thread");
      exit(EXIT_FAILURE);
    }
    
#endif

    lib_pt = open_lib("libmo.so", "thread_mobility");
    printf("ok!\nStarting Mobility Thread....\t\t\t");
    if (pthread_create(&mobility_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("mobility thread");
      exit(EXIT_FAILURE);
    }

    lib_pt = open_lib("librvs.so", "thread_rvs_update");
    printf("ok!\nStarting RVS Thread....\t\t\t\t");
    if (pthread_create(&rvs_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("rvs thread");
      exit(EXIT_FAILURE);
    }

    lib_pt = open_lib("libco.so", "thread_control_gw");
    printf("ok!\nStarting Gateway Control Thread....\t\t");
    if (pthread_create(&control_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("control thread");
      exit(EXIT_FAILURE);
    }

    lib_pt = open_lib("libgw.so", "thread_gw_talker");
    printf("ok!\nStarting Gateway Talker Thread....\t\t");
    if (pthread_create(&control_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("gw talker thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libgw.so", "thread_gw_listener");
    printf("ok!\nStarting Gateway Listener Thread....\t\t");
    if (pthread_create(&control_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("gw listener thread");
      exit(EXIT_FAILURE);
    }

    if (get_dns()){
      //Rodolfo
      lib_pt = open_lib("libdnsh.so", "thread_dnshandler");
      printf("ok!\nStarting DNS Handler Thread ....\t\t");
      if (pthread_create(&dnshandler_thr, NULL, (void *) lib_pt, NULL) < 0){
	perror("dns handler thread");
	exit(0);
      }
    }

    printf("ok!\nAll services started!\n");    
    pthread_join(nidfilter_thr, NULL);
    break;
  case CORE:

    newNMEntry(nid, getNIDcore());

    lib_pt = open_lib("libnf.so", "thread_nidfilter");
    printf("Starting NID DHCP Client..........\t\t");
    if (pthread_create(&nidfilter_thr, NULL, (void *)lib_pt, NULL) < 0){
      perror("nidfilter thread");
      exit(EXIT_FAILURE);
    }

    lib_pt = open_lib("libph.so", "thread_ph_output");
    printf("ok!\nStarting Packet Handler Output Thread....\t");
    if (pthread_create(&ph_output_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("packet handler output thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libph.so", "thread_ph_input");
    printf("ok!\nStarting Packet Handler Input Thread....\t");
    if (pthread_create(&ph_input_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("packet handler input thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libnr.so", "thread_nr_output_core");
    printf("ok!\nStarting NID Routing Output Thread ....\t\t");
    if (pthread_create(&nr_output_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing output thread");
      exit(EXIT_FAILURE);
    }
    
    lib_pt = open_lib("libnr.so", "thread_nr_input_gw");
    printf("ok!\nStarting NID Routing Input Thread....\t\t");
    if (pthread_create(&nr_input_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing input thread");
      exit(EXIT_FAILURE);
    }

#ifdef DEBUG_PRINT_RT

    lib_pt = open_lib("libnr.so", "thread_debug_rt");
    printf("ok!\nStarting NID Routing Debug Thread....\t\t");
    if (pthread_create(&nr_debug_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("nid routing debug thread");
      exit(EXIT_FAILURE);
    }
    
#endif

    lib_pt = open_lib("libco.so", "thread_control_gw_core");
    printf("ok!\nStarting Gateway Control Thread....\t\t");
    if (pthread_create(&control_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("control thread");
      exit(EXIT_FAILURE);
    }

    lib_pt = open_lib("libgw.so", "thread_gw_core_listener");
    printf("ok!\nStarting Gateway Core Listener Thread....\t");
    if (pthread_create(&control_thr, NULL, (void *) lib_pt, NULL) < 0){
      perror("gw core listener thread");
      exit(EXIT_FAILURE);
    }

    if (get_dht()){
    
      lib_pt = open_lib("libdht.so", "thread_dht_update");
      printf("ok!\nStarting DHT Update Thread....\t\t");
      if (pthread_create(&dht_up_thr, NULL, (void *) lib_pt, NULL) < 0){
	perror("dht update thread");
	exit(EXIT_FAILURE);
      }
      
      lib_pt = open_lib("libdht.so", "thread_dht_get");
      printf("ok!\nStarting DHT Resolver Thread....\t\t");
      if (pthread_create(&dht_get_thr, NULL, (void *) lib_pt, NULL) < 0){
	perror("dht resolver thread");
	exit(EXIT_FAILURE);
      }
    }

    if (get_dns()){
      //Rodolfo
      lib_pt = open_lib("libdnsh.so", "thread_dnshandler");
      printf("ok!\nStarting DNS Handler Thread ....\t\t");
      if (pthread_create(&dnshandler_thr, NULL, (void *) lib_pt, NULL) < 0){
	perror("dns handler thread");
	exit(0);
      }
    }

    printf("ok!\nAll services started!\n");
    pthread_join(nidfilter_thr, NULL);
    break;
  }
  
  return EXIT_SUCCESS;
}
