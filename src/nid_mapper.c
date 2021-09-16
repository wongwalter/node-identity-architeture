#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "nid_mapper.h"
#include "hash.h"
#include "hash32.h"
#include "configuration.h"

//#define TEST

hash_table nidTable;
hash_table32 nid32Table;

NID niddns;
NID32 nid32dns;

void newNMEntry(const NID nid, const NID nidgw)
{ 
  NMEntry *nme = (NMEntry *) hash_lookup(nid, &nidTable);
  
  if (nme){
    nme->nidgw = nidgw;    
    hash_insert(nid, nme, &nidTable);
    newNMEntry32(nid2nid32(nid), nid, nidgw);
    return;
  }
  
  nme = (NMEntry *) calloc(1, sizeof(NMEntry));  
  nme->nidgw = nidgw;  
  hash_insert(nid, nme, &nidTable);
  newNMEntry32(nid2nid32(nid), nid, nidgw);  
}


void newNMEntry32(const NID32 nid32, const NID nid, const NID nidgw)
{
  NMEntry32 *nme = (NMEntry32 *) hash_lookup32(nid32, &nid32Table);
  
  if (nme){
    nme->nid = nid;
    nme->nidgw = nidgw;
    hash_insert32(nid32, nme, &nid32Table);
    return;
  }
  
  nme = (NMEntry32 *) calloc(1, sizeof(NMEntry32));  
  nme->nid = nid;
  nme->nidgw = nidgw;
  hash_insert32(nid32, nme, &nid32Table);
}


NMEntry *getNMEntry(const NID nid)
{
  return (NMEntry *)hash_lookup(nid, &nidTable);
}


NMEntry32 *getNMEntry32(const NID32 nid32)
{
  return ((NMEntry32 *) hash_lookup32(nid32, &nid32Table));
}


int init_nid_mapper()
{    
  construct_hash_table(&nidTable, 10);
  construct_hash_table32(&nid32Table, 10);

  if (!get_dns()){
  
    NID32 nid32;
    NID nid, nidgw;
    
    inet_pton(AF_INET, "1.0.0.1", &nid32);
    inet_pton(AF_INET6, "3ffe::1", &nid);
    inet_pton(AF_INET6, "bbbb::b", &nidgw);
    newNMEntry32(nid32, nid, nidgw);

    inet_pton(AF_INET, "1.0.0.2", &nid32);
    inet_pton(AF_INET6, "3ffe::2", &nid);
    inet_pton(AF_INET6, "3ffe::7", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.0.3", &nid32);
    inet_pton(AF_INET6, "3ffe::3", &nid);
    inet_pton(AF_INET6, "3ffe::4", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.0.4", &nid32);
    inet_pton(AF_INET6, "3ffe::4", &nid);
    inet_pton(AF_INET6, "3ffe::4", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.0.5", &nid32);
    inet_pton(AF_INET6, "3ffe::5", &nid);
    inet_pton(AF_INET6, "3ffe::4", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.0.6", &nid32);
    inet_pton(AF_INET6, "3ffe::6", &nid);
    inet_pton(AF_INET6, "bbbb::b", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.0.7", &nid32);
    inet_pton(AF_INET6, "3ffe::7", &nid);
    inet_pton(AF_INET6, "3ffe::7", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.0.8", &nid32);
    inet_pton(AF_INET6, "3ffe::8", &nid);
    inet_pton(AF_INET6, "bbbb::b", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.0.9", &nid32);
    inet_pton(AF_INET6, "3ffe::9", &nid);
    inet_pton(AF_INET6, "3ffe::7", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.0.11", &nid32);
    inet_pton(AF_INET6, "3ffe:3::11", &nid);
    inet_pton(AF_INET6, "ffff:ffff::f", &nidgw);
    newNMEntry32(nid32, nid, nidgw);  
    
    inet_pton(AF_INET, "1.0.1.1", &nid32);
    inet_pton(AF_INET6, "3ffe::101", &nid);
    inet_pton(AF_INET6, "ffff:ffff::f", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.1.2", &nid32);
    inet_pton(AF_INET6, "3ffe::102", &nid);
    inet_pton(AF_INET6, "ffff:ffff::f", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.1.3", &nid32);
    inet_pton(AF_INET6, "3ffe::103", &nid);
    inet_pton(AF_INET6, "ffff:ffff::f", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
    
    inet_pton(AF_INET, "1.0.0.16", &nid32);
    inet_pton(AF_INET6, "3ffe::10", &nid);
    inet_pton(AF_INET6, "ffff:ffff::f", &nidgw);
    newNMEntry32(nid32, nid, nidgw);
  }

  return 0;
}

void destroy_nid_mapper()
{
  free_hash_table(&nidTable, NULL);
  printf("Destroying NID Mapper table...\n");
  free_hash_table32(&nid32Table, NULL);
  printf("Destroying NID32 Mapper table...\n");
}


void updateNM(){

  hash_del(niddns, &nidTable);
  hash_del32(nid32dns, &nid32Table);
  
  newNMEntry(getNIDdns(), getNIDcore());
  newNMEntry32(nid2nid32(getNIDdns()), getNIDdns(), getNIDcore());
}



#ifdef TEST

#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{  
  NID nid1, nid2, nid3;
  NID32 nid132, nid232;
  char buf[128];
  void *tmp;
  char *str = "teste";
  NMEntry *npt;
  NMEntry32 *npt32;

  init_nid_mapper();     

  /*
  inet_pton(AF_INET6, "3ffe::3", &nid1);
  inet_pton(AF_INET6, "3ffe::1", &nid2);
  
  inet_pton(AF_INET, "1.0.0.17", &nid132); 
  inet_pton(AF_INET, "1.0.1.5", &nid232);

  newNMEntry(nid1, str, nid1);
    
  npt = (NMEntry *) getNMEntry(nid1);  
  inet_ntop(AF_INET6, &npt->nidgw, buf, 128);
  printf("print nid=%s\n", buf);
  
  newNMEntry32(nid132, str, nid1, nid2);  
  */
  
  inet_pton(AF_INET, "1.0.0.17", &nid132);

  npt32 = (NMEntry32 *) getNMEntry32(nid132);  
  inet_ntop(AF_INET6, &npt32->nid, buf, 128);
  printf("print nid=%s\n", buf);

  destroy_nid_mapper();

  return 0;
}

#endif
