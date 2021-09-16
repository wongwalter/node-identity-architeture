int modify_dns_query(unsigned char *buf, unsigned char *fqdn, uint16_t *queryType);

int analyse_dns_response(unsigned char *buf, unsigned char *queryName, unsigned char *answerAns, unsigned char *nsNs);						 

int get_nids(unsigned char *nidtuple, unsigned char *nnid, unsigned char *nnidr);

int build_dns_response(unsigned char *buf, unsigned int nid32addr, unsigned char nidaddr[16], uint16_t qType);

int build_dns_response_with_fqdn_table(unsigned char *buf, unsigned int nid32addr, unsigned char nidaddr[16], uint16_t qType);

void update_dns_server_address(NID niddnsserver);

void *thread_dnshandler(void);

// O Walter é um mala!
