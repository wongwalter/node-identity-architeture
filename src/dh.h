#include <openssl/dh.h>
#define PRIME_LEN 32 // 3 < PRIME_LEN < 490

typedef struct _dh{
  char p[PRIME_LEN];
  char g[PRIME_LEN];
  char pub_key[PRIME_LEN];
} dh_struct;
