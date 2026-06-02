#ifndef TIMED_MESSAGING_H
#define TIMED_MESSAGING_H
#include <sys/types.h>
#include "logical_clock.h"

/**
 * Same interface as regular linux send, except it will send the clocks time
 * before it. As per the lamport logical clock algorithm, sending a message is
 * considered an event, thus the clock's time is increased before being sent
 *
 * @param clock a non nullptr pointer reference to the clock object
 * @return -1 if any function calls fails, else it returns the sum of sending
 *         the clock data + the actual message 
 */
ssize_t timed_send(int socket, const void *buffer, size_t length, int flags, logical_clock_t *clock);

/**
 * Same interface as regular linux recv, except it expects a uint32_t logical
 * clock time stamp to be sent before it. As per the lamport logical clock
 * algorithm, receiving a message is considered an event, thus the clock's time is
 * increased before being received 
 */
ssize_t timed_recv(int socket, void *buffer, size_t length, int flags, logical_clock_t *clock);

#endif
