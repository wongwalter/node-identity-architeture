#include "nid.h"

#ifndef __LLIST__
#define __LLIST__

typedef struct _ll_node{
  NID nid;
  char *ip;
  struct _ll_node *next;
}ll_node;

typedef struct _ll_head{
  int size;
  ll_node *next;  
}ll_head;

#endif

ll_head *init_ll();
int add_ll(NID nid, char *ip, ll_head *ll);
void print_ll(ll_head *ll);
void destroy_ll(ll_head *ll);
