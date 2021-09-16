#include <stddef.h>           /* For size_t */
#include "nid.h"
#include "llist.h"

#ifndef HASH32__H
#define HASH32__H

typedef struct bucket32 {
  NID32 nid32;
  void *data;
  struct bucket32 *next;
} bucket32;

typedef struct hash_table32 {
  size_t size;
  bucket32 **table;
} hash_table32;

unsigned hash32(NID32 nid32);
hash_table32 *construct_hash_table32(hash_table32 *table, size_t size);
void *hash_insert32(NID32 nid32, void *data, hash_table32 *table);
void *hash_lookup32(NID32 nid32, hash_table32 *table);
void *hash_del32(NID32 nid32, hash_table32 *table);
void free_hash_table32(hash_table32 *table, void (*func)(void *));

#endif /* HASH32__H */
