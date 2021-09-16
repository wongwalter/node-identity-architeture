#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

#include "nid.h"
#include "configuration.h"
#include "connection_mapper.h"
#include "tools.h"
#include "packetHandler.h"
#include "dnshandler.h"

static unsigned short int type				= 0;
static unsigned short int gatewayPort			= 0;
static unsigned short int dhtPort			= 0;
static unsigned short int rvsTimeout 			= 0;
static unsigned short int dhcp				= 0;
static unsigned short int dht				= 0;
static unsigned short int dns				= 0;
/* 0 - intradomain mobility; 1 - interdomain mobility */
static unsigned short int mobility			= 0;

static char primaryIface[32];

static NID nid;
static NID nidgw;
static NID nidrvs;
static NID nidcore;
static NID nidcorehome;
static NID niddns;
static NID niddht;

static NID32 nid32 					= 0;
static NID32 gatewayIP  				= 0;
static NID32 rvsIP					= 0;
static NID32 dhtIP					= 0;
static NID32 ip;

/* Permanent configuration */

unsigned short int getType(){
  return type;
}

NID getNID(){
  return nid;
}

unsigned short int get_dhcp(){
  return dhcp;
}

unsigned short int get_dht(){
  return dht;
}

unsigned short int get_dns(){
  return dns;
}

/* End of permanent configuration */

char *getPrimaryIface(){
  return primaryIface;
}

unsigned short int getGatewayPort(){
  return gatewayPort;
}

unsigned short int getRvsTimeout(){
  return rvsTimeout;
}

unsigned short int getDhtPort(){
  return dhtPort;
}

unsigned short int get_mobility(){
  return mobility;
}

NID getNIDgw(){
  return nidgw;
}

NID getNIDrvs(){
  return nidrvs;
}

NID getNIDcore(){
  return nidcore;
}

NID getNIDcorehome(){
  return nidcorehome;
}

NID getNIDdht(){
  return niddht;
}

//Rodolfo
NID getNIDdns(){
  return niddns;
}

NID32 getGatewayIP(){
  return gatewayIP;
}

NID32 getNID32(){
  return nid32;
}

NID32 getRvsIP(){
  return rvsIP;
}

NID32 getDhtIP(){
  return dhtIP;
}

NID32 getIP(){
  return ip;
}

void setGatewayIP(const NID32 newIP){
  gatewayIP = newIP;
}

void setGatewayPort(const unsigned short int port){
  gatewayPort = port;
}

void setRvsIP(const NID32 newIP){
  rvsIP = newIP;
}

void setDhtIP(const NID32 newIP){
  dhtIP = newIP;
}

void setIP(const NID32 newIP){
  ip = newIP;
}

void set_mobility(const unsigned short int new){
  mobility = new;
}

static void setNID32(){
  
  static u_int8_t *pt = (u_int8_t *) &nid + 12;
  static u_int8_t *pt32 = (u_int8_t *) &nid32;      
  
  memcpy(pt32, pt, 4);
  *pt32 = 1;  
}

static void get_data(xmlNode *node)
{
  xmlNode *pt = NULL;
  xmlChar *data = NULL;
  
  for (pt = node; pt; pt = pt->next) {
    if (pt->type == XML_ELEMENT_NODE) {
      
      if (!strcmp((char *)pt->name, "type")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  type = atoi((char *)data);
	  free(data);
	}
	else {
	  fprintf(stderr, "type not found in the configuration file\n");
	  exit(-1);
	}
      }

      if (!strcmp((char *)pt->name, "ip")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  inet_pton(AF_INET, (char *)data, &gatewayIP);
	  free(data);
	}
	else {
	  fprintf(stderr, "gateway IP not found in the configuration file\n");
	  exit(-1);
	}
      } 
      
      if (!strcmp((char *)pt->name, "nodeID")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  inet_pton(AF_INET6, (char *)data, &nid);
	  setNID32();
	  free(data);
	}
	else {
	  fprintf(stderr, "nid not found in the configuration file\n");
	  exit(-1);
	}
      }
      
      if (!strcmp((char *)pt->name, "nidgw")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  inet_pton(AF_INET6, (char *)data, &nidgw);
	  free(data);	
	}
	else {
	  fprintf(stderr, "nidgw not found in the configuration file\n");
	  exit(-1);
	}
      }

      if (!strcmp((char *)pt->name, "nidcorehome")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  inet_pton(AF_INET6, (char *)data, &nidcorehome);
	  free(data);
	}
	else {
	  fprintf(stderr, "nidcorehome not found in the configuration file\n");
	  exit(-1);
	}
      }

      if (!strcmp((char *)pt->name, "nidcore")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  inet_pton(AF_INET6, (char *)data, &nidcore);
	  free(data);
	}
	else {
	  fprintf(stderr, "nidcore not found in the configuration file\n");
	  exit(-1);
	}
      }
      
      if (!strcmp((char *)pt->name, "nidrvs")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  inet_pton(AF_INET6, (char *)data, &nidrvs);
	  free(data);
	}
	else {
	  fprintf(stderr, "nidrvs not found in the configuration file\n");
	  exit(-1);
	}
      }

      if (!strcmp((char *)pt->name, "rvsIP")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  inet_pton(AF_INET, (char *)data, &rvsIP);
	  free(data);
	}
	else {
	  fprintf(stderr, "rvsIP not found in the configuration file\n");
	  exit(-1);
	}
      }

      if (!strcmp((char *)pt->name, "rvsTimeout")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  rvsTimeout = atoi((char *)data);
	  free(data);
	}
	else {
	  fprintf(stderr, "rvsTimeout not found in the configuration file\n");
	  exit(-1);
	}
      }

      if (!strcmp((char *)pt->name, "niddns")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  inet_pton(AF_INET6, (char *)data, &niddns);
	  free(data);
          update_dns_server_address(niddns);
	}
	else {
	  fprintf(stderr, "niddns not found in the configuration file\n");
	  exit(-1);
	}
      }

      if (type == CORE){

	if (!strcmp((char *)pt->name, "niddht")) {
	  if ((data = xmlNodeGetContent(pt->children)) != NULL){
	    inet_pton(AF_INET6, (char *)data, &niddht);
	    free(data);
	  }
	  else {
	    fprintf(stderr, "niddht not found in the configuration file\n");
	    exit(-1);
	  }
	}
	
	if (!strcmp((char *)pt->name, "dhtIP")) {
	  if ((data = xmlNodeGetContent(pt->children)) != NULL){
	    inet_pton(AF_INET, (char *)data, &dhtIP);
	    free(data);
	  }
	  else {
	    fprintf(stderr, "dhtIP not found in the configuration file\n");
	    exit(-1);
	  }
	}
	
	if (!strcmp((char *)pt->name, "dhtPort")) {
	  if ((data = xmlNodeGetContent(pt->children)) != NULL){
	    dhtPort = atoi((char *)data);
	    free(data);
	  }
	  else {
	    fprintf(stderr, "dhtPort not found in the configuration file\n");
	    exit(-1);
	  }
	}

	if (!strcmp((char *)pt->name, "dht")) {
	  if ((data = xmlNodeGetContent(pt->children)) != NULL){
	    dht = atoi((char *)data);
	    free(data);
	  }
	}
      }
      
      
      if (!strcmp((char *)pt->name, "dhcp")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  dhcp = atoi((char *)data);
 	  free(data);
	}
      }

      if (!strcmp((char *)pt->name, "dns")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  dns = atoi((char *)data);
 	  free(data);
	}
      }
      
      if (!strcmp((char *)pt->name, "primaryIface")) {
	if ((data = xmlNodeGetContent(pt->children)) != NULL){
	  bzero(primaryIface, 32);
	  strcpy(primaryIface, (char *) data);
	  free(data);
	}
	else {
	  fprintf(stderr, "primary interface not selected in the configuration file\n");
	  exit(-1);
	}
      }

      
      
    }
    get_data(pt->children);
  }
}

void read_config(const char *filename)
{
  xmlDoc *doc 			= NULL;
  xmlNode *root_element 	= NULL;
  
  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION
    
    /*parse the file and get the DOM */
    doc = xmlReadFile(filename, NULL, 0);

  if (!doc) {
    printf("error: could not parse file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  root_element = xmlDocGetRootElement(doc);
  get_data(root_element);
  /*free the document */
  xmlFreeDoc(doc);
  xmlCleanupParser();
}

void set_conf_dhcp(char *str){
  const char *delimiter = "|";
  char *token;
  
  if ((token = strtok(str, delimiter)) != NULL){
    inet_pton(AF_INET6, token, &nidcore);
  }
  
  if ((token = strtok(NULL, delimiter)) != NULL){
    inet_pton(AF_INET6, token, &nidgw);
  }
  
  if ((token = strtok(NULL, delimiter)) != NULL){
    inet_pton(AF_INET, token, &gatewayIP);
  }

  if ((token = strtok(NULL, delimiter)) != NULL){
    inet_pton(AF_INET6, token, &nidrvs);
  }
  
  if ((token = strtok(NULL, delimiter)) != NULL){
    inet_pton(AF_INET, token, &rvsIP);
  }
  
  if ((token = strtok(NULL, delimiter)) != NULL){
    inet_pton(AF_INET6, token, &niddns);
  }

  if ((token = strtok(NULL, delimiter)) != NULL){
    inet_pton(AF_INET, token, &ip);
  }

  //Rodolfo  
  update_dns_server_address(niddns);
  updateNM();
  updatePH();
  //updateCMconf();
}

char *get_locator(const char *dev)
{
  const int s = new_socket();
  static struct ifreq ifr;
  static struct sockaddr_in *sin_pt = (struct sockaddr_in *) &ifr.ifr_addr;
  
  bzero(&ifr, sizeof(struct ifreq));  
  memcpy(ifr.ifr_name, dev, strlen(dev));
  
  if (ioctl(s, SIOCGIFADDR, &ifr) < 0){
    perror("SIOCGIFADDR");
    printf("device %s\n", dev);
  }
  
  close(s);
  return nid322char(sin_pt->sin_addr.s_addr);
}

void print_dhcp()
{
  static char aux[INET6_ADDRSTRLEN];

  printf("ok!\n-------- DHCP Response ------------\n");
  inet_ntop(AF_INET6, &nidcore, aux, INET6_ADDRSTRLEN);
  printf("Core NID:\t %s\n", aux);
  inet_ntop(AF_INET6, &nidgw, aux, INET6_ADDRSTRLEN);
  printf("Gateway NID:\t %s\n", aux);
  inet_ntop(AF_INET, &gatewayIP, aux, INET6_ADDRSTRLEN);
  printf("Gateway IP:\t %s\n", aux);
  inet_ntop(AF_INET6, &nidrvs, aux, INET6_ADDRSTRLEN);
  printf("RVS NID:\t %s\n", aux);
  inet_ntop(AF_INET, &rvsIP, aux, INET_ADDRSTRLEN);
  printf("RVS IP:\t\t %s\n", aux);
  inet_ntop(AF_INET6, &niddns, aux, INET6_ADDRSTRLEN);
  printf("DNS NID:\t %s\n", aux);
  inet_ntop(AF_INET, &ip, aux, INET_ADDRSTRLEN);
  printf("Node IP:\t %s\n", aux);
  printf("-------- End DHCP Response --------\n");
}

void print_conf()
{
  static char aux[INET6_ADDRSTRLEN];
	  
  printf("-------- Node information -----------\n");
  printf("Type:\t\t\t ");
  switch(type){
  case NODE:
    printf("NODE\n");
    break;
  case RVS:
    printf("RVS\n");
    break;
  case GATEWAY:
    printf("GATEWAY\n");
    break;
  case CORE:
    printf("CORE\n");
    break;
  default:
    printf("unknown type!\n");
    break;
  }

  inet_ntop(AF_INET6, &nid, aux, INET6_ADDRSTRLEN);
  printf("NID:\t\t\t %s\n", aux);
  inet_ntop(AF_INET6, &nidcorehome, aux, INET6_ADDRSTRLEN);
  printf("NIDGW CORE HOME:\t %s\n", aux);
  inet_ntop(AF_INET6, &nidcore, aux, INET6_ADDRSTRLEN);
  printf("NIDGW CORE:\t\t %s\n", aux);
  printf("-------- Gateway information --------\n");
  inet_ntop(AF_INET6, &nidgw, aux, INET6_ADDRSTRLEN);
  printf("Gateway NID:\t\t %s\n", aux);
  inet_ntop(AF_INET, &gatewayIP, aux, INET6_ADDRSTRLEN);
  printf("Gateway IP:\t\t %s\n", aux);
  printf("-------- RVS information ------------\n");
  inet_ntop(AF_INET6, &nidrvs, aux, INET6_ADDRSTRLEN);
  printf("RVS NID:\t\t %s\n", aux);
  inet_ntop(AF_INET, &rvsIP, aux, INET6_ADDRSTRLEN);
  printf("RVS IP:\t\t\t %s\n", aux);
  printf("-------- DNS information ------------\n");
  inet_ntop(AF_INET6, &niddns, aux, INET6_ADDRSTRLEN);
  printf("DNS NID:\t\t %s\n", aux);

  if (type == CORE){
    inet_ntop(AF_INET, &dhtIP, aux, INET6_ADDRSTRLEN);
    printf("-------- DHT information ------------\n");
    printf("DHT IP:\t\t\t %s\n", aux);
  }
}

void print_nid(const NID nid, const char *str){
  static char aux[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &nid, aux, INET6_ADDRSTRLEN);
  printf(">>>%s NID: %s\n", str, aux);
}

void print_nid32(const NID32 nid32, const char *str){
  static char aux[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &nid32, aux, INET_ADDRSTRLEN);
  printf(">>>%s NID32: %s\n", str, aux);
}

void print_ipv4(const NID32 nid32, const char *str){
  static char aux[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &nid32, aux, INET_ADDRSTRLEN);
  printf(">>>%s IP: %s\n", str, aux);
}

void print_ctrl(const NID_msg nmsg)
{
  static char aux[INET6_ADDRSTRLEN];
  
  printf("--- NID Control message: ---\n");
  printf("Type:\t\t %d\n", nmsg.type);
  inet_ntop(AF_INET6, &nmsg.srcNID, aux, INET6_ADDRSTRLEN);
  printf("srcNID:\t\t %s\n", aux);  
  inet_ntop(AF_INET6, &nmsg.destNID, aux, INET6_ADDRSTRLEN);
  printf("destNID:\t %s\n", aux);
  inet_ntop(AF_INET6, &nmsg.srcNIDgw, aux, INET6_ADDRSTRLEN);
  printf("srcNIDgw:\t %s\n", aux);
  inet_ntop(AF_INET6, &nmsg.destNIDgw, aux, INET6_ADDRSTRLEN);
  printf("destNIDgw:\t %s\n", aux);  
  inet_ntop(AF_INET6, &nmsg.nid, aux, INET6_ADDRSTRLEN);
  printf("NID:\t\t %s\n", aux);
  inet_ntop(AF_INET, &nmsg.ip, aux, INET6_ADDRSTRLEN);
  printf("IP:\t\t %s\n", aux);
}

void print_nh(const NID_header *nh_pt)
{
  static char aux[INET6_ADDRSTRLEN];

  printf("--- NID Header: ---\n");
  printf("Type:\t\t %d\n", nh_pt->type);
  inet_ntop(AF_INET6, &nh_pt->srcNID, aux, INET6_ADDRSTRLEN);
  printf("srcNID:\t\t %s\n", aux);  
  inet_ntop(AF_INET6, &nh_pt->destNID, aux, INET6_ADDRSTRLEN);
  printf("destNID:\t %s\n", aux);
  inet_ntop(AF_INET6, &nh_pt->srcNIDgw, aux, INET6_ADDRSTRLEN);
  printf("srcNIDgw:\t %s\n", aux);
  inet_ntop(AF_INET6, &nh_pt->destNIDgw, aux, INET6_ADDRSTRLEN);
  printf("destNIDgw:\t %s\n", aux);  
}

char *nid2char(const NID nid)
{
  char *pt = (char *) calloc(INET6_ADDRSTRLEN, sizeof(char));
  inet_ntop(AF_INET6, &nid, pt, INET6_ADDRSTRLEN);
  
  return pt;
}

char *nid322char(const NID32 nid32)
{
  char *pt = (char *) calloc(INET_ADDRSTRLEN, sizeof(char));
  inet_ntop(AF_INET, &nid32, pt, INET_ADDRSTRLEN);
  
  return pt;
}

NID char2nid(char *str)
{
  NID nid;

  inet_pton(AF_INET6, str, &nid); 
  return nid;
}

NID32 char2nid32(char *str)
{
  NID32 nid32;

  inet_pton(AF_INET, str, &nid32);
  return nid32;
}

NID32 nid2nid32(NID nid){
  NID32 aux;  

  static u_int8_t *pt;
  static u_int8_t *pt32;

  pt = (u_int8_t *) &nid + 12;
  pt32 = (u_int8_t *) &aux;

  memcpy(pt32, pt, 4);
  *pt32 = 1;
  return aux;
}

