/*
 ============================================================================
 Name        : CMutex.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Encapsulation of mutex
 ============================================================================
 */


#include "CMutex.h"

#include <sys/errno.h>

CMutex::CMutex(bool recursive) {
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);

	if (recursive) {
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	}

	if (0 != pthread_mutex_init(&_mutex, &attr)) {
		log_fatal("CMutex::CMutex: failed to call pthread_mutex_init");
	}

	pthread_mutexattr_destroy(&attr);
}

CMutex::~CMutex() {
	if (0 != pthread_mutex_destroy(&_mutex)) {
		log_fatal("CMutex::~CMutex: failed to call pthread_mutex_destroy");
	}
}

bool CMutex::lock(bool check) {
	int ret = 0;

	if (check) {
		ret = pthread_mutex_trylock(&_mutex);
	} else {
		ret = pthread_mutex_lock(&_mutex);
	}

	if (EBUSY == ret && check) {
		return false;
	}

	if (0 != ret) {
		if (check) {
			log_fatal("CMutex::Lock: failed to call pthread_mutext_trylock");
		} else {
			log_fatal("CMutex::Lock: failed to call pthread_mutext_lock");
		}

		return false;
	}

	return true;
}

void CMutex::unlock() {
	if (0 != pthread_mutex_unlock(&_mutex)) {
		log_fatal("CMutex::Unlock: failed to call pthread_mutex_unlock");
	}
}

