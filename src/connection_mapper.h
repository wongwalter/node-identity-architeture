#include "hash.h"

int  getCMEntry(const NID nid, NID32 *dst);
void newCMEntry(const NID nid, const NID32 ip);
void staticCMEntry(const NID nid, const NID32 ip);
void delCMEntry(const NID nid);
void init_conn_mapper();
void destroy_conn_mapper();
void updateCMconf();
ll_head *getAllConnections();
