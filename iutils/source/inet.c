#include "inet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>       /* inet(3) functions */
#include <sys/epoll.h> /* epoll function */
#include <fcntl.h>     /* nonblocking */
#include <sys/resource.h> /*setrlimit */
#include <stdio.h>
#include "iutil.h"


bool inet_debug = false;


/*处理接收客户端消息函数*/
void *socket_recv_thread_cb(void * arg)
{
    int recv_num = 0;
    socket_recv_t * recv = arg;
    if (recv == NULL || recv->socket_recv_cb == NULL) {
        return NULL;
    }

    if (inet_debug) {
        ilog_info("[Socket Server] recv thread start");
        ilog_info("client "PX_IP":%d, server "PX_IP":%d", PS_IP(recv->client_ip), recv->client_port, PS_IP(recv->serv_ip), recv->serv_port);
    }


    recv_num = recv->socket_recv_cb(recv);

    if (inet_debug) {
        ilog_info("[Socket Server] recv thread end, num=%d", recv_num);
        ilog_info("client "PX_IP":%d, server "PX_IP":%d", PS_IP(recv->client_ip), recv->client_port, PS_IP(recv->serv_ip), recv->serv_port);
        close(recv->client_fd);
    }

    free(recv);
    return NULL;
}


/**************************************************************************
* 函数名称: socket_tcp_server
* 功能描述:
* 输入参数: uint16_t port
* 输入参数: int * run_flag  NULL或者值为1时，运行
* 输出参数:
* 返 回 值: bool
* 其它说明:
* ------------------------------------------------------------------------
* 修改日期			版本号     修改人          修改内容
* 2017年2月3日		v1.0      何兴诗          创建
**************************************************************************/
bool socket_tcp_server(uint16_t port, int * run_flag, void * recv_cb)
{
    int listenfd;
    struct sockaddr_in serv_addr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        ilog_error("socket error");
        return false;
    }

    /* 避免重启服务器，端口处于TIME_WAIT状态，无法绑定端口的问题 */
    int nopt = true;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&nopt, sizeof(int)) < 0) {
        ilog_error("socket setsockopt error");
		close(listenfd);
        return false;
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        ilog_error("bind error");
        close(listenfd);
        return false;
    }

    if (listen(listenfd, 10000) < 0) {
        ilog_error("listen error");
        close(listenfd);
        return false;
    }



    int client_fd;
    pthread_t recv_tid;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    socket_recv_t * recv = NULL;

    while (run_flag == NULL || *run_flag == true) {
        if ((client_fd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            ilog_error("accept error");
            break;
        }

        recv = malloc(sizeof(socket_recv_t));
        if (recv == NULL) {
            close(client_fd);
            continue;
        }

        recv->socket_type = SOCK_STREAM;
        recv->serv_ip = serv_addr.sin_addr.s_addr;
        recv->serv_port = ntohs(serv_addr.sin_port);
        recv->serv_fd = listenfd;
        recv->socket_recv_cb = recv_cb;
        recv->client_fd = client_fd;
        recv->client_ip = client_addr.sin_addr.s_addr;
        recv->client_port = ntohs(client_addr.sin_port);

        if (pthread_create(&recv_tid, NULL, socket_recv_thread_cb, recv) == -1) {
            ilog_error("socket %d recv thread create error, ", client_fd);
            free(recv);
            close(client_fd);
            break;
        }
        pthread_detach(recv_tid);
    }

    close(listenfd);
    return true;
}

int socket_tcp_client(char * ip, uint16_t port)
{
    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        ilog_error("socket create error");
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) < 0) {
        ilog_error("inet_pton error for %s\n", ip);
        close(sockfd);
        return -1;
    }

    struct timeval timeo = { 0, 200*1000 };
    socklen_t len = sizeof(timeo);
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) < 0) {
        if (inet_debug){
            ilog_error("socket opt set timeout failed!");
        }
        close(sockfd);
        return -1;
    }


    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        if (inet_debug) {
            ilog_error("connect error");
        }
        close(sockfd);
        return -1;
    }

    return sockfd;
}




bool socket_udp_server(uint16_t port, int * run_flag, void * recv_cb)
{
    int listenfd;
    struct sockaddr_in serv_addr;

    if ((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        ilog_error("socket error");
        return false;
    }

    /* 避免重启服务器，端口处于TIME_WAIT状态，无法绑定端口的问题 */
    int nopt = true;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&nopt, sizeof(int)) < 0) {
        ilog_error("socket setsockopt error");
		close(listenfd);
        return false;
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        ilog_error("bind error");
        close(listenfd);
        return false;
    }

    socket_recv_t * recv = NULL;
    recv = malloc(sizeof(socket_recv_t));
    if (recv == NULL) {
        close(listenfd);
        return false;
    }

    recv->socket_type = SOCK_DGRAM;
    recv->serv_ip = serv_addr.sin_addr.s_addr;
    recv->serv_port = ntohs(serv_addr.sin_port);
    recv->serv_fd = listenfd;
    recv->socket_recv_cb = recv_cb;

    while (run_flag == NULL || *run_flag == true) {
        recv->socket_recv_cb(recv);
    }

    free(recv);
    close(listenfd);
    return true;
}


int socket_udp_send(char * ip, uint16_t port, void * data, uint16_t data_len, void * recv_buf, uint16_t recv_max)
{
    int sockfd;
    struct sockaddr_in addr;
    int len = sizeof(addr);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        ilog_error("socket create error");
        return -1;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) < 0) {
        ilog_error("inet_pton error for %s\n", ip);
        close(sockfd);
        return -1;
    }

    if(sendto(sockfd, data, data_len, 0, (struct sockaddr *)&addr, sizeof(addr))<0) {
        ilog_error("socket send error!");
    }

    struct timeval timeo = { 0, 200 * 1000 };
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo)) < 0) {
        if (inet_debug) {
            ilog_error("socket opt set timeout failed!");
        }
		close(sockfd);
        return -1;
    }
    int n = recvfrom(sockfd, recv_buf, recv_max, 0, (struct sockaddr *)&addr, &len);
    
    close(sockfd);
    return n;
}




struct pcap_hdr
{
    uint32_t magic_number;   /* magic number */
    uint16_t version_major;  /* major version number */
    uint16_t version_minor;  /* minor version number */
    int32_t thiszone;        /* GMT to local correction */
    uint32_t sigfigs;        /* accuracy of timestamps */
    uint32_t snaplen;        /* max length of captured packets */
    uint32_t network;        /* data link type */
};
#pragma pack()

struct pcaprec_hdr
{
    uint32_t ts_sec;         /* timestamp seconds */
    uint32_t ts_usec;        /* timestamp microseconds */
    uint32_t incl_len;       /* number of octets of packet saved in file */
    uint32_t orig_len;       /* actual length of packet */
};
#pragma pack()




void inet_pcap_write_header(FILE * fp)
{
    if(fp == NULL) {
        return;
    }
    /* The pcap reader is responsible for figuring out endianness based on the
    * magic number, so the lack of htonX calls here is intentional. */
    struct pcap_hdr ph;
    ph.magic_number = 0xa1b2c3d4;
    ph.version_major = 2;
    ph.version_minor = 4;
    ph.thiszone = 0;
    ph.sigfigs = 0;
    ph.snaplen = 8000;
    ph.network = 1;             /* Ethernet */
    fwrite(&ph, sizeof(ph), 1, fp);
}



void inet_pcap_write(FILE * fp, uint8_t * pkt, uint32_t len, uint64_t ns)
{
    if(fp == NULL || pkt == NULL) {
        return;
    }


    struct pcaprec_hdr prh;
    prh.ts_sec = ns / 1000000000;
    prh.ts_usec = (ns % 1000000000) / 1000;
    prh.incl_len = len;
    prh.orig_len = len;

    fwrite(&prh, sizeof(prh), 1, fp);
    fwrite(pkt, len, 1, fp);

}



void inet_pcap_ut()
{
    char path[256];
    sprintf(path, "/ut.pcap");
    uint8_t pkt_buf[96];
    int i = 0;

    FILE * fp = fopen(path, "a+");
    inet_pcap_write_header(fp);
    printf("%ld\n", iutil_time_ns());
    for(i = 0; i < 100000; i++) {
        inet_pcap_write(fp, pkt_buf, 94, iutil_time_ns());
    }
    printf("%ld\n", iutil_time_ns());
	fclose(fp);

}


/* -------------------------[      测试用例        ]------------------------- */


int socket_tcp_ut_cb(struct socket_recv_s * recv_data)
{
    if (recv_data == NULL) {
        return 0;
    }

    int sockfd = recv_data->client_fd;
    char buf[MAX_LINE];
    int n;

    while (1) {
        memset(buf, 0, MAX_LINE);
        if ((n = recv(sockfd, buf, MAX_LINE, 0)) <= 0) {
            break;
        }
        buf[n] = '\0';
        ilog_info("%s", buf);
    }

    return 0;
}


void socket_tcp_ut()
{
    socket_tcp_server(8000, NULL, socket_tcp_ut_cb);
}



int socket_udp_ut_cb(struct socket_recv_s * recv_data)
{
    if (recv_data == NULL) {
        return 0;
    }

    int sockfd = recv_data->serv_fd;
    char buf[MAX_LINE];
    struct sockaddr_in clientAddr;
    int len = sizeof(clientAddr);
    int n;

    n = recvfrom(sockfd, buf, MAX_LINE, 0, (struct sockaddr*)&clientAddr, &len);
    if (n > 0) {
        buf[n] = 0;
        ilog_info(buf);
        if(sendto(sockfd, "OK", 2, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr))<0) {
            ilog_info("send Error");
        }
    } else {
        ilog_info("recv Error");
    }

    return n;
}


void socket_udp_client_ut(char * str)
{
    char buf[10];
    int n = socket_udp_send("192.168.137.103", 8010, str, strlen(str), buf, 10);
	if(n >= 0){
		buf[n] = '\0';
	}
    ilog_info(buf);
}

void socket_udp_ut()
{
    socket_udp_server(8010, NULL, socket_udp_ut_cb);
}



unsigned char pkt_arp[60] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe0,		/* ........ */
    0x4c, 0x62, 0x00, 0x5e, 0x08, 0x06, 0x00, 0x01,		/* Lb.^.... */
    0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0xe0,		/* ........ */
    0x4c, 0x62, 0x00, 0x5e, 0xc7, 0xc7, 0xc7, 0x01,		/* Lb.^.... */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc7, 0xc7,		/* ........ */
    0xc7, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* ........ */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* ........ */
    0x00, 0x00, 0x00, 0x00,								/* .. */
};

unsigned char pkt_arp2[64] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe0,		/* ........ */
    0x4c, 0x62, 0x00, 0x5e, 0x81, 0x00, 0x00, 0x64, 0x08, 0x06, 0x00, 0x01,		/* Lb.^.... */
    0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0xe0,		/* ........ */
    0x4c, 0x62, 0x00, 0x5e, 0xc8, 0xc8, 0xc8, 0x01,		/* Lb.^.... */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc7, 0xc7,		/* ........ */
    0xc7, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* ........ */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* ........ */
    0x00, 0x00, 0x00, 0x00,								/* .. */
};

char pkt_lacp[] = {
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x02, 0x00, 0xD0, 0xD0, 0x1A, 0xAC, 0xF3, 0x88, 0x09, 0x01, 0x01, /*................*/
    0x01, 0x14, 0x00, 0x00, 0x00, 0xD0, 0xD0, 0x1A, 0xAC, 0xF3, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x47, 0x00, 0x00, 0x00, 0x02, 0x14, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /*G...............*/
    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xA6, 0x60, 0x00, /*..............`.*/
};



char pkt_lacp2[] = {
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x02, 0x00, 0xD0, 0xD0, 0x1A, 0xAC, 0xF3, 0x81, 0x00, 0x00, 0x0e, 0x88, 0x09, 0x01, 0x01, /*................*/
    0x01, 0x14, 0x00, 0x00, 0x00, 0xD0, 0xD0, 0x1A, 0xAC, 0xF3, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x47, 0x00, 0x00, 0x00, 0x02, 0x14, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /*G...............*/
    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*................*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xA6, 0x60, 0x00, /*..............`.*/
};
