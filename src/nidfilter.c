#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/netfilter.h>
#include <libipq/libipq.h>
#include <pthread.h>

#include "nid.h"

#define MAXBUFSIZE 1500

extern int sp_nf2ph[2];

/*
 * Function to detect errors and close the ip_queue
 */
static void die(struct ipq_handle *h)
{
  ipq_perror("passer ");
  ipq_destroy_handle(h);
  exit(1);
}

/* Main thread which receives packets from the ip_queue.
 * It reads packets and send them to the Packet Handler (PH)
 * through a socket unix.
 */

void *thread_nidfilter(void)
{
  static int status;
  static u_char buf[MAXBUFSIZE];
  struct ipq_handle *h;
  struct iphdr *ip;  
  ipq_packet_msg_t *m;
  
  h = ipq_create_handle(0, PF_INET);
  if (!h)
    die(h);
  
  /* Set the capture mode: IPQ_COPY_PACKET captures the entire packet */
  status = ipq_set_mode(h, IPQ_COPY_PACKET, MAXBUFSIZE);
  if (status < 0)
    die(h);
  
  while (1){
    ipq_read(h, buf, MAXBUFSIZE, 0);
        
    switch (ipq_message_type(buf)) {
      /* Types of packets. We are interested in IP packets */
    case IPQM_PACKET: {
      /* IP packets are here! */
      m = ipq_get_packet(buf);
      ip = (struct iphdr *) m->payload;
      
      /* Send to Packet Handler Output */      
      write(sp_nf2ph[1], ip, ntohs(ip->tot_len));
      
      /* Drop original packet */
      ipq_set_verdict(h, m->packet_id, NF_DROP, ntohs(ip->tot_len), (u_char *)ip);
    }            
    }
  }
  
  ipq_destroy_handle(h);
  free (buf);
  
  return 0;
}
