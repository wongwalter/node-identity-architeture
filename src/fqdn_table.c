//#include <stdlib.h>
//#include <string.h>
#include "fqdn_hash.h"

fqdn_hash_table fqdnTable;

int newFQDNTEntry(char *fqdn, char *nids)
{    
  return (fqdn_hash_insert(fqdn, nids, &fqdnTable) && 1);
}

char *getNIDsbyName(char *fqdn)
{
  char *nids = (char *) fqdn_hash_lookup(fqdn, &fqdnTable);
  return (nids ? nids : NULL);
}

int init_fqdn_table()
{    
  return (construct_fqdn_hash_table(&fqdnTable, 100) && 1);
}

void destroy_fqdn_table()
{
  free_fqdn_hash_table(&fqdnTable, NULL);
}
