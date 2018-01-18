#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "queue.h"

typedef struct _node_t {
    char data[QUEUE_NODE_SIZE];
} node_t;

struct _queue_t {
    int q_size;
    int arr_size;
    node_t *nodes;
    int front;
    int rear;
    pthread_mutex_t qlock;
};

static inline int is_full(queue_t *q);
static int is_empty(queue_t *q);

queue_t *queue_init(int q_size)
{
    queue_t *q = (queue_t *)malloc(sizeof(queue_t));
    q->q_size = q_size;
    q->arr_size = q_size + 1;
    q->front = 0;
    q->rear = 0;
    q->nodes = (node_t *)malloc(sizeof(node_t) * (q_size + 1));
    if (!q->nodes) {
        free(q);
        q = NULL;
    }
    pthread_mutex_init(&q->qlock, NULL);
    return q;
}

void queue_destroy(queue_t *q)
{
    if (q ) {
        free(q->nodes);
        pthread_mutex_destroy(&q->qlock);
    }
    free(q);
}

int queue_enqueue(queue_t *q, const char *data)
{
    if (!data || strnlen(data, QUEUE_NODE_SIZE) > QUEUE_NODE_SIZE)
        return -1;
    pthread_mutex_lock(&q->qlock);
    if (is_full(q)) {
        pthread_mutex_unlock(&q->qlock);
        return -1;
    }

    strncpy(q->nodes[q->rear].data, data, QUEUE_NODE_SIZE);
    q->rear++;
    if (q->rear == q->arr_size) q->rear = 0;
    pthread_mutex_unlock(&q->qlock);
    return 0;
}

char *queue_dequeue(queue_t *q)
{
    pthread_mutex_lock(&q->qlock);
    if (is_empty(q)) {
        pthread_mutex_unlock(&q->qlock);
        return NULL;
    }
    char *ret = q->nodes[q->front].data;
    q->front++;
    if (q->front == q->arr_size) q->front = 0;
    pthread_mutex_unlock(&q->qlock);
    return ret;
}

static inline int is_full(queue_t *q)
{
    return (q->rear + 1 == q->front) || (q->rear == q->arr_size -1 && q->front == 0);;
}

static inline int is_empty(queue_t *q)
{
    return (q->front == q->rear);
}

