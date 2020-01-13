#ifndef INET_H
#define INET_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct socket_recv_s
{
    int socket_type;
    uint32_t serv_ip;
    uint16_t serv_port;
    int serv_fd;

    int client_fd;
    uint32_t client_ip;
    uint16_t client_port;

    int(*socket_recv_cb)(struct socket_recv_s * recv_data);
}socket_recv_t;

bool socket_tcp_server(uint16_t port, int * run_flag, void * recv_cb);
int socket_tcp_client(char * ip, uint16_t port);
void inet_pcap_write_header(FILE * fp);
void inet_pcap_write(FILE * fp, uint8_t * pkt, uint32_t len, uint64_t ns);

#endif





