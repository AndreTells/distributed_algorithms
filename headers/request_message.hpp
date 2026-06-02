#ifndef REQUEST_MESSAGE_H
#define REQUEST_MESSAGE_H
#include <queue>
#include <semaphore.h>

typedef enum {
  REQUEST_TYPE_LOCK,
  REQUEST_TYPE_RELEASE,
  REQUEST_TYPE_GRANT
}request_type_e;

/**
 * Queue that can be  
 */
template <typename T>
class request_queue_t{
  sem_t access_mutex;
  sem_t data;
  std::queue<T> queue;

  public:
    void req_init(){
      sem_init(&access_mutex,0,1);
      sem_init(&data,0,0);
    }

    int req_wait(){
      int wait_res = sem_wait(&data);
      return wait_res;
    }

    T req_pop(){
      sem_wait(&access_mutex);
      T v = queue.front();
      queue.pop();
      sem_post(&access_mutex);
      return v;
    }

    void req_push(T value){
      sem_wait(&access_mutex);
      queue.push(value);
      sem_post(&data);
      sem_post(&access_mutex);
    }
};

#endif
