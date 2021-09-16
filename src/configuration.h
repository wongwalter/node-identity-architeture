#include "nid.h"

void read_config(const char *filename);
void set_conf_dhcp(char *str);

char *get_locator(const char *dev);
char *getPrimaryIface();

unsigned short int getType();
unsigned short int getGatewayPort();
unsigned short int getRvsPort();
unsigned short int getRvsTimeout();
unsigned short int getDhtPort();
unsigned short int get_dhcp();
unsigned short int get_dht();
unsigned short int get_dns();
unsigned short int get_mobility();

NID getNID();
NID getNIDcore();
NID getNIDcorehome();
NID getNIDgw();
NID getNIDrvs();
NID getNIDdht();
NID getNIDdns();

NID32 getGatewayIP();
NID32 getNID32();
NID32 getRvsIP();
NID32 getDhtIP();
NID32 getIP();

void setGatewayPort(const unsigned short int port);
void setRvsIP(const NID32 newIP);
void setRvsPort(const unsigned short int port);
void set_mobility(const unsigned short int);
void setIP(const NID32 newIP);

void print_conf();
void print_dhcp();
void print_nid(const NID nid, const char *str);
void print_nid32(const NID32 nid32, const char *str);
void print_ipv4(const NID32 nid32, const char *str);
void print_ctrl(const NID_msg msg);
void print_nh(const NID_header *nh_pt);

char *nid2char(const NID nid);
char *nid322char(const NID32 nid32);
NID  char2nid(char *str);
NID32 char2nid32(char *str);
NID32 nid2nid32(NID nid);
