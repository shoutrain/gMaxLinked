/*
 ============================================================================
 Name        : CSem.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of semaphore
 ============================================================================
 */

#ifndef _C_SEM_H_
#define _C_SEM_H_

#include <semaphore.h>

#include "CLock.h"

class CSem : public CLock {
public:
    CSem(ub4_ value = 1);
    virtual ~CSem();

    bool_ lock(bool_ check = false_v);
    none_ unlock();

    ub4_ getValue();

private:
    sem_t _sem;
};

#endif  // _C_SEM_H_
