#define DNS_MAXNAMELENGTH 	255

struct dnshdr {
   uint16_t transactionID;
   uint16_t flags;
   uint16_t questions;
   uint16_t answerRR;
   uint16_t authorityRR;
   uint16_t additionalRR;
};

struct query {
   unsigned char name[DNS_MAXNAMELENGTH+1];
   uint16_t type;
   uint16_t class;
};

struct answer {
   uint16_t name;
   uint16_t type;
   uint16_t class;
   uint32_t ttl;
   uint16_t dataLength;
   unsigned char ans[DNS_MAXNAMELENGTH+1];
};
struct authoritativeNS {
   uint16_t name;
   uint16_t type;
   uint16_t class;
   uint32_t ttl;
   uint16_t dataLength;
   unsigned char ns[DNS_MAXNAMELENGTH+1];
};



