/*
 ============================================================================
 Name        : CSem.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of semaphore
 ============================================================================
 */

#include "CSem.h"

#include <sys/errno.h>

CSem::CSem(ub4_ value) {
    if (-1 == sem_init(&_sem, 0, value))
        log_fatal("CSem::CSem: failed to call sem_init");
}

CSem::~CSem() {
    if (-1 == sem_destroy(&_sem))
        log_fatal("CSem::~CSem: failed to call sem_destroy");
}

bool_ CSem::lock(bool_ check) {
    b4_ iRet = 0;

    if (check) {
        iRet = sem_trywait(&_sem);
    } else {
        iRet = sem_wait(&_sem);
    }

    if (-1 == iRet) {
        if (EBUSY == errno && check) {
            return false_v;
        }

        if (check) {
            log_fatal("CSem::Lock: failed to call sem_trywait");
        } else {
            log_fatal("CSem::Lock: failed to call sem_wait");
        }

        return false_v;
    }

    return true_v;
}

none_ CSem::unlock() {
    if (-1 == sem_post(&_sem)) {
        log_fatal("CSem::Unlock: failed to call sem_post");
    }
}

ub4_ CSem::getValue() {
    b4_ n = 0;

    if (-1 == sem_getvalue(&_sem, &n)) {
        log_fatal("CSem::GetValue: failed to call sem_getvalue");
    }

    return (ub4_)n;
}
