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

#include "CLock.h"

#include <semaphore.h>

class CSem: public CLock {
public:
	CSem(unsigned int value = 1);
	virtual ~CSem();

	bool lock(bool check = false);
	void unlock();

	unsigned int getValue();

private:
	sem_t _sem;
};

#endif // _C_SEM_H_
