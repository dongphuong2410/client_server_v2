#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "event.h"
#include "queue.h"

extern queue_t *send_queue;

void send_event(const char *event)
{
    queue_enqueue(send_queue, event);
}
