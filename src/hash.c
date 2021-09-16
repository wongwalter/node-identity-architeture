#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

#include "hash.h"
//#include "connection_mapper.h"
//#define TEST

/*
** public domain code by Jerry Coffin, with improvements by HenkJan Wolthuis.
*/


/* Initialize the hash_table to the size asked for.  Allocates space
** for the correct number of pointers and sets them to NULL.  If it
** can't allocate sufficient memory, signals error by setting the size
** of the table to 0.
*/

hash_table *construct_hash_table(hash_table *table, size_t size)
{
  bucket **temp;
  
  table->size = size;
  table->table = (bucket **) calloc(size, sizeof (bucket *));
  temp = table->table;
  
  if (temp == NULL){
    table->size = 0;
    return table;
  }
  
  return table;
}


/*
** Hashes a string to produce an unsigned short, which should be
** sufficient for most purposes.
*/

unsigned hash(NID nid)
{
  unsigned ret_val = 0;
  u_int i, n;
  u_char *pt = (u_char *) &nid;
  
  for (n = 0; n < 16; n++){    
    i = (int) *pt++;
    ret_val ^= i;
    ret_val <<= 1;
  }
  
  return ret_val;
}


/*
** Insert 'key' into hash table.
** Returns pointer to old data associated with the key, if any, or
** NULL if the key wasn't in the table previously.
*/

void *hash_insert(NID nid, void *data, hash_table *table)
{
  unsigned val = hash(nid) % table->size;
  bucket *ptr;
  /*
  ** NULL means this bucket hasn't been used yet.  We'll simply
  ** allocate space for our new bucket and put our data there, with
  ** the table pointing at it.
  */

  if (NULL == (table->table)[val]){
    (table->table)[val] = (bucket *)malloc(sizeof(bucket));
    if (NULL==(table->table)[val])
      return NULL;
    
    (table->table)[val]->nid = nid;
    (table->table)[val]->next = NULL;
    (table->table)[val]->data = data;
    
    return (table->table)[val]->data;
  }
  
  /*
  ** This spot in the table is already in use.  See if the current string
  ** has already been inserted, and if so, increment its count.
  */
  
  for (ptr = (table->table)[val]; ptr; ptr = ptr->next)
    if (!memcmp(&nid, &ptr->nid, 16)){
      void *old_data;
      
      old_data = ptr->data;
      ptr->data = data;
      return old_data;
    }
  
  /*
  ** This key must not be in the table yet.  We'll add it to the head of
  ** the list at this spot in the hash table.  Speed would be
  ** slightly improved if the list was kept sorted instead.  In this case,
  ** this code would be moved into the loop above, and the insertion would
  ** take place as soon as it was determined that the present key in the
  ** list was larger than this one.
  */
  
  ptr = (bucket *)malloc(sizeof(bucket));
  if (!ptr)
    return 0;
  
  ptr->nid = nid;
  ptr->data = data;
  ptr->next = (table->table)[val];
  (table->table)[val] = ptr;
  
  return data;
}


/*
** Look up a key and return the associated data.  Returns NULL if
** the key is not in the table.
*/

void *hash_lookup(NID nid, hash_table *table)
{
  unsigned val = hash(nid) % table->size;
  bucket *ptr;
    
  if (NULL == (table->table)[val])
    return NULL;
  
  for (ptr = (table->table)[val]; ptr; ptr = ptr->next){
    if (!memcmp(&nid, &ptr->nid, 16)){
      return ptr->data;
    }
  }
  
  return NULL;
}

/*
** Delete a key from the hash table and return associated
** data, or NULL if not present.
*/

void *hash_del(NID nid, hash_table *table)
{
  unsigned val = hash(nid) % table->size;
  void *data;
  bucket *ptr, *last = NULL;
  
  if (NULL == (table->table)[val])
    return NULL;
  
  /*
  ** Traverse the list, keeping track of the previous node in the list.
  ** When we find the node to delete, we set the previous node's next
  ** pointer to point to the node after ourself instead.  We then delete
  ** the key from the present node, and return a pointer to the data it
  ** contains.
  */
  
  for (last = NULL, ptr = (table->table)[val]; ptr; last = ptr, ptr = ptr->next){
    if (!memcmp(&nid, &ptr->nid, 16)){
      if (last){
	data = ptr->data;
	last->next = ptr->next;
	free(ptr);
	return data;
      }
      
      /*
      ** If 'last' still equals NULL, it means that we need to
      ** delete the first node in the list. This simply consists
      ** of putting our own 'next' pointer in the array holding
      ** the head of the list.  We then dispose of the current
      ** node as above.
      */
      
      else{
	data = ptr->data;
	(table->table)[val] = ptr->next;
	free(ptr);
	return data;
      }
    }
  }
  
  /*
  ** If we get here, it means we didn't find the item in the table.
  ** Signal this by returning NULL.
  */
  
  return NULL;
}

/*
** Frees a complete table by iterating over it and freeing each node.
** the second parameter is the address of a function it will call with a
** pointer to the data associated with each node.  This function is
** responsible for freeing the data, or doing whatever is needed with
** it.
*/

void free_hash_table(hash_table *table, void (*func)(void *))
{
  unsigned i;
  bucket *ptr, *temp;
  
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

/*
** Simply invokes the function given as the second parameter for each
** node in the table, passing it the key, the associated data and 'rock'.
*/

void hash_enumerate(hash_table *table, ll_head **ll)
{
  register unsigned i;
  bucket *temp;
  conn_entry *ce;
  char aux[INET_ADDRSTRLEN];
  
  for (i = 0; i < table->size; i++){
    if ((table->table)[i] != NULL){
      for (temp = (table->table)[i]; temp; temp = temp->next){
	ce = (conn_entry *) temp->data;
	if ((ce->tstamp + RVS_UPDATE_TIMEOUT >  time(NULL)) || (ce->tstamp == 0)){
	  inet_ntop(AF_INET, &ce->ip, aux, INET_ADDRSTRLEN);
	  add_ll(temp->nid, aux, *ll);
	}
      }
    }
  }
}


#ifdef TEST

#include <stdio.h>

void fatal(const char* s, int code)
{
  fprintf(stderr, "hash: %s\r\n", s);
  exit(code);
}

void printer(char *string, void *data)
{
  printf("%s: %s\n", string, (char *)data);
}

int main(void)
{
  hash_table table;
  char *str, buf[128];  
  NID nid1, nid2;
  void *tmp;
  
  construct_hash_table(&table, 100);
  
  inet_pton(AF_INET6, "3ffe::3", &nid1);
  inet_pton(AF_INET6, "3ffe::4", &nid2);
  
  inet_ntop(AF_INET6, &nid1, buf, 128);
  printf("nid1=%s\n", buf);
  
  inet_ntop(AF_INET6, &nid2, buf, 128);
  printf("nid2=%s\n", buf);

  hash_insert(nid1, &nid2, &table);
  hash_insert(nid2, &nid1, &table);
  
  tmp = hash_lookup(nid1, &table);
  inet_ntop(AF_INET6, tmp, buf, 128);  
  
  printf("lookup for nid1=%s\n", buf);

  tmp = hash_lookup(nid2, &table);
  inet_ntop(AF_INET6, tmp, buf, 128);  

  printf("lookup for nid2=%s\n", buf);

  free_hash_table(&table, NULL);
  
  return 0;
}

#endif /* TEST */
