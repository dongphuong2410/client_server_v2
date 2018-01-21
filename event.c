#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "event.h"
#include "queue.h"
#include "comm.h"

extern queue_t *send_queue;

void send_event(const char *event)
{
    while (queue_enqueue(send_queue, event))
    {
        queue_wait(send_queue);
        if (!nw_okay())
            break;
        else
            usleep(1000);
    }
}
