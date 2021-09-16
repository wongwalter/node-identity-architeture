#include <sys/types.h>
#include <netinet/in.h>

#ifndef __NIDH__
#define __NIDH__

typedef struct in6_addr NID;
typedef in_addr_t NID32;

enum rvs_msg_type {RVS_UPDATE = 5,
		   RVS_ACK,
		   RVS_GET,
		   RVS_RESPONSE,
};

enum nid_ctrl_type {DATA,
		    REDIRECT,
		    RELOCATE,
		    REGISTRY,
		    REDIRECT_CORE,
};

enum node_types {NODE, 
		 RVS, 
		 GATEWAY, 
		 CORE};

enum mobility_types {INTRADOMAIN, 
		     INTERDOMAIN};

typedef struct _nid_header{
  enum nid_ctrl_type type;
  NID srcNID;
  NID destNID;
  NID srcNIDgw;
  NID destNIDgw;
}NID_header;

typedef struct _nid_tuple{
  NID NIDt;
  NID NIDgw;
}NID_tuple;

typedef struct _control_message{
  enum rvs_msg_type type;
  NID srcNID;
  NID destNID;
  NID srcNIDgw;
  NID destNIDgw;
  NID nid;
  NID32 ip;
}NID_msg;

typedef struct _rvs_msg{
  enum rvs_msg_type type;
  NID srcNID;
  NID destNID;
  NID srcNIDgw;
  NID destNIDgw;
  NID nid;
  NID32 ip;
}RVS_msg;

typedef struct _cm_struct{
  NID32 ip;
  time_t tstamp;
}conn_entry;

int sp_nf2ph[2];
int sp_ph2nr[2];
int sp_nr2ph[2];
int sp_nr[2];
int sp_mo2ph[2];
int sp_cm2rvs[2];
int sp_gwt2ph[2];
int sp_co2gwl[2];
int sp_ph2co[2];
int sp_rvs2ph[2];
int sp_co2rvs[2];
int sp_rvs2gwt[2];
int sp_gwl2gwt[2];
int sp_dht[2];

#endif

#define DEVICE               "nid0"
#define DEVICE_MTU           1392
#define CONFIG_FILE          "nid.conf"
#define DATA_PORT            20000
#define RVS_UPDATE_TIMEOUT   5
#define RVS_PROACTIVE        RVS_UPDATE_TIMEOUT-1
#define RVS_TIMEOUT          10
#define GATEWAY_TIMEOUT      10

#define DHT_UPDATE           8
#define DHT_TTL              10

#define RVS_LISTENER         9090
#define RVS_TALKER           9091

#define DEBUG_PRINT_RT_TIME  3

#ifndef MAX
#define MAX(a, b) ((a > b) ? a : b)
#endif

//#define DEBUG_NR_IN
//#define DEBUG_NR_OUT
//#define DEBUG_PRINT_RT
//#define DEBUG_GMS
