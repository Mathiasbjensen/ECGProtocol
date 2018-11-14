/*
 * alarm.h
 *
 * Interface to simple timers (called alarms)
 */

#ifndef _ALARM_H_
#define _ALARM_H_

#include "errors.h"

typedef struct {long long int expiration_time; }  alarm_t;

// Alarms must be initialized before usage
int alarm_init(alarm_t * alm);

// Set (or reset) alarm to time out in ms milliseconds
int alarm_set(alarm_t * alm, int ms);

// Get remaining time in millisecond until timeout (0 if timed out)
int alarm_rem(const alarm_t * alm);

// Check if alarm has timed out
int alarm_expired(const alarm_t * alm);

#endif // _ALARM_H_
