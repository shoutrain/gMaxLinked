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

CSem::CSem(unsigned int value) {
	if (-1 == sem_init(&_sem, 0, value))
		log_fatal("CSem::CSem: failed to call sem_init");
}

CSem::~CSem() {
	if (-1 == sem_destroy(&_sem))
		log_fatal("CSem::~CSem: failed to call sem_destroy");
}

bool CSem::lock(bool check) {
	int iRet = 0;

	if (check) {
		iRet = sem_trywait(&_sem);
	} else {
		iRet = sem_wait(&_sem);
	}

	if (-1 == iRet) {
		if (EBUSY == errno && check) {
			return false;
		}

		if (check) {
			log_fatal("CSem::Lock: failed to call sem_trywait");
		} else {
			log_fatal("CSem::Lock: failed to call sem_wait");
		}

		return false;
	}

	return true;
}

void CSem::unlock() {
	if (-1 == sem_post(&_sem)) {
		log_fatal("CSem::Unlock: failed to call sem_post");
	}
}

unsigned int CSem::getValue() {
	int n = 0;

	if (-1 == sem_getvalue(&_sem, &n)) {
		log_fatal("CSem::GetValue: failed to call sem_getvalue");
	}

	return (unsigned int) n;
}

