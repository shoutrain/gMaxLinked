/*
 ============================================================================
 Name        : CCond.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of condition
 ============================================================================
 */

#ifndef _C_COND_H_
#define _C_COND_H_

#include <pthread.h>

#include "CLock.h"

class CMutex;

class CCond : public CLock {
public:
    CCond(CMutex *mutex);
    virtual ~CCond();

    bool_ lock(bool_ check = false_v);
    none_ unlock();

private:
    pthread_cond_t _cond;
    CMutex *_mutex;
};

#endif  // _C_COND_H_
