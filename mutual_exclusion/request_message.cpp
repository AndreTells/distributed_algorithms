#include "request_message.hpp"

template <typename T>
int request_queue_t<T>::req_wait(){
  int wait_res = sem_wait(&data);
  return wait_res;
}

template <typename T>
T request_queue_t<T>::req_pop(){
  sem_wait(&access_mutex);
  T v = queue.front();
  queue.pop();
  sem_post(&access_mutex);
  return v;
}

template <typename T>
void request_queue_t<T>::req_push(T value){
  sem_wait(&access_mutex);
  queue.push(value);
  sem_post(&data);
  sem_post(&access_mutex);
}

template <typename T>
void request_queue_t<T>::req_init(){
  sem_init(&access_mutex,0,1);
  sem_init(&data,0,0);
}
