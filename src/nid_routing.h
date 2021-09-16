#include "nid.h"

void init_rtable();
void destroy_rtable();
void newRTEntry(const NID nid, const NID32 ip);
int  getRTEntry(const NID nid, NID32 *dst);
void staticRTEntry(const NID nid, const NID32 ip);
