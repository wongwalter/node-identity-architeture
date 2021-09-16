#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nid.h"
#include "tools.h"

int init_tap(char *dev, NID nid, NID32 nid32)
{
  int s					= new_socket();
  int tapfd				= 0;
  u_int flags				= 0;
  u_int mask				= 0;
  struct ifreq ifr;
  struct sockaddr_in *sin_pt 		= (struct sockaddr_in *) &ifr.ifr_addr;
  char aux[128];

  if ((tapfd = open("/dev/net/tun", O_RDWR)) < 0){
    perror("can't open tap");
    exit(0);
  }
  
  bzero(&ifr, sizeof(struct ifreq));
  
  ifr.ifr_flags = IFF_NO_PI | IFF_TAP;
  memcpy(ifr.ifr_name, dev, strlen(dev));
  
  if (ioctl(tapfd, TUNSETIFF, (void *)&ifr) < 0){
    perror("TUNSETIFF");
    exit(0);
  }
  
  bzero(&ifr, sizeof(struct ifreq));

  sin_pt->sin_family = AF_INET;
  memcpy(ifr.ifr_name, dev, strlen(dev));
  memcpy(&sin_pt->sin_addr, &nid32, 4);

  if (ioctl(s, SIOCSIFADDR, &ifr) < 0){
    perror("SIOCSIFADDR");
    exit(0);
  }

  printf("-------- Virtual Interface ----------\n");
  inet_ntop(AF_INET, &sin_pt->sin_addr, aux, 128);
  printf("Set device %s with address %s\n", dev, aux);
  
  bzero(&ifr, sizeof(ifr));
  
  memcpy(ifr.ifr_name, dev, strlen(dev));  
  ifr.ifr_mtu = DEVICE_MTU;
  
  if (ioctl(s, SIOCSIFMTU, &ifr) < 0){
    perror("SIOCSIFMTU");
    exit(0);
  }

  printf("Set MTU to %d\n", DEVICE_MTU);
  
  bzero(&ifr, sizeof(ifr));
  memcpy(ifr.ifr_name, dev, strlen(dev));

  if (ioctl(s, SIOCGIFFLAGS, &ifr)){
    perror("SIOCGIFFLAGS");
    exit(0);
  }

  flags = mask = IFF_UP;
  
  if ((ifr.ifr_flags ^flags) & mask) {
    ifr.ifr_flags &= ~mask;
    ifr.ifr_flags |= mask & flags;

    if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0){
      perror("SIOCIFFLAGS");
      exit(0);
    }
  }
  
  printf("Set device flag to UP\n");

  /*
  bzero(aux, 128);

  inet_ntop(AF_INET6, &nid, aux, 128);  
  sprintf(buf, "ip -6 addr add %s/64 dev %s", aux, dev);
  system(buf);

  printf("Set device IPv6 address %s\n", aux);
  */
  close(s);

  return tapfd;
}
