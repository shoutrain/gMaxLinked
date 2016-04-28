/*
 ============================================================================
 Name        : CCond.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of condition
 ============================================================================
 */


#include "CCond.h"
#include "CMutex.h"

CCond::CCond(CMutex *mutex) {
	assert(mutex);
	_mutex = mutex;

	if (0 != pthread_cond_init(&_cond, NULL)) {
		log_fatal("CCond::CCond: failed to call pthread_cond_init");
	}
}

CCond::~CCond() {
	if (0 != pthread_cond_destroy(&_cond)) {
		log_fatal("CCond::~CCond: failed to call pthread_cond_destroy");
	}
}

bool CCond::lock(bool check) {
	if (0 != pthread_cond_wait(&_cond, _mutex->getMutex())) {
		log_fatal("CCond::Lock: failed to call pthread_cond_wait");

		return false;
	}

	return true;
}

void CCond::unlock() {
	if (0 != pthread_cond_signal(&_cond)) {
		log_fatal("CCond::Unlock: failed to call pthread_cond_signal");
	}
}

