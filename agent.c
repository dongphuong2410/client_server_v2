#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "comm.h"
#include "queue.h"
#include "simul_params.h"

static void handle_signal(void);
static void create_queue(void);
static void destroy_queue(void);
static void create_threads(void);
static void destroy_threads(void);
static void create_mutex(void);
static void destroy_mutex(void);
static void parse_msg(const char *msg);
static void final_agent(void);

static void *send_thread(void *data);
static void *recv_thread(void *data);
static void *module_thread(void *data);
static void *check_manager_thread(void *data);
static void *time_thread(void *data);

int running = 1;
time_t health_time;
static unsigned long long event_cnt = 0;
static pthread_mutex_t health_lock;
static pthread_mutex_t module_lock;

static struct sigaction act;
void sig_handler(int signal)
{
    final_agent();
}

/* Global queues */
queue_t *send_queue;
queue_t *recv_queue;

/* Global threads */
pthread_t send_thread_id;
pthread_t recv_thread_id;
pthread_t module_thread_ids[MODULE_NO];
pthread_t check_manager_thread_id;
pthread_t time_thread_id;

int is_running()
{
    int ret = running;
}

int main(int argc, char **argv)
{
    create_mutex();
    handle_signal();
    create_queue();
    create_threads();

    int iCount = 0;
    time_t start_time;
    time(&start_time);
    while (running) {
        nw_connect();
        if (!nw_okay) {
            sleep(1);
        }
        /* request connect */
        nw_write("ACQ", 3);

        while (nw_okay()) {
            char *msg = queue_dequeue(recv_queue);
            if (msg) {
                parse_msg(msg);
            }
            else {
                iCount++;
                if (iCount >= HANG_TEST_LOOP) {
                    iCount = 0;
                    usleep(10 * 1000);
                }
            }
            usleep(10 * 1000);
        }
        usleep(10 * 1000);
        nw_destroy();
    }
    time_t end_time;
    time(&end_time);

    printf("time %u eps %llu\n", end_time - start_time, event_cnt / (end_time - start_time));
    destroy_threads();
    destroy_queue();
    destroy_mutex();
    return 0;
}

static void handle_signal(void)
{
    act.sa_handler = sig_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
}

static void create_queue(void)
{
    send_queue = queue_init(QUEUE_SIZE);
    recv_queue = queue_init(QUEUE_SIZE);
}

static void destroy_queue(void)
{
    queue_destroy(send_queue);
    queue_destroy(recv_queue);
}

static void create_threads(void)
{
    if (pthread_create(&recv_thread_id, NULL, recv_thread, NULL))
    {
        printf("Error creating recv thread\n");
    }
    else {
        printf("Receive thread created\n");
    }
    if (pthread_create(&send_thread_id, NULL, send_thread, NULL))
    {
        printf("Error creating send thread\n");
    }
    else {
        printf("Send thread created\n");
    }
    if (pthread_create(&check_manager_thread_id, NULL, check_manager_thread, NULL))
    {
        printf("Error creating check_manager thread\n");
    }
    else {
        printf("Check_manager thread created\n");
    }
    if (pthread_create(&time_thread_id, NULL, time_thread, NULL))
    {
        printf("Error creating time thread\n");
    }
    else {
        printf("Time thread created\n");
    }
    int i;
    for (i = 0; i < MODULE_NO; i++) {
        if (pthread_create(&module_thread_ids[i], NULL, module_thread, NULL)) {
            printf("Error creating module thread\n");
        }
        else {
            printf("Module thread %d created\n", i);
        }
    }
}

static void destroy_threads(void)
{
    int i;
    for (i = 0; i < MODULE_NO; i++) {
        pthread_join(module_thread_ids[i]);
        printf("Module thread %d destroyed\n", i);
    }
    pthread_join(time_thread_id);
    printf("Time_thread destroyed\n");
    pthread_join(check_manager_thread_id);
    printf("Check_manager thread destroyed\n");
    pthread_join(send_thread_id);
    printf("Send thread destroyed\n");
    pthread_join(recv_thread_id);
    printf("Receive thread destroyed\n");
}

static void *recv_thread(void *data)
{
    static char buff[PACKET_LEN];
    while (running) {
        if (!nw_okay()) {
            usleep(10 * 1000);
        }
        else {
            int bytes = nw_read(buff);
            if (bytes <= 0) {
                usleep(101 * 1000);
            }
            else {
                queue_enqueue(recv_queue, buff);
            }
        }
    }
}

static void *send_thread(void *data)
{
    int iCount = 0;
    while (running) {
        if (!nw_okay) {
            usleep(1000 * 1000);
        }
        else {
            char *buff = NULL;
            while ((buff = queue_dequeue(send_queue))) {
                nw_write(buff, strnlen(buff, PACKET_LEN));
                if (!nw_okay()) {
                    break;
                }
                iCount++;
                if (iCount == 10000) {
                    usleep(10 * 1000);
                    iCount = 0;
                }
            }
            if (!buff) {
                usleep(10 * 1000);
            }
        }
    }
}

static void *check_manager_thread(void *data)
{
    int iCount = 0;
    while (running) {
        iCount++;
        if (iCount >= HEALTH_TIME) {
            iCount = 0;
            send_event("ZZZ");
        }
        usleep(1000 * 1000);
    }
}

static void *time_thread(void *data)
{
    int iCount = 0;
    while (running) {
        iCount++;
        if (iCount == 10) {
            if (nw_okay()) {
                time_t curr;
                time(&curr);
                pthread_mutex_lock(&health_lock);
                if (curr - health_time > HEALTH_TIME * MANAGER_RETRY) {
                    printf("Manager not response for long time. Disconnect ...\n");
                    nw_disconnect();
                }
                pthread_mutex_unlock(&health_lock);
            }
            iCount = 0;
        }
        usleep(1000 * 1000);
    }
}

static void *module_thread(void *data)
{
    while (running) {
        //pthread_mutex_lock(&module_lock);
        //event_cnt++;
        //pthread_mutex_unlock(&module_lock);
        send_event("EVT");
        usleep(MODULE_SLEEP);
    }
}

static void parse_msg(const char *msg)
{
    pthread_mutex_lock(&health_lock);
    time(&health_time);
    pthread_mutex_unlock(&health_lock);
    if (!strncmp(msg, "ACK", PACKET_LEN)) {
        printf("ACK received\n");
    }
    else if (!strncmp(msg, "ZZZ", PACKET_LEN)) {
    }
}

static void final_agent(void)
{
    nw_disconnect();
    running = 0;
}

static void create_mutex(void)
{
    pthread_mutex_init(&health_lock, NULL);
    pthread_mutex_init(&module_lock, NULL);
}

static void destroy_mutex(void)
{
    pthread_mutex_destroy(&health_lock);
    pthread_mutex_destroy(&module_lock);
}
