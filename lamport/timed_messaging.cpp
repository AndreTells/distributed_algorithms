#include "timed_messaging.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>

#include "logical_clock.h"

ssize_t timed_send(int socket, const void *buffer, size_t length, int flags, logical_clock_t* clock){
  if(inc_clock(clock) != LOGICAL_CLOCK_RC_SUCCESS){
    return -1;
  }
  uint32_t time = 0; 
  get_time(clock, &time);
  ssize_t clock_send_res = send(socket, &time, sizeof(uint32_t), flags); 

  if(clock_send_res == -1)
    return -1;

  ssize_t send_res = send(socket, buffer, length, flags); 
  if(send_res == -1)
    return -1;

  return send_res+clock_send_res;
}

ssize_t timed_recv(int socket, void *buffer, size_t length, int flags, logical_clock_t* clock){
  uint32_t remote_time;
  ssize_t clock_recv_res = recv(socket, &remote_time, sizeof(uint32_t), flags);

  if(clock_recv_res == -1)
    return -1;

  if(forward_time(clock, remote_time) != LOGICAL_CLOCK_RC_SUCCESS){
    return -1;
  }

  if(inc_clock(clock) != LOGICAL_CLOCK_RC_SUCCESS){
    return -1;
  }

  ssize_t recv_res = recv(socket, buffer, length, flags); 
  if(recv_res == -1)
    return -1;

  return recv_res+clock_recv_res;
}
