#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "dns.h"
#include "fqdn_table.h"
#include "nid.h"
#include "nid_mapper.h"

#define PROXY_PORT 		53
#define SERVER_PORT 	54
#define BUFFER_LENGTH 	512
#define NIDCHAR_MAXLENGTH  39
// NIDCHAR example: 2006:452A:5341:DE01:938F:FEEC:DCA9:0151,2006:AB45:42A1:A517:FEEC:726A:1029:6AB5

char server_address[16];

int modify_dns_query(unsigned char *buf, unsigned char *fqdn, uint16_t *queryType) {
   unsigned char queryName[DNS_MAXNAMELENGTH+1];
   uint16_t txt=16;
   strcpy(queryName, (unsigned char *) (buf + sizeof(struct dnshdr)));
   strcpy(fqdn, queryName);

   memcpy(queryType, (buf + 1 + sizeof(struct dnshdr) 
	                      + strlen(queryName)), sizeof(uint16_t));
   *queryType=ntohs(*queryType);

   if ((*queryType == 1) || (*queryType == 28)) {
      txt=htons(txt);
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName)), &txt, sizeof(txt));
   };
   return 0; 
};


int analyse_dns_response(unsigned char *buf, unsigned char *queryName, 
						 unsigned char *answerAns, unsigned char *nsNs) {
   struct dnshdr *hdr;
   uint16_t answerDataLength;   
   uint16_t nsDataLength;
   uint16_t b=15;

   bzero(queryName,DNS_MAXNAMELENGTH+1);
   bzero(answerAns,DNS_MAXNAMELENGTH+1);
   bzero(nsNs,DNS_MAXNAMELENGTH+1);

   hdr = (struct dnshdr *) buf;
	  
   strcpy(queryName, (unsigned char *) (buf + sizeof(struct dnshdr)));
 
   if ((ntohs(hdr->flags) & b) == 0) { 


      memcpy(&answerDataLength, (buf + 1 + sizeof(struct dnshdr) 
	                                 + strlen(queryName) 
	                                 + 5*sizeof(uint16_t) 
	                                 + sizeof(uint32_t)), sizeof(uint16_t));
      answerDataLength=ntohs(answerDataLength);

      memcpy(answerAns, (buf + sizeof(struct dnshdr) 
	                         + strlen(queryName) + 2
	                         + 6*sizeof(uint16_t)
	                         + sizeof(uint32_t)), answerDataLength-1);

      memcpy(&nsDataLength, (buf + 1 +sizeof(struct dnshdr) 
	                             + strlen(queryName)                                      
	                             + 6*sizeof(uint16_t) 
	                             + sizeof(uint32_t)
	                             + answerDataLength
	                             + 3*sizeof(uint16_t)
	                             + sizeof(uint32_t)), sizeof(uint16_t));
      nsDataLength = ntohs(nsDataLength);
   
      memcpy(nsNs, (buf + sizeof(struct dnshdr) 
   	                    + strlen(queryName) + 2
	                    + 6*sizeof(uint16_t) 
	                    + sizeof(uint32_t)
	                    + answerDataLength
	                    + 3*sizeof(uint16_t)
	                    + sizeof(uint32_t)
	                    + sizeof(uint16_t)), nsDataLength-1);
      return 0; 
   }
   else return -1;
}
						 
int get_nids(unsigned char *nidtuple, 
               unsigned char *nnid, unsigned char *nnidr) {
               	
   bzero(nnid, NIDCHAR_MAXLENGTH+1);
   bzero(nnidr,NIDCHAR_MAXLENGTH+1);
   memcpy(nnid, nidtuple, NIDCHAR_MAXLENGTH);
//   printf("NID = %s\n", nnid);
   memcpy(nnidr, nidtuple + NIDCHAR_MAXLENGTH + 1, NIDCHAR_MAXLENGTH);
//   printf("NID-R = %s\n", nnidr);
   return 0;
};


int build_dns_response(unsigned char *buf, unsigned int nid32addr, 
                                           unsigned char nidaddr[16], 
                                           uint16_t qType) {

   unsigned char queryName[DNS_MAXNAMELENGTH+1], answerAns[DNS_MAXNAMELENGTH+1], 
                 nsNs[DNS_MAXNAMELENGTH+1];
   struct dnshdr *hdr;
   uint16_t queryType;
   uint16_t answerDataLength;
   uint16_t nsName;
   uint32_t nsTTL;
   uint16_t nsType, nsClass, nsDataLength;
   uint16_t a=1, aaaa=28, dataLength=4, b=15;
   struct authoritativeNS nsResponse;

   bzero(queryName,DNS_MAXNAMELENGTH+1);
   bzero(answerAns,DNS_MAXNAMELENGTH+1);
   bzero(nsNs,DNS_MAXNAMELENGTH+1);

   hdr = (struct dnshdr *) buf;

   strcpy(queryName, (unsigned char *) (buf + sizeof(struct dnshdr)));
 
   memcpy(&queryType, (buf + 1 + sizeof(struct dnshdr) 
	                      + strlen(queryName)), sizeof(uint16_t));
   queryType=ntohs(queryType);

   memcpy(&answerDataLength, (buf + 1 + sizeof(struct dnshdr) 
	                              + strlen(queryName) 
	                              + 5*sizeof(uint16_t) 
	                              + sizeof(uint32_t)), sizeof(uint16_t));
   answerDataLength=ntohs(answerDataLength);
   
   memcpy(&nsName, (buf + 1 + sizeof(struct dnshdr) 
	                    + strlen(queryName)
	                    + 6*sizeof(uint16_t) 
	                    + sizeof(uint32_t)
	                    + answerDataLength), sizeof(uint16_t));
   nsName=ntohs(nsName);
   
   memcpy(&nsType, (buf + 1 + sizeof(struct dnshdr) 
                        + strlen(queryName)
	                    + 6*sizeof(uint16_t) 
                        + sizeof(uint32_t)
                        + answerDataLength
                        + sizeof(uint16_t)), sizeof(uint16_t));
   nsType=ntohs(nsType);    
   
   memcpy(&nsClass, (buf + 1 + sizeof(struct dnshdr) 
                         + strlen(queryName)
                         + 6*sizeof(uint16_t) 
 	                     + sizeof(uint32_t)
                         + answerDataLength
                         + 2*sizeof(uint16_t)), sizeof(uint16_t));
   nsClass=ntohs(nsClass);
   
   memcpy(&nsTTL, (buf + 1 + sizeof(struct dnshdr) 
	                   + strlen(queryName)
	                   + 6*sizeof(uint16_t) 
	                   + sizeof(uint32_t)
	                   + answerDataLength
	                   + 3*sizeof(uint16_t)), sizeof(uint32_t));
   nsTTL=ntohl(nsTTL);

   memcpy(&nsDataLength, (buf + 1 + sizeof(struct dnshdr) 
	                          + strlen(queryName)                                      
	                          + 6*sizeof(uint16_t) 
	                          + sizeof(uint32_t)
	                          + answerDataLength
	                          + 3*sizeof(uint16_t)
	                          + sizeof(uint32_t)), sizeof(uint16_t));
   nsDataLength = ntohs(nsDataLength);
   
   memcpy(nsNs, (buf + 1 + sizeof(struct dnshdr) 
   	                 + strlen(queryName)
	                 + 6*sizeof(uint16_t) 
	                 + sizeof(uint32_t)
	                 + answerDataLength
	                 + 3*sizeof(uint16_t)
	                 + sizeof(uint32_t)
	                 + sizeof(uint16_t)), nsDataLength);
   
   if ((queryType == 16) && (qType == 1)) {
      a=1;
      a=htons(a);
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	                  + strlen(queryName)), &a, sizeof(a));
   }; 

   if ((queryType == 16) && (qType == 28)) {
      aaaa=28;
      aaaa=htons(aaaa);
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	                  + strlen(queryName)), &aaaa, sizeof(aaaa));
   }; 

   if ((queryType == 16) && ((ntohs(hdr->flags) & b) == 0)) { 

      if (qType == 1) { 
         a=1;
         a=htons(a);
         memcpy((buf + 1 + sizeof(struct dnshdr) 
	                     + strlen(queryName) 
	                     + 3*sizeof(uint16_t)), &a, sizeof(a));

         memcpy((buf + 1 + sizeof(struct dnshdr) 
	                     + strlen(queryName) 
	                     + 6*sizeof(uint16_t)
	                     + sizeof(uint32_t)), &nid32addr, 4);

         dataLength=4;
         dataLength=htons(dataLength);
         memcpy((buf + 1 + sizeof(struct dnshdr) 
	                     + strlen(queryName) 
	                     + 5*sizeof(uint16_t) 
	                     + sizeof(uint32_t)), &dataLength, sizeof(dataLength));
      }
      
      if (qType == 28) { 
         aaaa=28;
         aaaa=htons(aaaa);
         memcpy((buf + 1 + sizeof(struct dnshdr) 
	                     + strlen(queryName) 
	                     + 3*sizeof(uint16_t)), &aaaa, sizeof(aaaa));

         memcpy((buf + 1 + sizeof(struct dnshdr) 
	                     + strlen(queryName) 
	                     + 6*sizeof(uint16_t)
	                     + sizeof(uint32_t)), nidaddr, 16);

         dataLength=16;
         dataLength=htons(dataLength);
         memcpy((buf + 1 + sizeof(struct dnshdr) 
	                     + strlen(queryName) 
	                     + 5*sizeof(uint16_t) 
	                     + sizeof(uint32_t)), &dataLength, sizeof(dataLength));
      }     
            
      
      memcpy(&answerDataLength, (buf + 1 + sizeof(struct dnshdr) 
	                              + strlen(queryName) 
	                              + 5*sizeof(uint16_t) 
	                              + sizeof(uint32_t)), sizeof(uint16_t));
      answerDataLength=ntohs(answerDataLength);
      
      nsResponse.name = htons(nsName);
      nsResponse.type = htons(nsType);
      nsResponse.class = htons(nsClass);
      nsResponse.ttl = htonl(nsTTL);
      nsResponse.dataLength = htons(nsDataLength);
 
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	                     + strlen(queryName)
	                     + 6*sizeof(uint16_t) 
	                     + sizeof(uint32_t)
	                     + answerDataLength), &nsResponse.name, sizeof(uint16_t));
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	                     + strlen(queryName)
	                     + 6*sizeof(uint16_t) 
	                     + sizeof(uint32_t)
	                     + answerDataLength
	                     + sizeof(uint16_t)), &nsResponse.type, sizeof(uint16_t));
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	                      + strlen(queryName)
	                      + 6*sizeof(uint16_t) 
	                      + sizeof(uint32_t)
	                      + answerDataLength
	                      + 2*sizeof(uint16_t)), &nsResponse.class, sizeof(uint16_t));
	  memcpy((buf + 1 +sizeof(struct dnshdr) 
	                   + strlen(queryName)
	                   + 6*sizeof(uint16_t) 
	                   + sizeof(uint32_t)
	                   + answerDataLength
	                   + 3*sizeof(uint16_t)), &nsResponse.ttl, sizeof(uint32_t));
	  memcpy((buf + 1 +sizeof(struct dnshdr) 
	                          + strlen(queryName)                                      
	                          + 6*sizeof(uint16_t) 
	                          + sizeof(uint32_t)
	                          + answerDataLength
	                          + 3*sizeof(uint16_t)
	                          + sizeof(uint32_t)), &nsResponse.dataLength, sizeof(uint16_t));	                   
	  bzero((buf + 1 + sizeof(struct dnshdr) 
   	                    + strlen(queryName)
	                    + 6*sizeof(uint16_t) 
	                    + sizeof(uint32_t)
	                    + answerDataLength
	                    + 3*sizeof(uint16_t)
	                    + sizeof(uint32_t)
	                    + sizeof(uint16_t)), DNS_MAXNAMELENGTH+1);
      memcpy((buf + 1 + sizeof(struct dnshdr) 
   	                    + strlen(queryName)
	                    + 6*sizeof(uint16_t) 
	                    + sizeof(uint32_t)
	                    + answerDataLength
	                    + 3*sizeof(uint16_t)
	                    + sizeof(uint32_t)
	                    + sizeof(uint16_t)), nsNs, nsDataLength);
   } 
   else return -1;
   return 0; 
};


int build_dns_response_with_fqdn_table(unsigned char *buf, 
                                       unsigned int nid32addr, 
                                       unsigned char nidaddr[16], 
                                       uint16_t qType) {
   unsigned char queryName[DNS_MAXNAMELENGTH+1];
   uint16_t a=1, aaaa=28, name=49164, dl=4, class=1,
            flags=34176, questions=1, ansRR=1, authRR=1, addRR=0,
            nsType=2;
   uint16_t transaction_id;
   uint32_t ttl=60;


   
   memcpy(&transaction_id, buf, sizeof(uint16_t));

   bzero(queryName,DNS_MAXNAMELENGTH+1);
   strcpy(queryName, (unsigned char *) (buf + sizeof(struct dnshdr)));
 
   bzero(buf,BUFFER_LENGTH);

   memcpy(buf, &transaction_id, sizeof(uint16_t));      

   flags=htons(flags); 
   memcpy((buf + sizeof(uint16_t)), &flags, sizeof(uint16_t));

   questions=htons(questions); 
   memcpy((buf + 2*sizeof(uint16_t)), &questions, sizeof(uint16_t));

   ansRR=htons(ansRR); 
   memcpy((buf + 3*sizeof(uint16_t)), &ansRR, sizeof(uint16_t));

   authRR=htons(authRR); 
   memcpy((buf + 4*sizeof(uint16_t)), &authRR, sizeof(uint16_t));

   addRR=htons(addRR); 
   memcpy((buf + 5*sizeof(uint16_t)), &addRR, sizeof(uint16_t));

   memcpy((buf + sizeof(struct dnshdr)), queryName,
           strlen(queryName));

   if (qType==1) {
      a=htons(a);
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName)), &a, sizeof(uint16_t));
   }
   else if (qType==28) {
           aaaa=htons(aaaa);
           memcpy((buf + 1 + sizeof(struct dnshdr) 
	                   + strlen(queryName)), &aaaa, sizeof(uint16_t));
   }   	       
   
   class=htons(class);
   memcpy((buf + 1 + sizeof(struct dnshdr) 
	           + strlen(queryName) 
	           + sizeof(uint16_t)), &class, sizeof(uint16_t));	

   name=htons(name); 
   memcpy((buf + 1 + sizeof(struct dnshdr) 
	           + strlen(queryName)
	           + 2*sizeof(uint16_t)), &name, sizeof(uint16_t));

   if (qType == 1) {
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName) 
	              + 3*sizeof(uint16_t)), &a, sizeof(uint16_t));
   } 
   else if (qType == 28) {
           memcpy((buf + 1 + sizeof(struct dnshdr) 
	                   + strlen(queryName) 
	                   + 3*sizeof(uint16_t)), &aaaa, sizeof(uint16_t));
   }   	 
  	           
   memcpy((buf + 1 + sizeof(struct dnshdr) 
	           + strlen(queryName) 
	           + 4*sizeof(uint16_t)), &class, sizeof(uint16_t));

   ttl=htonl(ttl);
   
   memcpy((buf + 1 + sizeof(struct dnshdr) 
	           + strlen(queryName) 
	           + 5*sizeof(uint16_t)), &ttl, sizeof(uint32_t));

   if (qType == 1) {
      dl=4;
   	  dl=htons(dl);
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName) 
	              + 5*sizeof(uint16_t))
	              + sizeof(uint32_t), &dl, sizeof(uint16_t)); 

      memcpy((buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName) 
	              + 6*sizeof(uint16_t)
	              + sizeof(uint32_t)), &nid32addr, 4); 
   } 
   else if (qType == 28) {

      dl=16;
   	  dl=htons(dl);
      memcpy((buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName) 
	              + 5*sizeof(uint16_t))
	              + sizeof(uint32_t), &dl, sizeof(uint16_t)); 

      memcpy((buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName) 
	              + 6*sizeof(uint16_t)
	              + sizeof(uint32_t)), nidaddr, 16);      
   }   	
   else return -1;

   nsType=htons(nsType);
   memcpy(buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName)
	              + 6*sizeof(uint16_t) 
	              + sizeof(uint32_t)
	              + (qType==1?4:16)
	              + 1, &nsType, sizeof(uint16_t));

   memcpy(buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName)
	              + 6*sizeof(uint16_t) 
	              + sizeof(uint32_t)
	              + (qType==1?4:16)
	              + 1 + sizeof(uint16_t), &class, sizeof(uint16_t));

   memcpy(buf + 1 + sizeof(struct dnshdr) 
	              + strlen(queryName)
	              + 6*sizeof(uint16_t) 
	              + sizeof(uint32_t)
	              + (qType==1?4:16)
	              + 1 + 2*sizeof(uint16_t), &ttl, sizeof(uint32_t));

   return 0; 
};


void update_dns_server_address(NID niddnsserver) {
   NID32 nid32dnsserver;
   unsigned int last24bit=0;
   int control;
   for (control=13;control<16;control++) {
       last24bit = last24bit * 256;
       last24bit = last24bit + niddnsserver.in6_u.u6_addr8[control];
//     para colocar 1 na frente devo somar 16777216 = 2^24
   };    
   nid32dnsserver = ntohl(last24bit + 16777216);
   inet_ntop(AF_INET,&(nid32dnsserver),server_address,sizeof(server_address));
   printf("Rodolfo!!\n");
}

void *thread_dnshandler(void) {
   int sd=-1, dnssd=-1;
   int rc, st, on=1;
   int ret=-1;
   unsigned char fqdn[DNS_MAXNAMELENGTH+1];
   unsigned char nid32str[INET_ADDRSTRLEN];
   unsigned char nid128str[INET6_ADDRSTRLEN],
                 nidr128str[INET6_ADDRSTRLEN];
   unsigned char buffer[BUFFER_LENGTH];
   unsigned char qName[DNS_MAXNAMELENGTH+1], 
                 ansAns[DNS_MAXNAMELENGTH+1], 
			     nNs[DNS_MAXNAMELENGTH+1];
   unsigned char nid[NIDCHAR_MAXLENGTH+1], 
                 nidr[NIDCHAR_MAXLENGTH+1];
   struct sockaddr_in proxyserveraddr, clientaddr, 
                      serverdnsaddr, proxyclientaddr;
   struct sockaddr_in6 nidaddr, nidraddr;
   struct sockaddr_in nid32addr, nidr32addr;
   int addrlen=sizeof(struct sockaddr_in);   
   unsigned char *nids_returned;
   uint16_t queryType;

   init_fqdn_table();
   do
   {

//        printf("serverdnsaddr %s\n", nid322char(serverdnsaddr.sin_addr.s_addr));
//        printf("string dns server %s\n", server_address);


      if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
         perror("socket() failed");
 //        exit(-1);
         break;
      }

      if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
         perror("setsockopt(SO_REUSEADDR) failed");
 //        exit(-1);       
         break;
      }

      bzero(&proxyserveraddr, sizeof(proxyserveraddr));
      proxyserveraddr.sin_family = AF_INET;
      proxyserveraddr.sin_port   = htons(PROXY_PORT);
      proxyserveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
      
      if (bind(sd,(struct sockaddr *) &proxyserveraddr, sizeof(struct sockaddr_in6)) < 0) {
         perror("dnsproxy bind() failed");
//         exit(-1);         
         break;
      }

      rc = recvfrom(sd,buffer,sizeof(buffer),0,(struct sockaddr *) &clientaddr,&addrlen);
      printf("%d bytes received\n", rc);

      if (rc < 0) {
         perror("dnsproxy recv() failed");
//         exit(-1);         
         break;
      }

      modify_dns_query(buffer, fqdn, &queryType);

      if ((nids_returned = (unsigned char *) getNIDsbyName(fqdn)) != NULL) {
//         printf("Recuperado da tabela hash\n");
//         printf("NIDs = %s\n", nids_returned);
       	 get_nids(nids_returned, nid, nidr);        
         
         nidaddr.sin6_family = AF_INET6;
	     if (inet_pton(AF_INET6, nid, &(nidaddr.sin6_addr))) {
//	        printf("NID valido, convertido com sucesso!\n");
	     } 
	     else {
	  	    printf("NID invalido.\n");
	  	    break;
	     };

         nidraddr.sin6_family = AF_INET6;	  
	     if (inet_pton(AF_INET6, nidr, &(nidraddr.sin6_addr))) {
//	  	    printf("NID-R valido, convertido com sucesso!\n");
	     } 
	     else {
	  	    printf("NID-R invalido.\n");
	  	    break;
	     };	  

         inet_ntop(AF_INET6,&(nidaddr.sin6_addr),nid128str,sizeof(nid128str));
//         printf("NID address is %s\n", nid128str);
         inet_ntop(AF_INET6,&nidraddr.sin6_addr,nidr128str,sizeof(nidr128str));  
//         printf("NID-R address is %s\n", nidr128str);
     
         unsigned int last24bit=0;
         int control;
         for (control=13;control<16;control++) {
      	     last24bit = last24bit * 256;
      	     last24bit = last24bit + nidaddr.sin6_addr.in6_u.u6_addr8[control];
//         para colocar 1 na frente devo somar 16777216 = 2^24
         };
            
         nid32addr.sin_addr.s_addr = ntohl(last24bit + 16777216);
         inet_ntop(AF_INET,&(nid32addr.sin_addr),nid32str,sizeof(nid32str));
//         printf("NID32 address is %s\n", nid32str);  
//         printf("Tamanho do Buffer = %d\n",sizeof(buffer));  

         newNMEntry(nidaddr.sin6_addr, nidraddr.sin6_addr);
         newNMEntry32(nid32addr.sin_addr.s_addr, nidaddr.sin6_addr,
                                          nidraddr.sin6_addr);
       
         build_dns_response_with_fqdn_table(buffer, nid32addr.sin_addr.s_addr, 
                                            nidaddr.sin6_addr.in6_u.u6_addr8, queryType);     
         rc = BUFFER_LENGTH;
      } 
      else {

      	if ((dnssd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      	   perror("dns socket() failed");
  //         exit(-1);
           break;
        }

    
        bzero(&serverdnsaddr, sizeof(serverdnsaddr));

        serverdnsaddr.sin_family = AF_INET;

        serverdnsaddr.sin_port   = htons(SERVER_PORT);

        serverdnsaddr.sin_addr.s_addr = inet_addr(server_address);


	//        printf("serverdnsaddr %s\n", nid322char(serverdnsaddr.sin_addr.s_addr));
        //printf("string dns server %s\n", server_address);
//	printf("comentario do rodolfo %s\n", nid322char(serverdnsaddr.sin_addr.s_addr));
        st = sendto(dnssd,buffer, rc, 0, 
                   (struct sockaddr *) &serverdnsaddr, 
                   sizeof(serverdnsaddr));
   
//        fflush(stdout);
        printf("%d bytes of data were sent\n", st);

        if (st < 0) {
           perror("dnsserver sendto() failed");
           //exit(-1);         
           break;
        }

//        printf("Antes do rcfrom\n");
                fflush(stdout);
        rc = recvfrom(dnssd, buffer, sizeof(buffer), 0, 
                     (struct sockaddr *)&proxyclientaddr, &addrlen);
//        printf("Depois do rcfrom\n");
//                fflush(stdout);
        if (rc < 0) {
           perror("dnsserver recv() failed");
//           exit(-1);         
           break;
        }

        ret = analyse_dns_response(buffer, qName, ansAns, nNs);

        if (ret == 0) {
 
           if (newFQDNTEntry(fqdn, ansAns)) {
 //   	        printf("Inserindo NIDs na tabela hash\n");
           }
      	   else {
      	   	 printf("NIDs nao form inseridos na FQDN table\n");
      	   }
           get_nids(ansAns, nid, nidr);

           nidaddr.sin6_family = AF_INET6;
	       if (inet_pton(AF_INET6, nid, &(nidaddr.sin6_addr))) {
//            printf("NID valido, convertido com sucesso!\n");
	       } 
	       else {
	          printf("NID invalido.\n");
	          break;
	       };

           nidraddr.sin6_family = AF_INET6;	  
	       if (inet_pton(AF_INET6, nidr, &(nidraddr.sin6_addr))) {
//	          printf("NID-R valido, convertido com sucesso!\n");
	       } 
	       else {
	            printf("NID-R invalido.\n");
	  	        break;
	       };	  

           inet_ntop(AF_INET6,&(nidaddr.sin6_addr),nid128str,sizeof(nid128str));
//           printf("NID address is %s\n", nid128str);
           inet_ntop(AF_INET6,&nidraddr.sin6_addr,nidr128str,sizeof(nidr128str));  
//           printf("NID-R address is %s\n", nidr128str);
     
           unsigned int last24bit=0;
           int control;
           for (control=13;control<16;control++) {
               last24bit = last24bit * 256;
      	       last24bit = last24bit + nidaddr.sin6_addr.in6_u.u6_addr8[control];
//               para colocar 1 na frente somar  16777216
           };
            
           nid32addr.sin_addr.s_addr = ntohl(last24bit + 16777216);
           inet_ntop(AF_INET,&(nid32addr.sin_addr),nid32str,sizeof(nid32str));
//           printf("NID32 address is %s\n", nid32str);    

           newNMEntry(nidaddr.sin6_addr, nidraddr.sin6_addr);
           newNMEntry32(nid32addr.sin_addr.s_addr, nidaddr.sin6_addr,
                                            nidraddr.sin6_addr);
         
           build_dns_response(buffer, nid32addr.sin_addr.s_addr, 
                              nidaddr.sin6_addr.in6_u.u6_addr8, queryType);
        };
      };
     
      st = sendto(sd, buffer, rc, 0, 
                  (struct sockaddr *)&clientaddr, sizeof(clientaddr));
                  
      if (st < 0) {
         perror("dnsproxy sendto() failed");
//         exit(-1);         
         break;
      }
       
   } while (1);

   if (sd != -1) close(sd);
   if (dnssd != -1) close(dnssd);
}
