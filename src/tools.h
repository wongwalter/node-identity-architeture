void set_skt_nonblock(const int s);
int get_skt_sndbuf(const int socket);
int get_skt_rcvbuf(const int socket);
void set_skt_sndbuf(const int socket, const int sndbuf);
void set_skt_rcvbuf(const int socket, const int sndbuf);
void set_skt_broadcast(const int s);
void set_iphdr_included(const int s);
void set_skt_reuseaddr(const int s);
int new_socket();
