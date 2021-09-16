#include <string.h>
#include <stdlib.h>

#include "fqdn_hash.h"

//#define FQDN_TEST

/*
** public domain code by Jerry Coffin, with improvements by HenkJan Wolthuis.
**
** Tested with Visual C 1.0 and Borland C 3.1.
** Compiles without warnings, and seems like it should be pretty
** portable.
**
** Modified for use with libcyrus by Ken Murchison.
**  - prefixed functions with 'hash_' to avoid symbol clashing
**  - make hash() a public function
**  - use xmalloc() and xstrdup()
**  - cleaned up free_hash_table(), doesn't use enumerate anymore
**  - added 'rock' to hash_enumerate()
*/


/* Initialize the hash_table to the size asked for.  Allocates space
** for the correct number of pointers and sets them to NULL.  If it
** can't allocate sufficient memory, signals error by setting the size
** of the table to 0.
*/

fqdn_hash_table *construct_fqdn_hash_table(fqdn_hash_table *table, size_t size)
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

unsigned fqdn_hash(const char *string)
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

/*
** Insert 'key' into hash table.
** Returns pointer to old data associated with the key, if any, or
** NULL if the key wasn't in the table previously.
*/

void *fqdn_hash_insert(char *key, void *data, fqdn_hash_table *table)
{
  unsigned val = fqdn_hash(key) % table->size;
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
      
    (table->table)[val]->key = strdup(key);
    (table->table)[val]->next = NULL;
    (table->table)[val]->data = data;
    
    return (table->table)[val]->data;
  }
  
  /*
  ** This spot in the table is already in use.  See if the current string
  ** has already been inserted, and if so, increment its count.
  */
  
  for (ptr = (table->table)[val]; ptr; ptr = ptr->next)
    if (!strcmp(key, ptr->key)){
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
  
  ptr->key = strdup(key);
  ptr->data = data;
  ptr->next = (table->table)[val];
  (table->table)[val] = ptr;
  
  return data;
}


/*
** Look up a key and return the associated data.  Returns NULL if
** the key is not in the table.
*/

void *fqdn_hash_lookup(char *key, fqdn_hash_table *table)
{
  unsigned val = fqdn_hash(key) % table->size;
  bucket *ptr;
  
  if (NULL == (table->table)[val])
    return NULL;
  
  for ( ptr = (table->table)[val]; ptr; ptr = ptr->next){
    if (!strcmp(key, ptr->key ))
      return ptr->data;
  }
  
  return NULL;
}

/*
** Delete a key from the hash table and return associated
** data, or NULL if not present.
*/

void *fqdn_hash_del(char *key, fqdn_hash_table *table)
{
  unsigned val = fqdn_hash(key) % table->size;
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
    if (!strcmp(key, ptr -> key)){
      if (last){
	data = ptr->data;
	last->next = ptr->next;
	free(ptr->key);
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
	free(ptr->key);
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

void free_fqdn_hash_table(fqdn_hash_table *table, void (*func)(void *))
{
  unsigned i;
  bucket *ptr, *temp;
  
  for (i = 0; i < table->size; i++){
    ptr = (table->table)[i];

    while (ptr){
      temp = ptr;
      ptr = ptr->next;
      free(temp->key);
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

void fqdn_hash_enumerate(fqdn_hash_table *table, void (*func)(char *, void *))
{
  unsigned i;
  bucket *temp;
  
  for (i = 0; i < table->size; i++){
    if ((table->table)[i] != NULL){
      for (temp = (table->table)[i]; temp; temp = temp->next)
	func(temp->key, temp->data);
    }
  }
}

#ifdef FQDN_TEST

#include <stdio.h>

void fqdn_fatal(const char* s, int code)
{
  fprintf(stderr, "hash: %s\r\n", s);
  exit(code);
}

void fqdn_printer(char *string, void *data)
{
  printf("%s: %s\n", string, (char *)data);
}

int main(void)
{
  fqdn_hash_table table;

  char *strings[] = {
    "The first string",
    "The second string",
    "The third string",
    "The fourth string",
    "A much longer string than the rest in this example.",
    "The last string",
    NULL
  };
  
  char *junk[] = {
    "The first data",
    "The second data",
    "The third data",
    "The fourth data",
    "The fifth datum",
    "The sixth piece of data"
  };
  
  int i;
  void *j;
  
  construct_fqdn_hash_table(&table,200);
  
  for (i = 0; strings[i]; i++ )
    fqdn_hash_insert(strings[i], junk[i], &table);
  
  for (i = 0; strings[i]; i++){
    printf("\n");
    fqdn_hash_enumerate(&table, printer);
    fqdn_hash_del(strings[i], &table);
  }
  
  for (i = 0; strings[i]; i++){
    j = fqdn_hash_lookup(strings[i], &table);
    if (!j)
      printf("\n'%s' is not in table",strings[i]);
    else 
      printf("\nERROR: %s was deleted but is still in table.", strings[i]);
  }
  
  free_fqdn_hash_table(&table, NULL);
  
  printf("\n");
  
  return 0;
}

#endif /* FQDN_TEST */
