#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hash32.h"

//#define TEST

hash_table32 *construct_hash_table32(hash_table32 *table, size_t size)
{
  bucket32 **temp;
  
  table->size = size;
  table->table = (bucket32 **) calloc(size, sizeof (bucket32 *));
  temp = table->table;
  
  if (temp == NULL){
    table->size = 0;
    return table;
  }
  
  return table;
}

unsigned hash32(NID32 nid32)
{
  unsigned ret_val = 0;
  u_int i, n;
  u_char *pt = (u_char *) &nid32;
  
  for (n = 0; n < 4; n++){
    i = (int) *pt++;
    ret_val ^= i;
    ret_val <<= 1;
  }
  
  return ret_val;
}


void *hash_insert32(NID32 nid32, void *data, hash_table32 *table)
{
  unsigned val = hash32(nid32) % table->size;
  bucket32 *ptr;

  if (NULL == (table->table)[val]){
    (table->table)[val] = (bucket32 *)malloc(sizeof(bucket32));
    if (NULL==(table->table)[val])
      return NULL;
    
    (table->table)[val]->nid32 = nid32;
    (table->table)[val]->next = NULL;
    (table->table)[val]->data = data;
    
    return (table->table)[val]->data;
  }
  
  for (ptr = (table->table)[val]; ptr; ptr = ptr->next)
    if (!memcmp(&nid32, &ptr->nid32, 4)){
      void *old_data;
      
      old_data = ptr->data;
      ptr->data = data;
      return old_data;
    }
  
  if ((ptr = (bucket32 *)malloc(sizeof(bucket32))) == NULL)
    return 0;
  
  ptr->nid32 = nid32;
  ptr->data = data;
  ptr->next = (table->table)[val];
  (table->table)[val] = ptr;
  
  return data;
}

void *hash_lookup32(NID32 nid32, hash_table32 *table)
{
  unsigned val = hash32(nid32) % table->size;
  bucket32 *ptr;
    
  if (NULL == (table->table)[val])
    return NULL;
  
  for (ptr = (table->table)[val]; ptr; ptr = ptr->next){
    if (!memcmp(&nid32, &ptr->nid32, 4)){
      return ptr->data;
    }
  }
  
  return NULL;
}

void *hash_del32(NID32 nid32, hash_table32 *table)
{
  unsigned val = hash32(nid32) % table->size;
  void *data;
  bucket32 *ptr, *last = NULL;
  
  if (NULL == (table->table)[val])
    return NULL;
  
  for (last = NULL, ptr = (table->table)[val]; ptr; last = ptr, ptr = ptr->next){
    if (!memcmp(&nid32, &ptr->nid32, 4)){
      if (last){
	data = ptr->data;
	last->next = ptr->next;
	free(ptr);
	return data;
      }      
      else{
	data = ptr->data;
	(table->table)[val] = ptr->next;
	free(ptr);
	return data;
      }
    }
  }
  
  return NULL;
}

void free_hash_table32(hash_table32 *table, void (*func)(void *))
{
  unsigned i;
  bucket32 *ptr, *temp;
  
  for (i = 0; i < table->size; i++){
    ptr = (table->table)[i];

    while (ptr){
      temp = ptr;
      ptr = ptr->next;
      if (func)
	func(temp->data);
      free(temp);
    }
  }
  
  free(table->table);
  table->table = NULL;
  table->size = 0;
}

#ifdef TEST

#include <stdio.h>

int main(void)
{
  hash_table32 table;
  char *str, buf[32];
  NID32 nid1, nid2;
  void *tmp;
  
  construct_hash_table32(&table, 100);
  
  inet_pton(AF_INET, "1.1.1.1", &nid1);
  inet_pton(AF_INET, "2.2.2.2", &nid2);
  
  inet_ntop(AF_INET, &nid1, buf, 32);
  printf("nid1=%s\n", buf);
  
  inet_ntop(AF_INET, &nid2, buf, 32);
  printf("nid2=%s\n", buf);

  hash_insert32(nid1, &nid2, &table);
  hash_insert32(nid2, &nid1, &table);
  
  tmp = (void *)hash_lookup32(nid1, &table);
  inet_ntop(AF_INET, tmp, buf, 32);
  
  printf("lookup for nid1=%s\n", buf);

  tmp = (void *)hash_lookup32(nid2, &table);
  inet_ntop(AF_INET, tmp, buf, 32);

  printf("lookup for nid2=%s\n", buf);

  free_hash_table32(&table, NULL);
  
  return 0;
}

#endif /* TEST */
