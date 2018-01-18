#ifndef __AGENT_QUEUE_H__
#define __AGENT_QUEUE_H__

#define QUEUE_NODE_SIZE 1024

typedef struct _queue_t queue_t;

/**
 * Init queue
 */
queue_t *queue_init(int q_size);

/**
 * Destroy queue
 */
void queue_destroy(queue_t *q);

/**
 * Enqueue
 * Return 0 on success, else -1 on error
 */
int queue_enqueue(queue_t *q, const char *data);

/**
 * Dequeue
 * Return 0 on success, else -1 on error
 */
char *queue_dequeue(queue_t *q);

#endif
