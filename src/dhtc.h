void init_dhtable();
void destroy_dhtable(); 
void addDHTEntry(NID nid, const NID32 ip);
int  getDHTEntry(const NID nid, NID32 *dst);

