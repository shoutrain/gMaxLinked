/*
 ============================================================================
 Name        : CAutoLock.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : helper for auto-locker
 ============================================================================
 */

#ifndef _C_AUTO_LOCK_H_
#define _C_AUTO_LOCK_H_

#include "CLock.h"

class CAutoLock: public CBase {
public:
	CAutoLock(CLock *lock = NULL);
	virtual ~CAutoLock();

	const CLock *get() const {
		return _lock;
	}

private:
	// Cannot new this class
	void *operator new(size_t size);
	CLock *_lock;
};

#endif // _C_AUTO_LOCK_H_
