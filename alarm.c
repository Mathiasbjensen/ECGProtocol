/*
 * timer.c
 *
 * Implementation of simple timers (called alarms)
 */


#include "alarm.h"

// Uses
#include <time.h>

long long int get_time_ms(void) {

    long long int ms;
    struct timespec spec;

    clock_gettime(CLOCK_MONOTONIC, &spec);

    ms =  spec.tv_sec  * 1000;
    ms += spec.tv_nsec / 1000000; // Convert nanoseconds to milliseconds

    return ms;
}


int alarm_init(alarm_t * alm) {

    alm->expiration_time = 0L;

    return ERR_OK;
}

int alarm_set(alarm_t * alm, int to_ms) {

    if (to_ms < 0) {
        return ERR_INVAL;
    }

    alm->expiration_time = get_time_ms() + to_ms;

    return ERR_OK;
}

int alarm_rem(const alarm_t * alm) {

    long long int rem = alm->expiration_time - get_time_ms();

    if (rem < 0) { rem = 0; }

    return (int) rem;
}

int alarm_expired(const alarm_t * alm) {

    return (alarm_rem(alm) == 0);
}

