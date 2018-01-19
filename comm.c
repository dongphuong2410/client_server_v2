#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "comm.h"
#include "simul_params.h"

int sock;
static int network_status = NW_STATUS_DISCONNECTED;
pthread_mutex_t nw_lock;

static int _check_recv(void);
static int _encrypt(const char *buff);

int nw_connect()
{
    struct sockaddr_in server;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create scoket\n");
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(2424);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connect failed\n");
        return 1;
    }
    network_status = NW_STATUS_OK;
    pthread_mutex_init(&nw_lock, NULL);
    return 0;
}

void nw_destroy()
{
    printf("Destroy network\n");
    pthread_mutex_destroy(&nw_lock);
    close(sock);
}

inline int nw_okay()
{
    return (network_status == NW_STATUS_OK);
}

int nw_write(const char *buff, size_t length)
{
    _encrypt(buff);
    pthread_mutex_lock(&nw_lock);
    int res = send(sock, buff, length, 0);
    pthread_mutex_unlock(&nw_lock);
    if (res < 0)
        printf("Send data error\n");
    return res;
    return 0;
}

int nw_read(char *buff)
{
    int res = _check_recv();
    if (res > 0) {
        return recv(sock, buff, PACKET_LEN, 0);
    }
    else
        return res;
}

void nw_disconnect(void)
{
    network_status = NW_STATUS_DISCONNECTED;
}

static int _check_recv(void)
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(sock, &set);
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    return select(sock + 1, &set, NULL, NULL, &timeout);

}

static int _encrypt(const char *buff)
{
    //Create a fake busy task
    //usleep(ENCRYPT_SLEEP);
    int i = 0;
    while (i < ENCRYPT_LOOP) {
        i++;
    }
}
