
/**
 * @file /magma/engine/controller/queue.c
 *
 * @brief	Functions used to distribute tasks to available worker threads.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

typedef struct {
	void (*function)(void *data), (*requeue)(void *data), *data;
	struct queue_t *next;
} queue_t;

struct {
	sem_t sema;
	pthread_t *workers;
	pthread_mutex_t lock;
	queue_t *items;
} queue = {
		.workers = NULL,
		.items = NULL
};

/**
 * @brief	Push a function on the job queue to be executed asynchronously.
 * @note	Warning: If this function fails to allocate a new queue_t object, the work unit is lost forever.
 * @param	function	a pointer to a function to be executed by the next available worker thread.
 * @param	requeue		an optional pointer to a requeue function to be called after function is executed.
 * @param	data		a pointer to an arbitrary block of data to be passed to function and/or requeue upon execution.
 * @return	This function returns no value.
 */
void requeue(void *function, void *requeue, void *data) {

	queue_t *local, *work;

	if (!(work = mm_alloc(sizeof(queue_t)))) {
		log_critical("Failed to allocate a queue_t structure. Work request is lost forever!");
		return;
	}

	work->function = function;
	work->requeue = requeue;
	work->data = data;

	mutex_get_lock(&queue.lock);

	if ((local = queue.items)) {
		while (local->next) local = (queue_t *)local->next;
		local->next = (struct queue_t *)work;
	}
	else {
		queue.items = work;
	}

	mutex_unlock(&queue.lock);
	sem_post(&queue.sema);

	return;
}

/**
 * @brief	Push a function on the job queue to be executed asynchronously.
 * @note	Warning: If this function fails to allocate a new queue_t object, the work unit is lost forever.
 * @param	function	a pointer to a function to be executed by the next available worker thread.
 * @param	data		a pointer to an arbitrary block of data to be passed to the specified function on execution.
 * @return	This function returns no value.
 */
void enqueue(void *function, void *data) {
	requeue(function, NULL, data);
	return;
}

/**
 * @brief	Wait for work to appear on the queue and then perform the work; if the job is to be requeue'd then requeue it.
 * @note	This is the thread pool entry point called from queue_init().
 * @return	This function returns no value.
 */
void dequeue(void) {

	queue_t *work;

	if (!thread_start()) {
		log_error("Unable to setup the thread context.");
		pthread_exit(NULL);
	}

	do {
		sem_wait(&queue.sema);

		mutex_get_lock(&queue.lock);
		work = queue.items;

		if (work) {
			queue.items = (queue_t *)work->next;
		}

		mutex_unlock(&queue.lock);

		if (work) {
			work->function(work->data);

			if (work->requeue) {
				work->requeue(work->data);
			}

			mm_free(work);
		}

	// Continue processing until the work queue is empty and the status tracker indicates a shutdown.
	} while (work || status());

	// Clear the thread specific error stack inside the OpenSSL library.
	thread_stop();
	pthread_exit(NULL);

	return;
}

/**
 * @brief	Signal all worker threads with SIGALRM to force their return.
 * @return	This function returns no value.
 */
void queue_signal(void) {

	int_t ret;

	for (uint64_t i = 0; queue.workers && i < magma.system.worker_threads; i++) {

		if (queue.workers + i && (ret = thread_signal(*(queue.workers + i), SIGALRM)) && status()) {
			log_info("Unable to signal the worker thread. {ret = %i}", ret);
		}

	}

	return;
}

/**
 * @brief	Create a queue of worker threads and set them into motion.
 * @note	Up to magma.system.worker_threads number of threads will be created.
 * @return	false on failure or true on success.
 */
bool_t queue_init(void) {

	if (sem_init(&queue.sema, 0, 0)) {
		return false;
	}

	if (mutex_init(&queue.lock, NULL)) {
		sem_destroy(&queue.sema);
		return false;
	}

	if (!(queue.workers = mm_alloc(sizeof(pthread_t) * magma.system.worker_threads))) {
		queue_shutdown();
		return false;
	}

	for (uint64_t i = 0; i < magma.system.worker_threads; i++) {

		if (thread_launch(queue.workers + i, &dequeue, NULL)) {
			log_error("Unable to launch the configured number of worker threads. {threads = %lu / configured = %u}", i, magma.system.worker_threads);
			queue_shutdown();
			return false;
		}

		stats_increment_by_name("core.threading.workers");
	}

	return true;
}

/**
 * @brief	Attempt to wake any sleeping workers, wait for all worker threads to shutdown and exit, and destroy the worker queue.
 * @return	This function returns no value.
 */
void queue_shutdown(void) {

	for (uint64_t i = 0; queue.workers && i < magma.system.worker_threads + 128; i++) {
		sem_post(&queue.sema);
	}

	for (uint64_t i = 0; queue.workers && i < magma.system.worker_threads; i++) {

		if (queue.workers + i) {
			thread_join(*(queue.workers + i));
			stats_decrement_by_name("core.threading.workers");
		}

	}

	mm_cleanup(queue.workers);
	mutex_destroy(&queue.lock);
	sem_destroy(&queue.sema);

	return;
}
