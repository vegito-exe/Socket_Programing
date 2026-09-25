#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define MAX_BUF_SIZE 2048
#define BACKLOG 20

int verify_and_parse_ip(char *ip_argument);
int verify_and_parse_port(char *port_argument);
int tcp_socket_connect(int ip, int port);
int tcp_socket_listen(int port);

#endif /* FUNCTIONS_H */
