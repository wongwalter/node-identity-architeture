#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "llist.h"

//#define TEST

ll_head *init_ll()
{  
  ll_head *ll = (ll_head *) calloc(1, sizeof(ll_head));
  
  ll->size = 0;
  ll->next = NULL;
  
  return ll;
}

ll_node *tail(ll_head *head)
{
  ll_node *current = head->next;
  
  while (current){

    if (current->next)
      current = current->next;
    else {
      return current;
    }
  }

  return current;
}

int add_ll(NID nid, char *ip, ll_head *ll)
{  
  ll_node *last, *node;
  
  node = (ll_node *) calloc(1, sizeof(ll_node));  
  node->nid = nid;
  node->ip = strdup(ip);
  node->next = NULL;
  
  ll->size++;  
  last = tail(ll);
  
  if (last){
    last->next = node;
  }
  else {
    ll->next = node;    
  }

  return 0;
}


int remove_ll(char *nid);
char *search_ll(char *nid);

char buf[128];

void print_ll(ll_head *ll)
{
  if (ll){    
    ll_node *pt = ll->next;
    
    while (pt){
      inet_ntop(AF_INET6, (void *)&pt->nid, buf, 128);
      printf("nid=%s ip=%s\n", buf, pt->ip);
      pt = pt->next;
    }
    
  }
}


void free_ll(ll_node *node)
{
  if (node){
    free_ll(node->next);
    free(node);
  }  
}

void destroy_ll(ll_head *ll)
{
  free_ll(ll->next);
  free(ll);
  ll = NULL;
}

#ifdef TEST
 
int main()
{ 
  ll_head *ll = init_ll();
  
  add_ll("nid1", "ip1", ll);
  add_ll("nid2", "ip2", ll);
  add_ll("nid3", "ip3", ll);
  print_ll(ll);
  destroy_ll(ll);
  print_ll(ll);
  
  return 0;
}

#endif
