#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdarg.h>

#define CMD_LEN 1024
#define PACKET_LEN 2000

static void thread_init(void);
static void *run(void *data);
static void *connection_handler(void *data);
static void mgrlog(const char *message, ...);
static void print_help(void);

int quit = 0;
FILE *logfd;
pthread_t network_thread;
int reply_zzz = 1;

int main(int argc, char **argv)
{
    logfd = fopen("manager.log", "w+");

    /* Create task threads */
    thread_init();

    /* User command */
    char command[CMD_LEN];
    printf("=== Manager simulation ===\n");
    printf("Type 'help' for command list\n");
    while (1) {
        printf("\nmgr>");
        scanf("%s", command);
        if (!strncmp(command, "quit", CMD_LEN)) {
            quit = 1;
            printf("Bye!\n");
            break;
        }
        else if (!strncmp(command, "no_zzz", CMD_LEN)) {
            reply_zzz = 0;
        }
        else if (!strncmp(command, "zzz", CMD_LEN)) {
            reply_zzz = 1;
        }
        else if (!strncmp(command, "help", CMD_LEN)) {
            print_help();
        }
        else {
            printf("Unknown command : %s", command);
        }
    }

    pthread_join(network_thread, NULL);
    fclose(logfd);
    return 0;
}

static void *connection_handler(void *data)
{
    char buff[PACKET_LEN];
    int *client_socket = (int *)data;
    unsigned long long pkt_cnt = 0;
    while (!quit) {
        int bytes = recv(*client_socket, buff, PACKET_LEN, 0);
        if (bytes > 0) {
            char *ptr = buff;
            while (ptr - buff < bytes) {
                //mgrlog("Receive : %c%c%c\n", ptr[0], ptr[1], ptr[2]);
                if (!strncmp(ptr, "ACQ", 3)) {
                    send(*client_socket, "ACK", 3, 0);
                    mgrlog("Send : %s\n", "ACK");
                }
                else if (!strncmp(ptr, "ZZZ", 3)) {
                    if (reply_zzz) {
                        send(*client_socket, "ZZZ", 3, 0);
                        mgrlog("Send : %s\n", "ZZZ");
                    }
                }
                else if (!strncmp(ptr, "EVT", 3)) {
                    pkt_cnt++;
                }
                ptr += 3;
            }
        }
        else {
            mgrlog("Connection close ..\n");
            mgrlog("Events received : %llu\n", pkt_cnt);
            break;
        }
    }
}

static void thread_init(void)
{
    if (pthread_create(&network_thread, NULL, run, NULL))
    {
        printf("Error creating thread\n");
    }
}

static void *run(void *data)
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        mgrlog("ERROR could not create socket\n");
    }
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2424);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        mgrlog("Bind failed\n");
        return;
    }
    listen(server_socket, 3);
    int c = sizeof(struct sockaddr_in);
    mgrlog("Waiting for incomming connections...\n");
    while (!quit) {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(server_socket, &set);
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        if (select(server_socket + 1, &set, NULL, NULL, &timeout) > 0) {
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&c);
            if (client_socket < 0) {
                mgrlog("Accept failed\n");
            }
            mgrlog("Connection accepted\n");
            if (pthread_create(&network_thread, NULL, connection_handler, &client_socket))
            {
                mgrlog("Error creating thread\n");
            }
        }
        else {
            sleep(1);
        }
    }
}

static void mgrlog(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    vfprintf(logfd, message, args);
    fflush(logfd);
    va_end(args);
}

static void print_help(void)
{
    printf("%-10s:%-100s\n", "zzz", "Enable ZZZ reply");
    printf("%-10s:%-100s\n", "no_zzz", "Disable ZZZ reply");
    printf("%-10s:%-100s\n", "help", "List commands");
    printf("%-10s:%-100s\n", "quit", "Exit");
}
