#define __STDC_LIMIT_MACROS
#include "logical_clock.h"
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>
#define MAX_TIME INT32_MAX 

struct logical_clock_t{
  uint32_t time;
  sem_t lock;
};

logical_clock_rc_e _inc_clock(logical_clock_t* clock){
  if(clock == nullptr)
    return LOGICAL_CLOCK_RC_INVALID_REFERENCE; 

  if(clock->time < 0)
    return LOGICAL_CLOCK_RC_INVALID_TIME; 

  if(clock->time == MAX_TIME){
    clock->time=0;
    return LOGICAL_CLOCK_RC_INVALID_TIME; 
  }

  clock->time++;

  return LOGICAL_CLOCK_RC_SUCCESS;
}

logical_clock_rc_e _get_time(logical_clock_t* clock, uint32_t* out){
  if(clock == nullptr)
    return LOGICAL_CLOCK_RC_INVALID_REFERENCE; 

  if(clock->time < 0)
    return LOGICAL_CLOCK_RC_INVALID_TIME; 

  *out = clock->time;
  return LOGICAL_CLOCK_RC_SUCCESS;
}

logical_clock_rc_e new_clock(logical_clock_t** clock){
  logical_clock_t* temp = (logical_clock_t*) malloc(sizeof(logical_clock_t));

  if(temp == nullptr)
    return LOGICAL_CLOCK_RC_INSUFFICIENT_MEMORY;

  temp->time = 0;
  sem_init(&temp->lock,0,1);

  *clock = temp;

  return LOGICAL_CLOCK_RC_SUCCESS;
}


logical_clock_rc_e _forward_time(logical_clock_t* clock, uint32_t new_time){
  if(clock == nullptr)
    return LOGICAL_CLOCK_RC_INVALID_REFERENCE; 

  if(clock->time < 0)
    return LOGICAL_CLOCK_RC_INVALID_TIME; 

  if(new_time < 0)
    return LOGICAL_CLOCK_RC_INVALID_TIME; 

  if(clock->time > new_time)
    return LOGICAL_CLOCK_RC_INVALID_TIME_ROLLBACK; 

  clock->time = new_time;

  return LOGICAL_CLOCK_RC_SUCCESS;
}

logical_clock_rc_e inc_clock(logical_clock_t* clock){
  sem_wait(&clock->lock);
  logical_clock_rc_e rc = _inc_clock(clock);
  sem_post(&clock->lock);
  return rc;
}

logical_clock_rc_e get_time(logical_clock_t* clock, uint32_t* out){
  sem_wait(&clock->lock);
  logical_clock_rc_e rc = _get_time(clock, out);
  sem_post(&clock->lock);
  return rc;
}

logical_clock_rc_e forward_time(logical_clock_t* clock, uint32_t new_time){
  sem_wait(&clock->lock);
  logical_clock_rc_e rc = _forward_time(clock, new_time);
  sem_post(&clock->lock);
  return rc;
}
