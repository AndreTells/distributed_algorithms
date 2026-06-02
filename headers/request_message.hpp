#ifndef REQUEST_MESSAGE_H
#define REQUEST_MESSAGE_H
#include <queue>
#include <semaphore.h>

typedef enum {
  REQUEST_TYPE_LOCK,      // Message to request the ressource for the sender 
  
  REQUEST_TYPE_RELEASE,   // Message to release the ressource given that it was
                          // locked beforehand
  
  REQUEST_TYPE_GRANT      // Message sent by the server to grant the ressource 
                          // to a client
}request_type_e;

/**
 * A thread-safe queue object that users can wait on for more data. To be
 * utilised, an object of this class must be initialized via a call of req_init
 */
template <typename T>
class request_queue_t{
  sem_t access_mutex;
  sem_t data;
  std::queue<T> queue;

  public:
    /**
     * Initializes the sem_t paramets with:
     * - access_mutex := 1
     * - data := 0
     */
    void req_init();

    /**
     * Blocks the thread if the queue is empty and allows one call to go through
     * ONCE for every piece of data in the queue. If a thread then does not
     * process an element of the queue using req_pop, the queue will be in an
     * invalid state. Consumes from the data semaphore
     * @return result of the call to sem_wait(&data);
     */
    int req_wait();

    /**
     * Returns the value of the first element in the queue and removes it from
     * the queue
     * @return The value of the first element of the queue
     */
    T req_pop();

    /**
     * Adds one element to the queue and posts to the data semaphore 
     */
    void req_push(T value);
};

#endif
