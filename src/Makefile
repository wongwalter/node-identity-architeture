CC=gcc
CFLAGS=-O2 -fPIC -march=pentium4 -mfpmath=sse,387 -pipe -fomit-frame-pointer
LDFLAGS=-lipq -lpthread -lxml2 -ldl -Wall
XDR=rpcgen
XDRCONFIG=rpcif.x
RPC_OBJ=dhtc.o rpcif_client.o rpcif_xdr.o
DYNLINKFLAGS=-shared -nostdlib
LIB_FILES=libnf.so libph.so libnr.so libmo.so libco.so librvs.so libgw.so libdhcp.so \
libdht.so libdnsh.so
INCLUDE=-I/usr/include/libxml2
NIDD_OBJ=connection_mapper.o tools.o nid_routing.o hash.o hash32.o configuration.o nidd.o \
tuntap.o llist.o nid_mapper.o dhtc.o rvsc.c rvs_queue_table.o fqdn_hash.o fqdn_table.o \
dnshandler.o dht_queue_table.o $(RPC_OBJ)

all: dht nidd rvs conecta

conecta: conecta.o
	$(CC) -o $@ $? $(LDFLAGS) $(CFLAGS)

dht: rpcstub compile

rpcstub:
	$(XDR) -h $(XDRCONFIG) > rpcif.h
	$(XDR) -l $(XDRCONFIG) > rpcif_client.c
	$(XDR) -c $(XDRCONFIG) > rpcif_xdr.c
compile:
	$(CC) $(CFLAGS) -c rpcif_client.c
	$(CC) $(CFLAGS) -c rpcif_xdr.c
	$(CC) $(CFLAGS) -c dhtc.c

nidd: ${NIDD_OBJ} ${LIB_FILES}
	$(CC) -o $@ $? $(LDFLAGS) $(CFLAGS)

rvs: connection_mapper.o configuration.o dnshandler.o fqdn_hash.o fqdn_table.o \
nid_mapper.o hash32.o hash.o rvs_queue_table.o hash.o llist.o rvs.o tools.o rvsc.o \
packetHandler.o
	$(CC) -o $@ $? $(LDFLAGS) $(CFLAGS)

libnf.so: nidfilter.o
	$(CC) $(DYNLINKFLAGS) -o $@ $< $(LDFLAGS) $(CFLAGS)

libph.so: packetHandler.o configuration.o nid_mapper.o hash.o hash32.o
	$(CC) $(DYNLINKFLAGS) -o $@ $? $(LDFLAGS) $(CFLAGS)

libnr.so: nid_routing.o configuration.o connection_mapper.o hash.o hash32.o
	$(CC) $(DYNLINKFLAGS) -o $@ $? $(LDFLAGS) $(CFLAGS)

libmo.so: hash.o llist.o connection_mapper.o configuration.o mobility.o
	$(CC) $(DYNLINKFLAGS) -o $@ $? $(LDFLAGS) $(CFLAGS)

libco.so: configuration.o connection_mapper.o control.o rvs_queue_table.o hash.o hash32.o
	$(CC) $(DYNLINKFLAGS) -o $@ $? $(LDFLAGS) $(CFLAGS)

librvs.so: configuration.o connection_mapper.o rvs_queue_table.o rvsc.o
	$(CC) $(DYNLINKFLAGS) -o $@ $? $(LDFLAGS) $(CFLAGS)

libgw.so: nid_routing.o hash.o configuration.o connection_mapper.o gwc.o
	$(CC) $(DYNLINKFLAGS) -o $@ $? $(LDFLAGS) $(CFLAGS)

libdhcp.so: configuration.o dhcp.o
	$(CC) $(DYNLINKFLAGS) -o $@ $? $(LDFLAGS) $(CFLAGS)

libdht.so: dhtc.o
	$(CC) $(DYNLINKFLAGS) -o $@ $? $(LDFLAGS) $(CFLAGS)

libdnsh.so: configuration.o hash.o hash32.o fqdn_hash.o fqdn_table.o nid_mapper.o \
dnshandler.o 
	$(CC) $(DYNLINKFLAGS) -o $@ $? $(LDFLAGS) $(CFLAGS)

%.o: %.c
	$(CC) $(INCLUDE) -c $< $(CFLAGS)

clean:
	rm -fr *.o nidd rvs conecta ${LIB_FILES} 
	rm -f rpcif.h rpcif_client.c rpcif_xdr.c

