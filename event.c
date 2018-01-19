#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "event.h"
#include "queue.h"

extern queue_t *send_queue;
extern pthread_mutex_t send_lock;
extern pthread_cond_t send_cond;

void send_event(const char *event)
{
    while (queue_enqueue(send_queue, event))
    {
        pthread_mutex_lock(&send_lock);
        struct timespec timeout;
        timeout.tv_sec = time(NULL) + 1;
        timeout.tv_nsec = 0;
        pthread_cond_timedwait(&send_cond,  &send_lock, &timeout);
        pthread_mutex_unlock(&send_lock);
        if (!nw_okay())
            break;
        else
            usleep(1000);
    }
}
