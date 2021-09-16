Project Description
==============================

This project allows host mobility without breaking the connectivity. This is done by creating a virtual interfaces (similar to VPN end-points) and binding the connection to those interface. This prototype captures the incoming packages from the Kernel netfilter library (through the ip_queue interface), adds it to the network stack and it is handled by the NIDA prototype. It then can forward it to any destinations, change the packages, etc.

More information about the prototype can be found here: [Article at Research Gate](https://www.researchgate.net/publication/221325114_A_next_generation_internet_architecture_for_mobility_and_multi-homing_support)

Project Requirements
------------
- autoconf
- automake
- gcc
- libxml2
- libxml2-dev
- iptables
- iptables-dev

Configuration Steps
------------
1. Configure the configuration file
nid.conf (use sample nid.conf.example as reference)

2. Load "ip_queue" lib
modprobe ip_queue

3. Load virtual interface library
modprobe tun

4. Install iptables filter
iptables -A OUTPUT -o nid0 -s 1.0.0.0/8 -d ! localhost -j QUEUE

5. Export the library path
EXPORT LD_LIBRARY_PATH=.

6. Execute nid daemon
./nidd
