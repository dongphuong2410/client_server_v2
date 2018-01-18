#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "queue.h"

int main(int argc, char **argv)
{
    char *data;
    int error;
    queue_t *q = queue_init(5);

    error = queue_enqueue(q, "One");
    assert(!error);
    error = queue_enqueue(q, "Two");
    assert(!error);
    error = queue_enqueue(q, "Three");
    assert(!error);
    error = queue_enqueue(q, "Four");
    assert(!error);
    error = queue_enqueue(q, "Five");
    assert(!error);
    error = queue_enqueue(q, "Six");
    assert(error);

    data = queue_dequeue(q);
    assert(data);
    assert(!strcmp(data, "One"));
    data = queue_dequeue(q);
    assert(data);
    assert(!strcmp(data, "Two"));
    data = queue_dequeue(q);
    assert(data);
    assert(!strcmp(data, "Three"));
    data = queue_dequeue(q);
    assert(data);
    assert(!strcmp(data, "Four"));
    data = queue_dequeue(q);
    assert(data);
    assert(!strcmp(data, "Five"));
    data = queue_dequeue(q);
    assert(!data);

    queue_destroy(q);

    return 0;
}
