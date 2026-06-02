#ifndef LOGICAL_CLOCK_H
#define LOGICAL_CLOCK_H
#include <stdint.h>

//macro that will make the entire program fail in case of any failure in the
//logical clock
#define LOGICAL_CLOCK_ERR_GUARD(x) if(x!=LOGICAL_CLOCK_RC_SUCCESS){exit(-1);}

typedef struct logical_clock_t logical_clock_t;

typedef enum {
  LOGICAL_CLOCK_RC_SUCCESS,               // No issues ocurred
  LOGICAL_CLOCK_RC_INVALID_REFERENCE,     // The provided clock pointer is not a
                                          // valid object

  LOGICAL_CLOCK_RC_INVALID_TIME,          // The provided time value is negative
  LOGICAL_CLOCK_RC_INVALID_TIME_ROLLBACK, // The provided fastforward time is
                                          // before the clock's current time
  LOGICAL_CLOCK_RC_WARNING_TIME_OVERFLOW, // Signals that the last operation
                                          // increase the clock's value past the
                                          // limit and it has been reset to 0
  LOGICAL_CLOCK_RC_INSUFFICIENT_MEMORY    // The program failed to get memory
} logical_clock_rc_e;

/**
 * Adds 1 to the clock's time
 * @param clock a non nullptr pointer reference to the clock object
 */
logical_clock_rc_e inc_clock(logical_clock_t* clock);

/**
 * Checks the latest value of the clock's time
 * @param clock non nullptr pointer reference to the clock object
 * @param out pointer to an int that will be used to stored the clock's time
 */
logical_clock_rc_e get_time(logical_clock_t* clock, uint32_t* out);

/**
 * Creates a new clock object and returns a pointer to it
 * @param clock pointer to a pointer of clock, where the resulting clock pointer
 *        will be stored
 */
logical_clock_rc_e new_clock(logical_clock_t** clock);

/**
 * Fastforwords a local clocks internal time to match a remote clock's time
 * @param clock non nullptr pointer reference to the clock object
 * @param new_time time recorded by the remote clock 
 */
logical_clock_rc_e forward_time(logical_clock_t* clock, uint32_t new_time);

#endif
