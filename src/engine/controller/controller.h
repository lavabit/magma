
/**
 * @file /magma/engine/controller/controller.h
 *
 * @brief	Functions involved with assigning tasks to worker threads and routing network connections to the proper protocol subsystem.
 */

#ifndef MAGMA_ENGINE_CONTROLLER_H
#define MAGMA_ENGINE_CONTROLLER_H

/// queue.c
void     dequeue(void);
void     enqueue(void *function, void *data);
bool_t   queue_init(void);
void     queue_shutdown(void);
void     queue_signal(void);
void     requeue(void *function, void *requeue, void *data);

/// protocol.c
bool_t protocol_init(void);
void protocol_process(server_t *server, int sockd);

#endif
