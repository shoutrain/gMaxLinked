#include "common.h"

#include <stddef.h>
#include <sys/time.h>
#include <sys/select.h>

int goo_sleep(unsigned int s, unsigned int ms) {
    struct timeval delay;

    delay.tv_sec  = s;
    delay.tv_usec = ms * 1000;

    return select(0, NULL, NULL, NULL, &delay);
}
