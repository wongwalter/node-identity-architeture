#include "nid.h"

#ifndef __NID_MAPPER__
#define __NID_MAPPER__

typedef struct _nmentry{
  char fqdn[256];
  NID nidgw;
} NMEntry;

typedef struct _nmentry32{
  char fqdn[256];
  NID nid;
  NID nidgw;
} NMEntry32;

#endif

void newNMEntry(const NID nid, const NID nidgw);
void newNMEntry32(const NID32 nid32, const NID nid, const NID nidgw);
NMEntry *getNMEntry(const NID nid);
NMEntry32 *getNMEntry32(const NID32 nid);

int init_nid_mapper();
void destroy_nid_mapper();
