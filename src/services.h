#include "nid.h"

void *thread_nidfilter(void);
void *thread_ph_output(void);
void *thread_ph_input(void);
void *thread_nr_output(void);
void *thread_nr_input(void);
void *thread_xmlrpc_server(void);
void *thread_mobility(void);
void *thread_control(void);
void *thread_rvs_send(void);
int init_tap(char *dev, NID nid, NID32 nid32);
