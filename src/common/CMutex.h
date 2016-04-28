/*
 ============================================================================
 Name        : CMutex.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of mutex
 ============================================================================
 */


#ifndef _C_MUTEX_H_
#define _C_MUTEX_H_

#include "CLock.h"

#include <pthread.h>

class CMutex: public CLock {
public:
	CMutex(bool recursive = false);
	virtual ~CMutex();

	bool lock(bool check = false);
	void unlock();

	pthread_mutex_t *getMutex() {
		return &_mutex;
	}

private:
	pthread_mutex_t _mutex;
};

#endif // _C_MUTEX_H_
