void init_registry_table();
void destroy_registry_table();
void addRegistry(const NID nid, const NID32 ip);
int  getRegistry(const NID nid, NID32 *dst);
